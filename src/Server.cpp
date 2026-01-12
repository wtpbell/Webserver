/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.ccodam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/18 12:43:53 by jboon         #+#    #+#                 */
/*   Updated: 2026/01/06 10:39:13 by bewong        ########   codam.nl         */
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

void Server::HandleRequest(EpollManager& manager, const struct epoll_event& event)
{
  ConnectionIterator it = connections_.find(event.data.fd);
  if ((event.events & (EPOLLERR | EPOLLHUP)) != 0)
    return (CloseConnection(manager, it));

  auto& [client_fd, connection] = *it;
  std::string msg;  // TODO: retrieve from HTTP Message Request
  ssize_t bytes = connection.socket.Recv(msg);
  if (bytes < 1)
  {
    if (bytes == 0)
      Logger::Log(LogLevel::INFO, "Server <{}> connection with <{}> got closed by client", socket_, client_fd);
    else
      Logger::Log(LogLevel::ERROR, "Recv failed ({})! Server <{}> closing connection <{}>", errno, socket_, client_fd);
    return (CloseConnection(manager, it));
  }

  if (msg.empty())
  {
    Logger::Log(LogLevel::LDEBUG, "Server {}: Empty message received from client {}", socket_, client_fd);
    return;
  }

  Logger::Log(LogLevel::LDEBUG, "Server {}: Received {} bytes from client {}", socket_, msg.length(), client_fd);

  auto result = connection.parser.Parse(msg);

  if (result == HTTPParser::ParseResult::Error)
  {
    Logger::Log(LogLevel::ERROR, "Server {}: Bad request from client {} - Parser error", socket_, client_fd);
    // return QueueResponse(manager, client_fd, connection, HTTPResponse::MakeError(400, "Bad Request"));
    return;
  }
  if (result == HTTPParser::ParseResult::NeedMoreData)
  {
    Logger::Log(LogLevel::LDEBUG, "Server {}: Incomplete request from client {}, waiting for more data", socket_,
                client_fd);
    return;  // wait for more data
  }
  connection.request.emplace(connection.parser.TakeRequest());
  // TODO: build real response from request
  assert(connection.request.has_value());
  const HTTPRequest& req = *connection.request;
  Logger::Log(LogLevel::INFO, "Server {}: {} {} from Client {}", socket_, req.GetMethodString(), req.GetRawPath(),
              client_fd);
  // Only log 200 OK for successful requests
  Logger::Log(LogLevel::INFO, "Server {}: 200 OK sent to client {}", socket_, client_fd);
}

void Server::HandleResponse(EpollManager& manager, const struct epoll_event& event)
{
  ConnectionIterator it = connections_.find(event.data.fd);
  if ((event.events & (EPOLLERR | EPOLLHUP)) != 0)
    return (CloseConnection(manager, it));

  auto& [client_fd, connection] = *it;
  std::string msg{"Hello from Server\r\n"};
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
