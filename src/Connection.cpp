/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Connection.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/03/17 11:34:04 by jboon         #+#    #+#                 */
/*   Updated: 2026/04/01 20:51:29 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Connection.hpp"

#include <sys/epoll.h>

#include <string>
#include <string_view>

#include "Logger.hpp"
#include "config/ServerRegistry.hpp"
#include "http/HTTPCookie.hpp"
#include "http/HTTPRequest.hpp"
#include "http/HTTPResponse.hpp"
#include "http/HTTPUtils.hpp"
#include "http/HTTPValidator.hpp"
#include "http/SessionManager.hpp"
#include "string.hpp"

void Connection::SetPeerClosed(bool close)
{
  peerClosed_ = close;
}

const Socket& Connection::GetSocket(void) const
{
  return socket_;
}

bool Connection::HasPendingOutput(void) const
{
  return !outputQueue_.empty();
}

void Connection::Clear(void)
{
  outputQueue_.clear();
  remaining_ = 0;
  closeAfterSend_ = false;
  peerClosed_ = false;

  socket_.Reset();
  parser_.Reset();
}

/********************************************* Event Handlers *********************************************************/

HTTPResponse Connection::DispatchRequest(const HTTPRequest& req)
{
  HTTPResponse r;

  switch (req.GetMethod())
  {
    case HTTP::Method::GET:
      r.SetStatus(HTTP::Status::OK);
      r.SetHeader("Content-Type", "text/plain");
      r.SetBody("Success\n");
      return r;

    case HTTP::Method::POST:
      r.SetStatus(HTTP::Status::NO_CONTENT);
      return r;

    case HTTP::Method::DELETE:
    case HTTP::Method::UNSUPPORTED:
      break;
  }
  r.SetStatus(HTTP::Status::NOT_IMPLEMENTED);
  r.SetHeader("Content-Type", "text/plain");
  r.SetBody("Method Not Implemented\n");
  return r;
}

Connection::State Connection::HandleRequest(const IpPort& ipport, const Router& router,
                                            const ServerRegistry& serverRegistry, SessionManager& session_manager)
{
  // TODO: Make use of these members
  (void)ipport;
  (void)router;
  (void)serverRegistry;

  std::string msg;
  const ReadResult rr = ReadOnce(msg);
  if (rr == ReadResult::kClosed)
  {
    peerClosed_ = true;
    if (outputQueue_.empty())
    {
      return State::kClose;
    }
    return State::kKeepAlive;
  }
  else if (rr == ReadResult::kFatal)
  {
    return State::kError;
  }

  std::string_view in = msg;
  while (true)
  {
    HTTPParser::ParseResult result = parser_.Parse(in);
    in = {};

    if (result == HTTPParser::ParseResult::NeedMoreData)
    {
      break;
    }
    if (result == HTTPParser::ParseResult::Error)
    {
      Logger::Log(LogLevel::ERROR, "Connection: Bad request from client {} - Parser error", socket_);
      QueueError(parser_.GetError());
      closeAfterSend_ = true;  // parser error: close
      return State::kKeepAlive;
    }
    if (result == HTTPParser::ParseResult::HeadersDone)
    {
      HandleHeadersDone();
      continue;
    }

    HTTPRequest req = parser_.TakeRequest();
    parser_.ResetNextRequest();
    HTTPResponse resp = DispatchRequest(req);
    session_manager.UseOrCreateSession(req, resp);
    const bool closeAfter = CheckConnectionClose(&req, resp);

    if (closeAfter)
    {
      resp.SetHeader("Connection", "close");
    }
    QueueResponse(resp, closeAfter);
    if (closeAfter)
    {
      return State::kKeepAlive;  // stop processing further pipelined requests on this connection
    }
  }
  return State::kKeepAlive;
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

void Connection::QueueResponse(const HTTPResponse& resp, bool closeAfter)
{
  outputQueue_.push_back(HTTP::wire::SerializeResponse(resp));
  closeAfterSend_ = closeAfterSend_ || closeAfter;

  // If we just queued the first item, reset send offset
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

// After ValidateRequest(req) == OK:
// - transfer-encoding is either absent or exactly "chunked"
// - transfer-encoding and content-length are not both present
// - content-length (if present) is valid and within limits
void Connection::InitBodyParser(HTTPParser& parser, const HTTPRequest& req)
{
  if (req.HasHeader("transfer-encoding"))
    return parser.SetChunked();

  if (auto len = req.GetContentLength())
    return parser.SetContentLength(*len);

  parser.SetNoBody();
}

void Connection::HandleHeadersDone(void)
{
  HTTPRequest& partial = parser_.GetRequestMutable();
  const ValidationResult vr = ValidateRequest(partial);

  if (vr != ValidationResult::OK)
  {
    Logger::Log(LogLevel::ERROR, "Connection: Validation error ({}) on req [{} {}] from client {}",
                static_cast<int>(vr), partial.GetMethodString(), partial.GetTarget(), socket_.GetFD());
    QueueError(vr);
    closeAfterSend_ = true;
    parser_.ResetNextRequest();
    return;
  }
  if (!HTTP::cookie::AttachCookies(partial))
  {
    QueueError(ValidationResult::BadRequest);
    closeAfterSend_ = true;
    parser_.ResetNextRequest();
    return;
  }
  InitBodyParser(parser_, partial);
}

bool Connection::CheckConnectionClose(const HTTPRequest* request, const HTTPResponse& response) const
{
  return String::IsCloseToken(request->GetFirstHeaderValueOf("connection")) ||
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
