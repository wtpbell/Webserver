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

bool Server::RegisterFD(EpollManager& manager)
{
  EpollManager::Result result = manager.AddFd(static_cast<int>(socket_), EPOLLIN | EPOLLERR,
                                              [this](EpollManager& manager2, const struct epoll_event& event)
                                              {
                                                this->Accept(manager2, event);
                                              });

  if (result != EpollManager::Result::kOk)
  {
    Logger::Log(LogLevel::ERROR, "Server <{}> failed to register: {}", socket_, EpollManager::ToString(result));
    return false;
  }

  Logger::Log(LogLevel::INFO, "Server <{}> registered...", socket_);
  return true;
}

void Server::Accept(EpollManager& manager, const struct epoll_event& event)
{
  if ((event.events & EPOLLERR) == EPOLLERR)
  {
    Logger::Log(LogLevel::ERROR, "Server <{}>: listener socket error", socket_);
    manager.RemoveFd(socket_.GetFD());
    return;
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

    auto callback = [this](EpollManager& manager2, const struct epoll_event& ev)
    {
      this->HandleConnection(manager2, ev);
    };

    EpollManager::Result result = manager.AddFd(client_fd, EPOLLERR | EPOLLHUP | EPOLLRDHUP | EPOLLIN, callback);

    if (result != EpollManager::Result::kOk)
    {
      Logger::Log(LogLevel::ERROR, "Server <{}> unable to register client <{}> into epoll: {}. Dropping connection!",
                  socket_, client_fd, EpollManager::ToString(result));
      connections_.erase(it);
      continue;
    }

    Logger::Log(LogLevel::INFO, "Server <{}> accepted client connection <{}>", socket_, client_fd);
  }
}

void Server::HandleConnection(EpollManager& manager, const epoll_event& event)
{
  ConnectionIterator it = connections_.find(event.data.fd);
  if (it == connections_.end())
  {
    Logger::Log(LogLevel::WARNING, "Server <{}>: Connection FD {} not found", socket_, event.data.fd);
    EpollManager::Result removeResult = manager.RemoveFd(event.data.fd);
    if (removeResult != EpollManager::Result::kOk)
    {
      Logger::Log(LogLevel::WARNING, "Server <{}>: failed to remove unknown FD {} from epoll: {}", socket_,
                  event.data.fd, EpollManager::ToString(removeResult));
    }
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

  EpollManager::Result modResult;
  if (connection.HasPendingOutput())
  {
    modResult = manager.ModifyFd(fd, EPOLLERR | EPOLLHUP | EPOLLRDHUP | EPOLLIN | EPOLLOUT);
  }
  else
  {
    modResult = manager.ModifyFd(fd, EPOLLERR | EPOLLHUP | EPOLLRDHUP | EPOLLIN);
  }

  if (modResult != EpollManager::Result::kOk)
  {
    Logger::Log(LogLevel::ERROR, "Server <{}>: failed to update epoll events for client <{}>: {}", socket_, fd,
                EpollManager::ToString(modResult));
    CloseConnection(manager, it);
    return;
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
  EpollManager::Result result = manager.RemoveFd(connection->first);
  if (result != EpollManager::Result::kOk && result != EpollManager::Result::kNotFound)
  {
    Logger::Log(LogLevel::WARNING, "Server <{}>: failed to remove client <{}> from epoll: {}", socket_,
                connection->first, EpollManager::ToString(result));
  }

  connections_.erase(connection);
}
