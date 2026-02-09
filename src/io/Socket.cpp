/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Socket.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/11 15:17:37 by jboon         #+#    #+#                 */
/*   Updated: 2026/01/13 19:08:25 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "io/Socket.hpp"

#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cerrno>
#include <charconv>
#include <cstring>
#include <ostream>
#include <string>

#include "exception/FileDescriptorException.hpp"
#include "io/SharedFD.hpp"
#include "string.hpp"

namespace
{
  /// @brief Checks if the port number is within the range of 0 and 65535
  /// @note Code found from https://stackoverflow.com/a/56634586
  bool IsValidPortNumber(std::string_view port)
  {
    int result{};
    const char* end = port.data() + port.length();
    auto [ptr, ec] = std::from_chars(port.data(), port.data() + port.length(), result);
    if (ec != std::errc() || ptr != end)
      return (false);
    return (result >= 0 && result <= 65535);
  }

  /// @brief Check if port is a valid argument for getaddrinfo
  bool IsValidService(const char* port)
  {
    if (String::IsEmptyOrNull(port) || !String::IsDigitOnly(port))
      return (true);
    return (IsValidPortNumber(port));
  }
}  // namespace

Socket Socket::CreateSocket(int domain, int type, int protocol)
{
  int socket_fd = socket(domain, type, protocol);
  if (socket_fd == -1)
    throw FileDescriptorException("CreateSocket", errno);
  return (Socket(socket_fd));
}

/**
 * @brief Create an IPv6 TCP socket for the specificed service (port)
 */
Socket Socket::CreateSocket(const char* host, const char* service, int flags)
{
  addrinfo_t hints{};

  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = flags;

  addrinfo_t* addr_info = GetAddrInfo(hints, host, service);
  for (const addrinfo_t* curr = addr_info; curr != nullptr; curr = curr->ai_next)
  {
    int socket_fd = socket(curr->ai_family, curr->ai_socktype, curr->ai_protocol);
    if (socket_fd == -1)
      continue;
    Socket sock = Socket(socket_fd, curr->ai_addr, curr->ai_addrlen);
    freeaddrinfo(addr_info);
    return (sock);
  }

  freeaddrinfo(addr_info);
  throw FileDescriptorException("CreateSocket", errno);
}

std::pair<Socket, Socket> Socket::SocketPair(int domain, int type, int protocol)
{
  int sv[2];
  if (socketpair(domain, type, protocol, sv) == -1)
    throw FileDescriptorException("socketpair", errno);
  return {Socket(sv[0]), Socket(sv[1])};
}

Socket::addrinfo_t* Socket::GetAddrInfo(addrinfo_t& hints, const char* host, const char* service)
{
  if (!IsValidService(service))
    throw FileDescriptorException("GetAddrInfo", "The `service` argument is not valid");

  addrinfo_t* result;
  int status = getaddrinfo(host, service, &hints, &result);
  if (status != 0)
    throw FileDescriptorException("GetAddrInfo", gai_strerror(status));
  return (result);
}

void Socket::Bind(void)
{
  if (bind(fd_, address_.GetAddrPointer(), address_.addrlen) == -1)
    throw FileDescriptorException("Bind", errno);
}

void Socket::Listen(int backlog)
{
  if (listen(fd_, backlog) == -1)
    throw FileDescriptorException("Listen", errno);
}

void Socket::Connect(void)
{
  if (connect(fd_, address_.GetAddrPointer(), address_.addrlen) == -1)
    throw FileDescriptorException("Connect", errno);
}

Socket Socket::Accept4(int flags)
{
  SocketAddress address{};

  int client_fd = accept4(fd_, address.GetAddrPointer(), address.GetAddrlenPointer(), flags);
  if (client_fd == -1)
  {
    if ((errno == EAGAIN || errno == EWOULDBLOCK) && is_non_blocking_)
      return (Socket());
    throw FileDescriptorException("Accept4", errno);
  }
  return (Socket(client_fd, address, ((flags & SOCK_NONBLOCK) == SOCK_NONBLOCK)));
}

