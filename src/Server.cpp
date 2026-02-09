/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/13 19:08:02 by bewong        #+#    #+#                 */
/*   Updated: 2026/02/06 17:42:12 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

#include <sys/epoll.h>

#include <cassert>
#include <cstring>
#include <string>
#include <string_view>

#include "EpollManager.hpp"
#include "Logger.hpp"
#include "exception/ServerException.hpp"
#include "http/HTTPParser.hpp"
#include "http/HTTPRequest.hpp"
#include "http/HTTPResponse.hpp"
#include "http/HTTPStatus.hpp"
#include "http/HTTPTypes.hpp"
#include "http/HTTPUtils.hpp"
#include "http/HTTPValidator.hpp"
#include "io/Socket.hpp"
#include "string.hpp"

/******************************************** Server Lifecycle ********************************************************/

Server::Server(const char* service) : socket_(Socket::CreateSocket(nullptr, service, AI_PASSIVE))
{
  socket_.SetNonBlocking(true);
  const int yes{1};
  socket_.SetSockOpt(SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
  // By default dual stack socket is enabled on linux systems, but on other systems you might need to enable it
  const int no{0};
  socket_.SetSockOpt(IPPROTO_IPV6, IPV6_V6ONLY, &no, sizeof(no));
  socket_.Bind();

  Logger::Log(LogLevel::INFO, "Server succesfully created and is bound to the socket <{}>", socket_);
}

Server::~Server(void)
{
  Logger::Log(LogLevel::INFO, "Server <{}> shutting down with {} active connections", socket_, connections_.size());
}

void Server::ListenAndRegister(EpollManager& manager2, int max_connections)
{
  socket_.Listen(max_connections);
  manager2.AddFd(static_cast<int>(socket_), EPOLLIN | EPOLLERR,
                 [this](EpollManager& manager, const struct epoll_event& event)
                 {
                   this->Accept(manager, event);
                 });
  Logger::Log(LogLevel::INFO, "Server is now listening on socket <{}>", socket_);
}

void Server::Accept(EpollManager& manager, const struct epoll_event& event)
{
  // TODO: What should happen to the server? finish up the pending request or force close all the connections?
  if ((event.events & EPOLLERR) == EPOLLERR)
  {
    manager.RemoveFd(socket_.GetFD());
    throw ServerException("Server", "broken socket");
  }

  const int max_accept = 100;
  auto callback = [this](EpollManager& manager, const struct epoll_event& event)
  {
    this->HandleRequest(manager, event);
  };

  for (int i = 0; i < max_accept; ++i)
  {
    Socket client{socket_.Accept4(SOCK_NONBLOCK)};
    if (client.GetFD() == -1)
      break;

    auto [it, status] = connections_.emplace(client.GetFD(), Connection{client});
    if (!status)
    {
      Logger::Log(LogLevel::CRITICAL,
                  "Server <{}> tried to emplace for <{}> but an entry was already there and will now reset the "
                  "Connection state",
                  socket_, client);
      it->second.Clear();
      it->second.socket = std::move(client);
    }

    try
    {
      manager.AddFd(client.GetFD(), EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP, callback);
      Logger::Log(LogLevel::INFO, "Server <{}> accepted client connection <{}>", socket_, client);
    }
    catch (const std::exception& ex)
    {
      Logger::Log(LogLevel::ERROR,
                  "Server <{}> unable to register the client <{}> into the poll: {}. Dropping the connection!", socket_,
                  client, ex.what());
      // TODO: Send HTTP 5xx response directly before closing the connection
      CloseConnection(manager, it);
    }
  }
}

void Server::CloseConnection(EpollManager& manager, ConnectionIterator it)
{
  Logger::Log(LogLevel::INFO, "Server <{}> closing <{}> connection", socket_, it->first);
  manager.RemoveFd(it->first);
  connections_.erase(it);
}
/*********************************************** SOCKET I/O ***********************************************************/

Server::ReadResult Server::ReadOnce(Connection& c, std::string& out)
{
  out.clear();
  const ssize_t bytes = c.socket.Recv(out);

  if (bytes == 0)
    return ReadResult::Closed;
  if (bytes < 0)
    return ReadResult::Fatal;
  return ReadResult::Ok;
}
/******************************************** Epoll Callbacks *********************************************************/

void Server::EnableReadWrite(EpollManager& manager, int fd)
{
  manager.ModifyFd(fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLHUP,
                   [this](EpollManager& m, const epoll_event& ev)
                   {
                     if (ev.events & EPOLLIN)
                       HandleRequest(m, ev);
                     if (ev.events & EPOLLOUT)
                       HandleResponse(m, ev);
                   });
}

void Server::EnableReadOnly(EpollManager& manager, int fd)
{
  manager.ModifyFd(fd, EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP,
                   [this](EpollManager& m, const epoll_event& ev)
                   {
                     HandleRequest(m, ev);
                   });
}

/*************************************** Parsing / Validation / Body **************************************************/

// After ValidateRequest(req) == OK:
// - transfer-encoding is either absent or exactly "chunked"
// - transfer-encoding and content-length are not both present
// - content-length (if present) is valid and within limits
void Server::InitBodyParser(HTTPParser& parser, const HTTPRequest& req)
{
  if (req.HasHeader("transfer-encoding"))
    return parser.SetChunked();

  if (auto len = req.GetContentLength())
    return parser.SetContentLength(*len);

  parser.SetNoBody();
}

void Server::HandleHeadersDone(EpollManager& manager, ConnectionIterator it, Connection& connection, int client_fd)
{
  const HTTPRequest& partial = connection.parser.GetRequest();
  const ValidationResult vr = ValidateRequest(partial);

  if (vr != ValidationResult::OK)
  {
    Logger::Log(LogLevel::ERROR, "Validation error ({}) on req [{} {}] from client {}", static_cast<int>(vr),
                partial.GetMethodString(), partial.GetTarget(), client_fd);
    QueueError(manager, it, vr);
    connection.closeAfterSend = true;
    connection.parser.ResetNextRequest();
    return;
  }
  InitBodyParser(connection.parser, partial);
}

bool Server::CheckConnectionClose(const HTTPRequest* request, const HTTPResponse& response) const
{
  return String::IsCloseToken(request->GetFirstHeaderValueOf("connection")) ||
         String::IsCloseToken(response.GetFirstHeaderValueOf("connection"));
}

/********************************************* Queue Response *********************************************************/
void Server::QueueResponse(EpollManager& manager, ConnectionIterator it, HTTPResponse resp, bool closeAfter)
{
  auto& [fd, c] = *it;

  c.outputQueue.push_back(HTTP::wire::SerializeResponse(resp));
  c.closeAfterSend = c.closeAfterSend || closeAfter;

  // If we just queued the first item, reset send offset
  if (c.outputQueue.size() == 1)
    c.remaining = 0;

  // Always ensure EPOLLOUT is enabled while outputQueue is non-empty
  EnableReadWrite(manager, fd);
}

void Server::QueueError(EpollManager& manager, ConnectionIterator it, ValidationResult result)
{
  HTTPResponse response;
  response.SetStatus(HTTP::ToHTTPStatus(result));
  response.SetHeader("Content-Type", "text/plain");
  response.SetBody(response.GetReason() + "\n");
  response.SetHeader("Connection", "close");
  QueueResponse(manager, it, std::move(response), true);
}

/********************************************* Event Handlers *********************************************************/

HTTPResponse Server::DispatchRequest(const HTTPRequest& req)
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

void Server::HandleRequest(EpollManager& manager, const epoll_event& event)
{
  ConnectionIterator it = connections_.find(event.data.fd);
  if (it == connections_.end())
    return;

  auto& [client_fd, connection] = *it;

  if (event.events & EPOLLERR)
    return CloseConnection(manager, it);
  if (event.events & (EPOLLHUP | EPOLLRDHUP))
    connection.peerClosed = true;

  std::string msg;
  const ReadResult rr = ReadOnce(connection, msg);

  if (rr == ReadResult::Closed)
  {
    connection.peerClosed = true;
    if (connection.outputQueue.empty())
      return CloseConnection(manager, it);
    return;  // let EPOLLOUT flush
  }
  if (rr == ReadResult::Fatal)
    return CloseConnection(manager, it);
  if (rr == ReadResult::Empty)
    return;

  Logger::Log(LogLevel::LDEBUG, "Server {}: Received {} bytes from client {}", socket_, msg.size(), client_fd);

  std::string_view in = msg;

  while (true)
  {
    HTTPParser::ParseResult result = connection.parser.Parse(in);
    in = {};

    if (result == HTTPParser::ParseResult::NeedMoreData)
      return;
    if (result == HTTPParser::ParseResult::Error)
    {
      Logger::Log(LogLevel::ERROR, "Server {}: Bad request from client {} - Parser error", socket_, client_fd);
      QueueError(manager, it, connection.parser.GetError());
      connection.closeAfterSend = true;  // parser error: close
      return;
    }
    if (result == HTTPParser::ParseResult::HeadersDone)
    {
      HandleHeadersDone(manager, it, connection, client_fd);
      continue;
    }

    HTTPRequest req = connection.parser.TakeRequest();
    connection.parser.ResetNextRequest();
    HTTPResponse resp = DispatchRequest(req);
    const bool closeAfter = CheckConnectionClose(&req, resp);

    if (closeAfter)
      resp.SetHeader("Connection", "close");
    QueueResponse(manager, it, std::move(resp), closeAfter);
    if (closeAfter)
      return;  // stop processing further pipelined requests on this connection
  }
}

void Server::FinishResponse(EpollManager& manager, ConnectionIterator it)
{
  auto& [client_fd, connection] = *it;

  if (!connection.outputQueue.empty())
  {
    EnableReadWrite(manager, client_fd);
    return;
  }

  if (connection.closeAfterSend || connection.peerClosed)
    return CloseConnection(manager, it);

  EnableReadOnly(manager, client_fd);
}

void Server::HandleResponse(EpollManager& manager, const epoll_event& event)
{
  ConnectionIterator it = connections_.find(event.data.fd);
  if (it == connections_.end())
    return;

  auto& [client_fd, connection] = *it;

  if (event.events & EPOLLERR)
    return CloseConnection(manager, it);

  if ((event.events & (EPOLLHUP | EPOLLRDHUP)) != 0)
    connection.peerClosed = true;

  if (connection.outputQueue.empty())
    return FinishResponse(manager, it);

  const std::string& out = connection.outputQueue.front();
  if (connection.remaining == 0)
    connection.remaining = out.length();

  const ssize_t ret = connection.socket.Send(out, connection.remaining);
  if (ret < 0)
    return CloseConnection(manager, it);
  if (ret == 0)
    return;

  // If fully sent current front item, pop and continue finishing.
  if (connection.remaining == 0)
  {
    connection.outputQueue.pop_front();
    return FinishResponse(manager, it);
  }
}

void Server::Connection::Clear(void)
{
  outputQueue.clear();
  remaining = 0;
  closeAfterSend = false;
  peerClosed = false;

  socket.Reset();
  parser.Reset();
}
