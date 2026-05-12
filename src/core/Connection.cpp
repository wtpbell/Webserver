/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Connection.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/03/17 11:34:04 by jboon         #+#    #+#                 */
/*   Updated: 2026/05/12 16:37:05 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "core/Connection.hpp"

#include <sys/epoll.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "utils/Logger.hpp"
#include "core/RequestContext.hpp"
#include "cgi/CGIProcess.hpp"
#include "cgi/CGIResponse.hpp"
#include "config/RouteView.hpp"
#include "http/BodySink.hpp"
#include "http/HTTPCookie.hpp"
#include "http/HTTPRequest.hpp"
#include "http/HTTPResponse.hpp"
#include "http/HTTPStatus.hpp"
#include "http/HTTPUtils.hpp"
#include "http/HTTPValidator.hpp"
#include "http/ResponseFactory.hpp"
#include "utils/string.hpp"

namespace fs = std::filesystem;

namespace
{
  fs::path MakeUploadTempPath(int client_fd)
  {
    static std::uint64_t counter = 0;
    fs::path dir = "./www/.upload_tmp";
    std::error_code ec;
    fs::create_directories(dir, ec);
    return dir / ("up_" + std::to_string(client_fd) + "_" + std::to_string(++counter) + ".tmp");
  }
}  // namespace

Connection::State Connection::FailRequest(ValidationResult error)
{
  QueueError(error);
  closeAfterSend_ = true;
  matchedRoute_ = nullptr;
  return State::kKeepAlive;
}

Connection::State Connection::ProcessInput(std::string_view in, RequestContext& requestContext)
{
  while (true)
  {
    // Parse And Validate request
    {
      HTTPParser::ParseResult result = parser_.Parse(in);
      in = {};

      if (result == HTTPParser::ParseResult::kNeedMoreData)
        return State::kKeepAlive;

      if (result == HTTPParser::ParseResult::kError)
        return FailRequest(parser_.GetError());

      if (result == HTTPParser::ParseResult::kHeadersDone)
      {
        std::optional<ValidationResult> headerError = HandleHeadersDone(requestContext);
        if (headerError)
        {
          parser_.ResetNextRequest();
          return FailRequest(*headerError);
        }
        continue;
      }
    }

    HTTPRequest request = parser_.TakeRequest();
    const RouteView* route = matchedRoute_;
    parser_.ResetNextRequest();
    matchedRoute_ = nullptr;

    std::variant<std::monostate, HTTPResponse, cgi::CGIProcess> dispatchResponse;
    if (route == nullptr)
    {
      dispatchResponse = HTTP::response::MakeError(HTTP::Status::kNotFound);
    }
    else
    {
      dispatchResponse = requestContext.Dispatch(request, *route, socket_.GetSocketInfo());
    }

    if (std::holds_alternative<HTTPResponse>(dispatchResponse))
    {
      HTTPResponse& httpResponse = std::get<HTTPResponse>(dispatchResponse);
      requestContext.GetSessionManager().UseOrCreateSession(request, httpResponse);
      const bool closeAfter = CheckConnectionClose(request, httpResponse);
      if (closeAfter)
        httpResponse.SetHeader("Connection", "close");
      QueueResponse(httpResponse, closeAfter);
      if (closeAfter)
        return State::kKeepAlive;
    }
    else
    {
      cgi::CGIProcess& cgiProcess = std::get<cgi::CGIProcess>(dispatchResponse);
      const bool isComplete = cgiProcess.GetRequest().IsClosedConnection();
      QueueCgiResponse(cgiProcess.GetSocket().GetFD(), isComplete);
      if (!requestContext.RegisterProcess(socket_.GetFD(), std::move(cgiProcess)))
      {
        Logger::Log(LogLevel::ERROR, "ProcessInput: Failed to register cgiFd {} in the registry",
                    cgiProcess.GetSocket().GetFD());
        UpdateCgiErrorResponse(cgiProcess.GetSocket().GetFD(), HTTP::Status::kInternalServerError,
                               cgiProcess.GetRequest(), requestContext);
      }
      if (isComplete)
      {
        return State::kKeepAlive;
      }
    }
  }
}

Connection::Connection(Socket socket)
    : socket_(std::move(socket)), matchedRoute_(nullptr), closeAfterSend_(false), peerClosed_(false)
{
}

void Connection::SetPeerClosed(bool close)
{
  peerClosed_ = close;
}

bool Connection::HasPendingOutput(void) const
{
  return !outputQueue_.empty();
}

bool Connection::HasPauseReading(void) const
{
  return closeAfterSend_ || outputQueue_.size() > 64;
}

const Socket& Connection::GetSocket(void) const
{
  return socket_;
}

void Connection::Clear(void)
{
  outputQueue_.clear();
  closeAfterSend_ = false;
  peerClosed_ = false;
  matchedRoute_ = nullptr;

  socket_.Reset();
  parser_.Reset();
}

/********************************************* Event Handlers *********************************************************/

