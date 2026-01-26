/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/13 19:08:02 by bewong        #+#    #+#                 */
/*   Updated: 2026/01/15 16:20:20 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

#include <sys/epoll.h>

#include <cassert>
#include <cstring>
#include <string>

#include "Logger.hpp"
#include "exception/ServerException.hpp"
#include "http/HTTPParser.hpp"
#include "http/HTTPValidator.hpp"
#include "io/Socket.hpp"

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
      manager.AddFd(client.GetFD(), EPOLLIN, callback);
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

Server::ReadResult Server::ReadOnce(Connection& c, std::string& out, int client_fd)
{
  out.clear();
  const ssize_t bytes = c.socket.Recv(out);

  if (bytes == 0)
  {
    Logger::Log(LogLevel::INFO, "Server <{}> connection with <{}> got closed by client", socket_, client_fd);
    return ReadResult::Closed;
  }
  if (bytes < 0)
  {
    Logger::Log(LogLevel::ERROR, "Recv failed ({})! Server <{}> closing connection <{}>", errno, socket_, client_fd);
    return ReadResult::Fatal;
  }
  return ReadResult::Ok;
}

// After ValidateRequest(req) == OK:
// - transfer-encoding is either absent or exactly "chunked"
// - transfer-encoding and content-length are not both present
// - content-length (if present) is valid and within limits
void Server::InitBodyParser(HTTPParser& parser, const HTTPRequest& req)
{
  const auto* te = req.GetHeaderValuesOf("transfer-encoding");
  const auto* cl = req.GetHeaderValuesOf("content-length");

  const bool has_te = te && !te->empty();
  const bool has_cl = cl && !cl->empty();

  if (has_te)
  {
    parser.SetChunked();
    return;
  }
  if (has_cl)
  {
    auto len = req.GetContentLength();
    if (!len)
    {
      parser.SetNoBody();
      return;  // validation should have already returned 400
    }
    parser.SetContentLength(*len);
    return;
  }
  parser.SetNoBody();
}

bool Server::HandleHeadersDone(EpollManager& manager, Connection& connection, int client_fd)
{
  (void)manager;  // remove once QueueError/QueueBadRequest uses it
  const HTTPRequest& partial = connection.parser.GetRequest();
  const ValidationResult vr = ValidateRequest(partial);

  if (vr != ValidationResult::OK)
  {
    Logger::Log(LogLevel::ERROR, "Server {}: Validation error ({}) from client {}", socket_, static_cast<int>(vr),
                client_fd);
    // TODO: QueueError(manager, it, vr);
    return false;
  }
  InitBodyParser(connection.parser, partial);
  auto vResult = connection.parser.Parse({});  // continue consuming what we have left instead of append new bytes
  if (vResult == HTTPParser::ParseResult::NeedMoreData)
    return false;
  if (vResult == HTTPParser::ParseResult::Error)
  {
    Logger::Log(LogLevel::ERROR, "Server {}: Parser error after body decision from client {}", socket_, client_fd);
    // TODO: QueueBadRequest(manager, it);
    return false;
  }
  return (vResult == HTTPParser::ParseResult::Done);
}

void Server::HandleRequest(EpollManager& manager, const epoll_event& event)
{
  ConnectionIterator it = connections_.find(event.data.fd);
  if (it == connections_.end())
    return;
  if ((event.events & (EPOLLERR | EPOLLHUP)) != 0)
    return CloseConnection(manager, it);
  auto& [client_fd, connection] = *it;
  std::string msg;
  const ReadResult rr = ReadOnce(connection, msg, client_fd);

  if (rr == ReadResult::Empty)
    return;
  if (rr == ReadResult::Closed || rr == ReadResult::Fatal)
    return CloseConnection(manager, it);
  Logger::Log(LogLevel::LDEBUG, "Server {}: Received {} bytes from client {}", socket_, msg.size(), client_fd);
  HTTPParser::ParseResult result = connection.parser.Parse(msg);
  if (result == HTTPParser::ParseResult::NeedMoreData)
    return;
  if (result == HTTPParser::ParseResult::Error)
  {
    Logger::Log(LogLevel::ERROR, "Server {}: Bad request from client {} - Parser error", socket_, client_fd);
    // TODO: QueueBadRequest(manager, it);
    return;
  }
  if (result == HTTPParser::ParseResult::HeadersDone)
  {
    const bool done = HandleHeadersDone(manager, connection, client_fd); // TODO will return state later, now keep it returning bool
    if (!done)
      return;
    result = HTTPParser::ParseResult::Done;
  }
  // Done: take request
  connection.request.emplace(connection.parser.TakeRequest());
  const HTTPRequest& req = *connection.request;
  Logger::Log(LogLevel::INFO, "Server {}: {} {} from Client {}", socket_, req.GetMethodString(), req.GetRawPath(),
              client_fd);
  // TODO: DispatchRequest(...) -> QueueResponse(...) -> switch to EPOLLOUT
}

void Server::HandleResponse(EpollManager& manager, const struct epoll_event& event)
{
  ConnectionIterator it = connections_.find(event.data.fd);
  if (it == connections_.end())
    return;
  if ((event.events & (EPOLLERR | EPOLLHUP)) != 0)
    return (CloseConnection(manager, it));

  auto& [client_fd, connection] = *it;
  std::string body = "Hello";
  std::string msg =
      "HTTP/1.1 200 OK\r\n"
      "Content-Length: " +
      std::to_string(body.size()) +
      "\r\n"
      "Connection: close\r\n"
      "\r\n" +
      body;

  std::size_t leftover{msg.size()};  // TODO: retrieve from HTTP Message Response

  // construct the http reponse

  if (connection.socket.Send(msg, leftover) == -1)
    return (CloseConnection(manager, it));
  else if (leftover > 0)
    return;

  // TODO: Log received response message to client (without the body)

  auto callback = [this](EpollManager& manager, const struct epoll_event& event)
  {
    this->HandleRequest(manager, event);
  };
  manager.ModifyFd(client_fd, EPOLLIN, callback);  // client must close the connection or timeout
}

void Server::Connection::Clear(void)
{
  request.reset();
  socket.Reset();
  parser.Reset();
}
