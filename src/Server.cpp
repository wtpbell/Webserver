/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/18 12:43:53 by jboon         #+#    #+#                 */
/*   Updated: 2025/12/15 14:22:38 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

#include <fcntl.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <cstring>
#include <string>

#include "Logger.hpp"
#include "exception/EPollManagerException.hpp"
#include "exception/ServerException.hpp"
#include "webserv.hpp"

Server::addrinfo_t* Server::GetAddrinfo(std::string_view service)
{
  addrinfo_t* result;
  addrinfo_t hints{};

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

void Server::ListenAndRegister(EpollManager& manager, int max_connections)
{
  if (listen(socket_fd_, max_connections) == -1)
    throw ServerException("listen", errno);
  manager.AddFd(socket_fd_, EPOLLIN | EPOLLERR,
                [this](EpollManager& manager, const struct epoll_event& event)
                {
                  this->Accept(manager, event);
                });
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
    Logger::Log(
        LogLevel::WARNING,
        "Server {} tried to emplace for {} but an entry was already there and will now reset the Connection state",
        socket_fd_, client_fd);
  }

  uint32_t events = EPOLLIN;
  auto callback = [this](EpollManager& manager, const struct epoll_event& event)
  {
    this->HandleRequest(manager, event);
  };

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
    CloseConnection(manager, it);
    return;
  }
  it->second.client_info_ = GetSocketInfo(client_fd);
  Logger::Log(LogLevel::INFO, "Server {} {} accepted socket connection {} {}", socket_fd_, socket_info_, client_fd,
              it->second.client_info_);
}

void Server::CloseConnection(EpollManager& manager, ConnectionIterator it)
{
  Logger::Log(LogLevel::INFO, "Server {} closing {} {} connection", socket_fd_, it->first, it->second.client_info_);
  manager.RemoveFd(it->first);
  close(it->first);
  connections_.erase(it);
}

bool Server::Recv(int fd, std::string& msg, std::size_t max_chunk_size)
{
  constexpr std::size_t buf_size = 1024;
  char buf[buf_size];

  assert(max_chunk_size >= buf_size && "buf_size is greater than maximum chunk size");
  for (std::size_t total{}; total < max_chunk_size;)
  {
    ssize_t bytes = recv(fd, buf, buf_size, 0);
    if (bytes == 0)
      return (false);  // Connection close by client => close connection
    else if (bytes == -1)
      return (true);  // everything read from buffer and is now giving EAGAIN OR EWOULDBLOCK
    msg.append(buf, static_cast<std::size_t>(bytes));
    total += static_cast<std::size_t>(bytes);
  }
  return (true);  // reached maximum chunk size (try again next tick)
}

bool Server::Send(int fd, const std::string& msg, std::size_t& leftover, std::size_t max_chunk_size)
{
  const char* buf = msg.data() + (msg.size() - leftover);

  for (std::size_t bytes_sent{}; bytes_sent < max_chunk_size && leftover > 0;)
  {
    std::size_t chunk = leftover > max_chunk_size ? max_chunk_size : leftover;
    ssize_t bytes = send(fd, buf, chunk, 0);
    if (bytes == 0)
      return (false);  // Connection closed by client => close connection
    else if (bytes == -1)
      return (true);  // EAGAIN OR EWOULDBLOCK => send next tick (any other error will be picked up by the poll manager)

    bytes_sent += static_cast<std::size_t>(bytes);
    leftover -= static_cast<std::size_t>(bytes);
    buf += static_cast<std::size_t>(bytes);
  }
  return (true);  // reached the maximum chunk size => send next tick
}

void Server::HandleRequest(EpollManager& manager, const struct epoll_event& event)
{
  ConnectionIterator it = connections_.find(event.data.fd);
  if ((event.events & (EPOLLERR | EPOLLHUP)) != 0)
    return (CloseConnection(manager, it));

  auto& [client_fd, connection] = *it;
  std::string msg;  // TODO: retrieve from HTTP Message Request

  if (!Recv(client_fd, msg))
  {
    Logger::Log(LogLevel::INFO, "Server {} connection with {} got closed by client", socket_fd_, client_fd);
    return (CloseConnection(manager, it));
  }

  if (msg.size() == 0 || msg.find('\n') == std::string::npos)
    return;

  // try parse and update parser state
  //  find empty newline to handle the header/meta data part

  // TODO: Log received request message from client

  // parse of the data complete goto next state
  auto callback = [this](EpollManager& manager, const struct epoll_event& event)
  {
    this->HandleResponse(manager, event);
  };
  manager.ModifyFd(client_fd, EPOLLOUT, callback);
}

void Server::HandleResponse(EpollManager& manager, const struct epoll_event& event)
{
  ConnectionIterator it = connections_.find(event.data.fd);
  if ((event.events & (EPOLLERR | EPOLLHUP)) != 0)
    return (CloseConnection(manager, it));

  auto& [client_fd, connection] = *it;
  std::string msg{"Hello from Server " + socket_info_ + "\r\n"};
  std::size_t tmp{msg.size()};  // TODO: retrieve from HTTP Message Response

  // construct the http reponse

  if (!Send(client_fd, msg, tmp))
    return (CloseConnection(manager, it));

  // TODO: Log received response message to client (without the body)

  auto callback = [this](EpollManager& manager, const struct epoll_event& event)
  {
    this->HandleRequest(manager, event);
  };
  manager.ModifyFd(client_fd, EPOLLIN, callback);  // client must close the connection or timeout
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
  // TODO: Server has no access to EPollManager... How will the deconstructor remove the entry from the poll queue?
  // Closing fds does implicitly remove them from the poll queue, but only if all references to the file have been
  // closed (think about dup fds for example)
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

void Server::Connection::Clear(void)
{
  request.reset();
  response.reset();
  client_info_.clear();
  bytes_remaining = 0;
}
