/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Connection.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/03/17 11:34:04 by jboon         #+#    #+#                 */
/*   Updated: 2026/04/07 09:53:24 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Connection.hpp"

#include <sys/epoll.h>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

#include "Logger.hpp"
#include "config/RouteView.hpp"
#include "config/ServerRegistry.hpp"
#include "http/HTTPCookie.hpp"
#include "http/HTTPRequest.hpp"
#include "http/HTTPResponse.hpp"
#include "http/HTTPUtils.hpp"
#include "http/HTTPValidator.hpp"
#include "http/ResponseFactory.hpp"
#include "http/SessionManager.hpp"
#include "router/Router.hpp"
#include "string.hpp"

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
  matchedHost_.clear();
  return State::kKeepAlive;
}

Connection::State Connection::ProcessInput(std::string_view in, const Router& router,
                                           const ServerRegistry& serverRegistry, SessionManager& sessionManager)
{
  while (true)
  {
    HTTPParser::ParseResult result = parser_.Parse(in);
    in = {};

    if (result == HTTPParser::ParseResult::kNeedMoreData)
      return State::kKeepAlive;

    if (result == HTTPParser::ParseResult::kError)
      return FailRequest(parser_.GetError());

    if (result == HTTPParser::ParseResult::kHeadersDone)
    {
      std::optional<ValidationResult> headerError = HandleHeadersDone(serverRegistry);
      if (headerError)
      {
        parser_.ResetNextRequest();
        return FailRequest(*headerError);
      }
      continue;
    }

    HTTPRequest request = parser_.TakeRequest();
    parser_.ResetNextRequest();

    const RouteView* route = matchedRoute_;
    const std::string hostName = matchedHost_;
    matchedRoute_ = nullptr;
    matchedHost_.clear();

    HTTPResponse response;
    if (route == nullptr)
      response = HTTP::response::MakeError(HTTP::Status::kNotFound);
    else
      response = router.Dispatch(request, *route, serverRegistry, ipport_, hostName);

    sessionManager.UseOrCreateSession(request, response);

    const bool closeAfter = CheckConnectionClose(request, response);
    if (closeAfter)
      response.SetHeader("Connection", "close");

    QueueResponse(response, closeAfter);

    if (closeAfter)
      return State::kKeepAlive;
  }
}

Connection::Connection(Socket socket)
    : socket_(std::move(socket)), matchedRoute_(nullptr), remaining_(0), closeAfterSend_(false), peerClosed_(false)
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

const Socket& Connection::GetSocket(void) const
{
  return socket_;
}

void Connection::Clear(void)
{
  outputQueue_.clear();
  remaining_ = 0;
  closeAfterSend_ = false;
  peerClosed_ = false;
  matchedRoute_ = nullptr;
  matchedHost_.clear();
  ipport_ = {};

  socket_.Reset();
  parser_.Reset();
}

/********************************************* Event Handlers *********************************************************/

Connection::State Connection::HandleRequest(const IpPort& ipport, const Router& router,
                                            const ServerRegistry& serverRegistry, SessionManager& sessionManager)
{
  ipport_ = ipport;

  std::string msg;
  const ReadResult result = ReadOnce(msg);
  if (result == ReadResult::kClosed)
  {
    peerClosed_ = true;
    return outputQueue_.empty() ? State::kClose : State::kKeepAlive;
  }
  if (result == ReadResult::kFatal)
    return State::kError;
  return ProcessInput(msg, router, serverRegistry, sessionManager);
}

Connection::State Connection::HandleResponse(void)
{
  if (outputQueue_.empty())
  {
    return (closeAfterSend_ || peerClosed_) ? State::kClose : State::kKeepAlive;
  }

  const std::string& out = outputQueue_.front();
  if (remaining_ == 0)
    remaining_ = out.length();

  const ssize_t ret = socket_.Send(out, remaining_);
  if (ret < 0)
  {
    return State::kError;
  }
  if (ret == 0)
  {
    return State::kKeepAlive;
  }

  // If fully sent current front item, pop and continue finishing.
  if (remaining_ == 0)
  {
    outputQueue_.pop_front();
    return (outputQueue_.empty() && (closeAfterSend_ || peerClosed_)) ? State::kClose : State::kKeepAlive;
  }
  return State::kKeepAlive;
}

/********************************************* Queue Response *********************************************************/

void Connection::QueueResponse(const HTTPResponse& response, bool closeAfter)
{
  std::string wire = HTTP::wire::SerializeResponse(response);

  if (!outputQueue_.empty())
    wire.insert(0, "\r\n");

  outputQueue_.push_back(std::move(wire));
  closeAfterSend_ = closeAfterSend_ || closeAfter;

  if (outputQueue_.size() == 1)
    remaining_ = 0;
}

void Connection::QueueError(ValidationResult result)
{
  HTTPResponse response;
  response.SetStatus(HTTP::ToHTTPStatus(result));
  response.SetHeader("Content-Type", "text/plain");
  response.SetBody(response.GetReason() + "\n");
  response.SetHeader("Connection", "close");
  QueueResponse(response, true);
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

void Connection::MatchRouteAndApplyLimits(const ServerRegistry& serverRegistry, const HTTPRequest& request)
{
  matchedHost_ = std::string(request.GetHost());
  const std::string targetPath(request.GetPath());

  Logger::Log(LogLevel::INFO, "lookup ip='{}' port='{}' host='{}' path='{}'", ipport_.ip, ipport_.port, matchedHost_,
              targetPath);

  matchedRoute_ = serverRegistry.GetRouteView(ipport_.ip, ipport_.port, matchedHost_, targetPath);

  if (matchedRoute_ != nullptr)
    parser_.SetMaxBody(matchedRoute_->clientMaxBody);
  else
    parser_.SetMaxBody(HTTP::kMaxBodySize);
}

std::optional<ValidationResult> Connection::ConfigureBodyStorage(HTTPRequest& request)
{
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

std::optional<ValidationResult> Connection::HandleHeadersDone(const ServerRegistry& serverRegistry)
{
  HTTPRequest& request = parser_.GetRequestMutable();

  if (std::optional<ValidationResult> validationError = ValidatePartialRequest(request))
    return validationError;

  MatchRouteAndApplyLimits(serverRegistry, request);

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
