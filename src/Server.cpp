/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/18 12:43:53 by jboon         #+#    #+#                 */
/*   Updated: 2025/12/11 23:12:08 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

#include <fcntl.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>

#include "Logger.hpp"
#include "exception/EPollManagerException.hpp"
#include "exception/ServerException.hpp"
#include "webserv.hpp"

Server::addrinfo_t* Server::GetAddrinfo(std::string_view service)
{
  addrinfo_t hints;
  addrinfo_t* result;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int status = getaddrinfo(NULL, service.data(), &hints, &result);
  if (status != 0)
    throw ServerException("GetAddrinfo", gai_strerror(status));
  return (result);
}

// TODO: look into dual stack socket (Linux defaults to dual stack and for Windows you must set the flag (IPV6_V6ONLY));
// however, if it is not supported what should happen next?
int Server::SocketAndBind(const addrinfo_t* result)
{
  for (const addrinfo_t* curr = result; curr != nullptr; curr = curr->ai_next)
  {
    int sfd = socket(curr->ai_family, curr->ai_socktype, curr->ai_protocol);
    if (sfd == -1)
    {
      Logger::Log(LogLevel::WARNING, "Failed to create socket for {}", GetNameInfo(curr->ai_addr, curr->ai_addrlen));
      continue;
    }
    if (fcntl(sfd, F_SETFL, O_NONBLOCK) == -1)
    {
      Logger::Log(LogLevel::WARNING, "Failed to set O_NONBLOCK for socket {}", GetSocketInfo(sfd));
      close(sfd);
      continue;
    }

    const int yes = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
    {
      Logger::Log(LogLevel::WARNING, "Failed to set SO_REUSEADDR for socket {}", GetSocketInfo(sfd));
      close(sfd);
      continue;
    }
    if (bind(sfd, curr->ai_addr, curr->ai_addrlen) == -1)
    {
      Logger::Log(LogLevel::WARNING, "Failed to bind socket to {}", GetSocketInfo(sfd));
      close(sfd);
      continue;
    }
    return sfd;
  }
  return (-1);
}

void Server::ListenAndRegister(EpollManager& manager, int max_connections = SOMAXCONN)
{
  if (listen(socket_fd_, max_connections) == -1)
    throw ServerException("listen", errno);
  manager.AddFd(socket_fd_, EPOLLIN | EPOLLERR,
                [this](EpollManager& manager, const struct epoll_event& event) { this->Accept(manager, event); });
  Logger::Log(LogLevel::INFO, "Server {} is now listening on socket: {}", socket_fd_, socket_info_);
}

void Server::Accept(EpollManager& manager, const struct epoll_event&)
{
  struct sockaddr_storage client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  int client_fd = accept4(socket_fd_, (struct sockaddr*)&client_addr, &client_addr_len, SOCK_NONBLOCK);
  if (client_fd == -1)
    throw ServerException("Accept", errno);

  auto [it, status] = connections_.emplace(client_fd, Connection{});
  if (!status)
  {
    it->second.Clear();
    Logger::Log(
        LogLevel::WARNING,
        "Server {} tried to emplace for {} but an entry was already there and will now reset the Connection state",
        socket_fd_, client_fd);
  }

  uint32_t events = EPOLLIN | EPOLLOUT;
  auto callback = [this](EpollManager& manager, const struct epoll_event& event) { this->Recv(manager, event); };
  try
  {
    manager.AddFd(client_fd, events, callback);
  }
  catch (const ExistInEPollException& e)
  {
    // TODO: what is next if modify fails?
    manager.ModifyFd(client_fd, events, callback);
    Logger::Log(LogLevel::WARNING, "Server {} overwriten an existing fd event and callback {} in the EPollManager",
                socket_fd_, client_fd);
  }
  catch (const std::exception& e)
  {
    Logger::Log(LogLevel::ERROR, "Server {} failed to add {} {} to the epollmanager and has dropped the connection: {}",
                socket_fd_, client_fd, GetSocketInfo(client_fd), e.what());
    connections_.erase(it);
    close(client_fd);
    return;
  }
  Logger::Log(LogLevel::INFO, "Server {} {} accepted socket connection {} {}", socket_fd_, socket_info_, client_fd,
              GetSocketInfo(client_fd));
}

void Server::Recv(EpollManager& manager, const struct epoll_event& event)
{
  (void)manager;
  (void)event;
  throw std::runtime_error("Recv: Implementation needed!");
}

void Server::Send(EpollManager& manager, const struct epoll_event& event)
{
  (void)manager;
  (void)event;
  throw std::runtime_error("Send: Implementation needed!");
}

Server::Server(std::string_view service)
{
  addrinfo_t* result = GetAddrinfo(service);
  socket_fd_ = SocketAndBind(result);
  freeaddrinfo(result);
  if (socket_fd_ == -1)
    throw ServerException("SocketAndBind", "Failed to create and bind a socket for this server!");
  socket_info_ = GetSocketInfo(socket_fd_);
  Logger::Log(LogLevel::INFO, "Server {} succesfully created and is bound to the socket {}", socket_fd_, socket_info_);
}

Server::~Server(void)
{
  for (const auto& [client_fd, connection] : connections_)
  {
    Logger::Log(LogLevel::INFO, "Server {} {} closing client {} connection {}", socket_fd_, socket_info_, client_fd,
                GetSocketInfo(client_fd));
    if (close(client_fd) == -1)
      Logger::Log(LogLevel::ERROR, "Server {} failed to close client {}: {}", socket_fd_, client_fd, strerror(errno));
  }

  if (close(socket_fd_) == 0)
    Logger::Log(LogLevel::INFO, "Server {} {} closed the socket", socket_fd_, socket_info_);
  else
    Logger::Log(LogLevel::ERROR, "Server {} was unable to close the socket: {}", socket_fd_, strerror(errno));
}
