/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   socket_info.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/18 15:47:14 by jboon         #+#    #+#                 */
/*   Updated: 2025/12/01 16:11:08 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <arpa/inet.h>
#include <netdb.h>

#include <Logger.hpp>
#include <cstring>
#include <string>

std::string GetNameInfo(struct sockaddr* addr, socklen_t addrlen)
{
  std::string str;
  char host[NI_MAXHOST];
  char serv[NI_MAXSERV];

  int status = getnameinfo(addr, addrlen, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
  if (status != 0)
  {
    Logger::Log(LogLevel::WARNING, "Failed to execute getnameinfo: {}", gai_strerror(status));
    return (std::string("UNKNOWN NAMEINFO"));
  }
  str.reserve(NI_MAXSERV + NI_MAXHOST + 1);
  str += host;
  str += ":";
  str += serv;
  return (str);
}

std::string GetSocketInfo(int sfd)
{
  struct sockaddr_storage addr_storage;
  socklen_t addrlen = sizeof(addr_storage);

  if (getsockname(sfd, (struct sockaddr*)&addr_storage, &addrlen) == -1)
  {
    Logger::Log(LogLevel::WARNING, "Failed to execute getsockname on fd {}: {}", sfd, strerror(errno));
    return std::string("UNKNOWN SOCKNAME");
  }
  return (GetNameInfo((struct sockaddr*)&addr_storage, addrlen));
}
