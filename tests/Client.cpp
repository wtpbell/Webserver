/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Client.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/01 15:03:21 by jboon         #+#    #+#                 */
/*   Updated: 2025/12/09 11:28:43 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <stdexcept>

#include "Logger.hpp"
#include "webserv.hpp"

Client::~Client(void)
{
  Logger::Log(LogLevel::INFO, "Client {} closing its connection {}", clientfd_, GetSocketInfo(clientfd_));
  Shutdown();
}

void Client::Connect(const char* host, const char* port)
{
  struct addrinfo hints, *res;
  int sockfd;
  int status;

  std::memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((status = getaddrinfo(host, port, &hints, &res)) != 0)
    throw std::runtime_error(gai_strerror(status));
  if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1 ||
      connect(sockfd, res->ai_addr, res->ai_addrlen) == -1 || fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1)
  {
    if (sockfd != -1)
      close(sockfd);
    freeaddrinfo(res);
    throw std::runtime_error(strerror(errno));
  }
  freeaddrinfo(res);
  clientfd_ = sockfd;
  Logger::Log(LogLevel::INFO, "Client {} established a connection with {}", clientfd_, GetSocketInfo(clientfd_));
}

void Client::Ping(const char* msg)
{
  int total = 0;
  int len = std::strlen(msg);
  int bytesleft = len;
  int n;

  while (total < len)
  {
    if ((n = send(clientfd_, msg + total, bytesleft, MSG_DONTWAIT)) == -1)
      throw std::runtime_error(strerror(errno));
    total += n;
    bytesleft -= n;
  }
}

void Client::Pong(void)
{
  const std::size_t buf_size = 1024;

  std::string msg;
  char buf[buf_size];

  for (;;)
  {
    ssize_t bytes = recv(clientfd_, buf, buf_size, 0);
    if (bytes == -1)
      break;
    else if (bytes == 0)
      throw std::runtime_error(strerror(errno));
    msg.append(buf, static_cast<std::size_t>(bytes));
  }
  std::cout << msg << std::endl;
}

void Client::Shutdown(void)
{
  if (clientfd_ == -1)
    return;
  shutdown(clientfd_, SHUT_RDWR);
  close(clientfd_);
  clientfd_ = -1;
}