void Socket::SetSockOpt(int level, int optname, const void* optval, socklen_t optlen)
{
  if (setsockopt(fd_, level, optname, optval, optlen) == -1)
    throw FileDescriptorException("SetSockOpt", errno);
}

void Socket::SetNonBlocking(bool is_non_blocking)
{
  int flags = fcntl(fd_, F_GETFL);
  if (flags == -1)
    throw FileDescriptorException("SetNonBlocking", errno);

  if (is_non_blocking)
    flags = (flags | O_NONBLOCK);
  else
    flags = (flags & (~O_NONBLOCK));

  if (fcntl(fd_, F_SETFL, flags) == -1)
    throw FileDescriptorException("SetNonBlocking", errno);

  is_non_blocking_ = is_non_blocking;
}

/**
 * @brief System call to recv and append the data onto msg
 * @return bytes read by recv or -1 if an error occurred
 */
ssize_t Socket::Recv(std::string& message, int flags, std::size_t max_chunk_size)
{
  std::size_t len = message.length();
  message.resize(message.length() + max_chunk_size);

  ssize_t bytes = recv(fd_, (message.data() + len), max_chunk_size, flags);
  if (bytes > 0)
    message.resize(len + bytes);
  return (bytes);
}

/**
 * @brief Send the message up to max_chunk_size amount of bytes by using the send system call.
 * Updates leftover by substracting the amount of bytes that were sent.
 * @return total amount of bytes sent or -1 if an error occurred
 */
// ssize_t Socket::Send(const std::string& msg, std::size_t& leftover, int flags, std::size_t max_chunk_size)
// {
//   return Send(std::string_view(msg), leftover, flags, max_chunk_size);
// }

ssize_t Socket::Send(std::string_view message, std::size_t& leftover, int flags, std::size_t max_chunk_size)
{
  const char* buf = message.data() + (message.size() - leftover);

  ssize_t bytes = send(fd_, buf, std::min(leftover, max_chunk_size), flags);
  if (bytes > 0)
    leftover -= static_cast<std::size_t>(bytes);
  return (bytes);
}

std::string_view Socket::GetSocketInfo(void) const
{
  return (info_);
}

std::ostream& operator<<(std::ostream& os, const Socket& socket)
{
  return (os << socket.fd_ << ", " << socket.info_);
}

/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ PRIVATE +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */

Socket::Socket(int socket_fd, bool is_non_blocking) : SharedFD(socket_fd), is_non_blocking_(is_non_blocking) {}

Socket::Socket(int socket_fd, const SocketAddress& address, bool is_non_blocking)
    : SharedFD(socket_fd), address_(address), is_non_blocking_(is_non_blocking)
{
  info_ = UpdateNameInfo();
}

Socket::Socket(int socket_fd, struct sockaddr* addr, socklen_t addrlen) : SharedFD(socket_fd)
{
  std::memmove(address_.GetAddrPointer(), addr, addrlen);
  address_.addrlen = addrlen;
  info_ = UpdateNameInfo();
}

std::string Socket::UpdateNameInfo(void) const
{
  std::string str;
  char host[NI_MAXHOST];
  char serv[NI_MAXSERV];

  int status = getnameinfo(address_.GetAddrPointer(), address_.addrlen, host, NI_MAXHOST, serv, NI_MAXSERV,
                           NI_NUMERICHOST | NI_NUMERICSERV);
  if (status != 0)
    throw FileDescriptorException("GetNameInfo", gai_strerror(status));

  str.reserve(NI_MAXSERV + NI_MAXHOST + 1);
  str += host;
  str += ":";
  str += serv;

  return (str);
}

// TODO: What to do with this function??
void Socket::GetSockName(SocketAddress& address) const
{
  if (getsockname(fd_, address.GetAddrPointer(), address.GetAddrlenPointer()) == -1)
    throw FileDescriptorException("GetSocketInfo", errno);
}
