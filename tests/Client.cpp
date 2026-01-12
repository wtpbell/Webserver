/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Client.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.ccodam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/01 15:03:21 by jboon         #+#    #+#                 */
/*   Updated: 2025/12/28 15:43:11 by jboon         ########   codam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <stdexcept>
#include <string>

#include "Logger.hpp"
#include "webserv.hpp"

Client::~Client(void)
{
  Logger::Log(LogLevel::INFO, "Client: Connection {} closed", socket_);
}

void Client::Connect(const char* host, const char* port)
{
  socket_ = Socket::CreateSocket(host, port);

  int flag = 1;
  socket_.SetSockOpt(IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
  socket_.SetSockOpt(IPPROTO_TCP, TCP_QUICKACK, &flag, sizeof(flag));

  // TODO: The server might need to set these settings for the incoming socket connections
  // https://www.extrahop.com/blog/tcp-nodelay-nagle-quickack-best-practices
  // https://brooker.co.za/blog/2024/05/09/nagle.html
  // int flag = 1;
  // client.SetSockOpt(IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
  // client.SetSockOpt(IPPROTO_TCP, TCP_QUICKACK, &flag, sizeof(flag));

  socket_.Connect();
}

bool Client::Ping(const std::string& message)
{
  std::size_t leftover{message.size()};
  while (leftover > 0 && socket_.Send(message, leftover) != -1)
    ;
  return (leftover == 0);
}

bool Client::Pong(void)
{
  std::string message;
  ssize_t bytes{};
  ssize_t prev{};

  while ((bytes = socket_.Recv(message)) > 0)
  {
    if (message.find('\n', static_cast<std::size_t>(prev)) != std::string::npos)
      break;
    prev += bytes;
  }
  std::cout << message << std::endl;
  return (bytes > 0);
}
