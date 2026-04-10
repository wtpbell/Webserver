/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/13 19:08:02 by bewong        #+#    #+#                 */
/*   Updated: 2026/04/02 12:40:32 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

#include <sys/epoll.h>
#include <sys/socket.h>

#include <cassert>

#include "Connection.hpp"
#include "EpollManager.hpp"
#include "Logger.hpp"
#include "config/ServerRegistry.hpp"
#include "exception/ServerException.hpp"
#include "io/Socket.hpp"
#include "string.hpp"

/******************************************** Server Lifecycle ********************************************************/

Server::Server(const IpPort& ipPort, Socket::Type type, const ServerRegistry& serverRegistry)
    : ipPort_(ipPort),
      socket_(Socket::CreateSocket(ipPort.ip.c_str(), ipPort.port.c_str(), AI_PASSIVE, type)),
      serverRegistry_(serverRegistry)
{
  const int yes{1};
  const int no{0};

  socket_.SetNonBlocking(true);
  socket_.SetSockOpt(SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
  if (type == Socket::Type::kIPv6)
  {
    socket_.SetSockOpt(IPPROTO_IPV6, IPV6_V6ONLY, &no, sizeof(no));  // enable dual stack socket
  }
  socket_.Bind();
  socket_.Listen(SOMAXCONN);
  Logger::Log(LogLevel::INFO, "Server <{}> bound and is listening...", socket_);
}

Server::~Server(void)
{
  Logger::Log(LogLevel::INFO, "Server <{}> shutting down with {} active connections", socket_, connections_.size());
}

void Server::RegisterFD(EpollManager& manager)
{
  manager.AddFd(static_cast<int>(socket_), EPOLLIN | EPOLLERR,
                [this](EpollManager& manager2, const struct epoll_event& event)
                {
                  this->Accept(manager2, event);
                });
  Logger::Log(LogLevel::INFO, "Server <{}> registered...", socket_);
}

void Server::Accept(EpollManager& manager, const struct epoll_event& event)
{
  if ((event.events & EPOLLERR) == EPOLLERR)
  {
    manager.RemoveFd(socket_.GetFD());
    throw ServerException("Server", "broken socket");
  }

  const int max_accept = 100;
  for (int i = 0; i < max_accept; ++i)
  {
    Socket client{socket_.Accept4(SOCK_NONBLOCK)};
    if (client.GetFD() == -1)
      break;

    const int client_fd = client.GetFD();

    auto [it, inserted] = connections_.emplace(client_fd, Connection{std::move(client)});
    if (!inserted)
    {
      Logger::Log(LogLevel::CRITICAL, "Server <{}>: duplicate client <{}> found! Dropped the connection...", socket_,
                  client_fd);
      connections_.erase(it);
      continue;
    }

    try
    {
      auto callback = [this](EpollManager& manager, const struct epoll_event& event)
      {
        HandleConnection(manager, event);
      };

      manager.AddFd(client_fd, EPOLLERR | EPOLLHUP | EPOLLRDHUP | EPOLLIN, callback);
      Logger::Log(LogLevel::INFO, "Server <{}> accepted client connection <{}>", socket_, client_fd);
    }
    catch (const std::exception& ex)
    {
      connections_.erase(it);
      Logger::Log(LogLevel::ERROR,
                  "Server <{}> unable to register the client <{}> into the poll: {}. Dropping the connection!", socket_,
                  client_fd, ex.what());
    }
  }
}

void Server::HandleConnection(EpollManager& manager, const epoll_event& event)
{
  ConnectionIterator it = connections_.find(event.data.fd);
  if (it == connections_.end())
  {
    Logger::Log(LogLevel::WARNING, "Server <{}>: Connection FD {} not found", socket_, event.data.fd);
    manager.RemoveFd(event.data.fd);
    return;
  }

  auto& [fd, connection] = *it;
  if (event.events & (EPOLLHUP | EPOLLRDHUP))
  {
    connection.SetPeerClosed(true);
    if (!connection.HasPendingOutput() && !(event.events & (EPOLLIN | EPOLLOUT)))
    {
      Logger::Log(LogLevel::INFO, "Server <{}>: Closing connection with <{}>", socket_, connection.GetSocket());
      CloseConnection(manager, it);
      return;
    }
  }

  Connection::State connection_state =
      (event.events & EPOLLERR) == 0 ? Connection::State::kKeepAlive : Connection::State::kError;
  if (event.events & EPOLLIN && connection_state == Connection::State::kKeepAlive)
  {
    connection_state = connection.HandleRequest(ipPort_, router_, serverRegistry_, sessionManager_);
  }
  if (event.events & EPOLLOUT && connection_state == Connection::State::kKeepAlive)
  {
    connection_state = connection.HandleResponse();
  }

  if (connection.HasPendingOutput())
  {
    manager.ModifyFd(fd, EPOLLERR | EPOLLHUP | EPOLLRDHUP | EPOLLIN | EPOLLOUT);
  }
  else
  {
    manager.ModifyFd(fd, EPOLLERR | EPOLLHUP | EPOLLRDHUP | EPOLLIN);
  }

  if (connection_state == Connection::State::kError)
  {
    Logger::Log(LogLevel::INFO, "Server <{}>: Internal server error: Connection closed with client <{}>", socket_,
                connection.GetSocket());
    CloseConnection(manager, it);
  }

  if (connection_state == Connection::State::kClose)
  {
    Logger::Log(LogLevel::INFO, "Server <{}>: Connection closed with client <{}>", socket_, connection.GetSocket());
    CloseConnection(manager, it);
  }
}

void Server::CloseConnection(EpollManager& manager, ConnectionIterator connection)
{
  manager.RemoveFd(connection->first);
  connections_.erase(connection);
}