Connection::State Connection::HandleRequest(RequestContext& requestContext)
{
  std::string msg;
  const ReadResult result = ReadOnce(msg);
  if (result == ReadResult::kClosed)
  {
    peerClosed_ = true;
    return outputQueue_.empty() ? State::kClose : State::kKeepAlive;
  }
  if (result == ReadResult::kFatal)
    return State::kError;
  return ProcessInput(msg, requestContext);
}

Connection::State Connection::HandleResponse(void)
{
  if (outputQueue_.empty())
  {
    return (closeAfterSend_ || peerClosed_) ? State::kClose : State::kKeepAlive;
  }

  PendingResponse& output = outputQueue_.front();
  if (output.state_ == PendingResponse::State::kWaiting)
  {
    return State::kKeepAlive;
  }
  const ssize_t ret = socket_.Send(output.wire_, output.remaining_);
  if (ret < 0)
  {
    return State::kError;
  }
  if (ret == 0)
  {
    return State::kKeepAlive;
  }

  // If fully sent current front item, pop and continue finishing.
  if (output.remaining_ == 0)
  {
    outputQueue_.pop_front();
    return (outputQueue_.empty() && (closeAfterSend_ || peerClosed_)) ? State::kClose : State::kKeepAlive;
  }
  return State::kKeepAlive;
}

/********************************************* Queue Response *********************************************************/

void Connection::QueueResponse(const HTTPResponse& response, bool closeAfter)
{
  std::string wire{HTTP::wire::SerializeResponse(response)};
  if (!isFirstResponse_)
    wire.insert(0, "\r\n");

  closeAfterSend_ = closeAfterSend_ || closeAfter;
  outputQueue_.emplace_back(PendingResponse::State::kReady, -1, std::move(wire), closeAfterSend_);
  isFirstResponse_ = false;
}

void Connection::QueueError(ValidationResult result)
{
  HTTP::Status status = HTTP::ToHTTPStatus(result);

  HTTPResponse response = HTTP::response::MakeError(status);
  response.SetHeader("Connection", "close");

  QueueResponse(response, true);
}
void Connection::QueueCgiResponse(const int cgiFd, bool closeAfter)
{
  closeAfterSend_ = closeAfterSend_ || closeAfter;
  outputQueue_.emplace_back(PendingResponse::State::kWaiting, cgiFd, isFirstResponse_ ? "" : "\r\n", closeAfterSend_);
  isFirstResponse_ = false;
}

void Connection::UpdateCgiErrorResponse(const int cgiFd, HTTP::Status errorStatus, const cgi::CGIRequest& cgiRequest,
                                        RequestContext& requestContext)
{
  auto it = std::find_if(outputQueue_.begin(), outputQueue_.end(),
                         [cgiFd](const PendingResponse& output)
                         {
                           return output.cgiFd_ == cgiFd;
                         });
  assert(it != outputQueue_.end());

  HTTPResponse httpResponse{requestContext.DispatchError(errorStatus, cgiRequest.GetRouteView(),
                                                         cgiRequest.GetIpPortServer(), cgiRequest.GetHostname())};
  it->cgiFd_ = -1;
  it->wire_.append(HTTP::wire::SerializeResponse(httpResponse));
  it->remaining_ = it->wire_.length();
  it->state_ = PendingResponse::State::kReady;
}

void Connection::UpdateCgiResponse(const int cgiFd, const cgi::CGIResponse& response)
{
  auto it = std::find_if(outputQueue_.begin(), outputQueue_.end(),
                         [cgiFd](const PendingResponse& output)
                         {
                           return output.cgiFd_ == cgiFd;
                         });
  assert(it != outputQueue_.end());

  it->cgiFd_ = -1;
  it->wire_.append(response.SerializeAsHttp());
  it->remaining_ = it->wire_.length();
  it->state_ = PendingResponse::State::kReady;
}

int Connection::RedirectCgiResponse(const int cgiFd, const std::string& localTarget, const std::string& hostname,
                                    RequestContext& requestContext)
{
  auto it = std::find_if(outputQueue_.begin(), outputQueue_.end(),
                         [cgiFd](const PendingResponse& output)
                         {
                           return output.cgiFd_ == cgiFd;
                         });
  assert(it != outputQueue_.end());

  std::variant<std::monostate, HTTPResponse, cgi::CGIProcess> dispatchResponse =
      requestContext.Dispatch(localTarget, hostname, socket_.GetSocketInfo());
  if (std::holds_alternative<HTTPResponse>(dispatchResponse))
  {
    HTTPResponse& httpResponse = std::get<HTTPResponse>(dispatchResponse);
    if (it->closeAfter_)
      httpResponse.SetHeader("Connection", "close");

    it->cgiFd_ = -1;
    it->wire_.append(HTTP::wire::SerializeResponse(httpResponse));
    it->remaining_ = it->wire_.length();
    it->state_ = PendingResponse::State::kReady;
    return cgiFd;
  }

  cgi::CGIProcess& cgiProcess = std::get<cgi::CGIProcess>(dispatchResponse);
  const int newCgiFd = cgiProcess.GetSocket().GetFD();
  if (!requestContext.RegisterProcess(socket_.GetFD(), std::move(cgiProcess)))
  {
    Logger::Log(LogLevel::ERROR, "RedirectCgiResponse: Failed to register cgiFd {} in the registry",
                cgiProcess.GetSocket().GetFD());
    UpdateCgiErrorResponse(cgiFd, HTTP::Status::kInternalServerError, cgiProcess.GetRequest(), requestContext);
    return cgiFd;
  }
  it->cgiFd_ = newCgiFd;
  return newCgiFd;
}

/*************************************** Parsing / Validation / Body **************************************************/

// After ValidateRequest(request) == kOk:
// - transfer-encoding is either absent or exactly "chunked"
// - transfer-encoding and content-length are not both present
// - content-length (if present) is valid and within limits
void Connection::InitBodyParser(HTTPParser& parser, const HTTPRequest& request)
{
  if (request.IsChunked())
    return parser.SetChunked();

  std::optional<std::size_t> len = request.GetContentLength();
  if (len)
    return parser.SetContentLength(*len);

  parser.SetNoBody();
}

std::optional<ValidationResult> Connection::ValidatePartialRequest(HTTPRequest& request)
{
  const ValidationResult validationResult = ValidateRequest(request);
  if (validationResult == ValidationResult::kOk)
    return std::nullopt;

  Logger::Log(LogLevel::ERROR, "Connection: Validation error ({}) on request [{} {}] from client {}",
              static_cast<int>(validationResult), request.GetMethodString(), request.GetTarget(), socket_.GetFD());

  return validationResult;
}

void Connection::MatchRouteAndApplyLimits(RequestContext& requestContext, const HTTPRequest& request)
{
  std::string_view hostname{request.GetHost()};
  std::string_view targetPath{request.GetPath()};

  Logger::Log(LogLevel::INFO, "lookup ip='{}' port='{}' host='{}' path='{}'", requestContext.GetIp(),
              requestContext.GetPort(), hostname, targetPath);

  matchedRoute_ = requestContext.GetRouteView(hostname, targetPath);
  if (matchedRoute_ != nullptr)
    parser_.SetMaxBody(matchedRoute_->clientMaxBody);
  else
    parser_.SetMaxBody(HTTP::kMaxBodySize);
}

std::optional<ValidationResult> Connection::ConfigureBodyStorage(HTTPRequest& request)
{
  if ((matchedRoute_ != nullptr && (matchedRoute_->cgi || matchedRoute_->cgiExePaths.has_value())))
  {
    parser_.SetBodySink(std::make_unique<MemorySink>(request));
    return std::nullopt;
  }

  const bool isChunked = request.IsChunked();
  const std::optional<std::size_t> contentLength = request.GetContentLength();
  if (isChunked || (contentLength && *contentLength >= HTTP::kMemoryThreshold))
  {
    const fs::path tempPath = MakeUploadTempPath(socket_.GetFD());
    request.SetBodyFilePath(tempPath);

    std::unique_ptr<FileSink> sink = std::make_unique<FileSink>(tempPath);
    if (!sink->IsOpen())
      return ValidationResult::kInternalServerError;

    parser_.SetBodySink(std::move(sink));
  }
  else
  {
    parser_.SetBodySink(std::make_unique<MemorySink>(request));
  }

  return std::nullopt;
}

std::optional<ValidationResult> Connection::AttachRequestCookies(HTTPRequest& request)
{
  if (HTTP::cookie::AttachCookies(request))
    return std::nullopt;

  return ValidationResult::kBadRequest;
}

std::optional<ValidationResult> Connection::HandleHeadersDone(RequestContext& requestContext)
{
  HTTPRequest& request = parser_.GetRequestMutable();

  if (std::optional<ValidationResult> validationError = ValidatePartialRequest(request))
    return validationError;

  MatchRouteAndApplyLimits(requestContext, request);

  if (std::optional<ValidationResult> storageError = ConfigureBodyStorage(request))
    return storageError;

  if (std::optional<ValidationResult> cookieError = AttachRequestCookies(request))
    return cookieError;

  InitBodyParser(parser_, request);
  return std::nullopt;
}

bool Connection::CheckConnectionClose(const HTTPRequest& request, const HTTPResponse& response) const
{
  return String::IsCloseToken(request.GetFirstHeaderValueOf("connection")) ||
         String::IsCloseToken(response.GetFirstHeaderValueOf("connection"));
}

/*********************************************** SOCKET I/O ***********************************************************/

Connection::ReadResult Connection::ReadOnce(std::string& out)
{
  out.clear();
  const ssize_t bytes = socket_.Recv(out);

  if (bytes == 0)
    return ReadResult::kClosed;
  if (bytes < 0)
    return ReadResult::kFatal;
  return ReadResult::kOk;
}
