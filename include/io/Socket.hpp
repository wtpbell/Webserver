/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Socket.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/11 14:37:22 by jboon         #+#    #+#                 */
/*   Updated: 2026/04/01 20:55:48 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_H_
#define SOCKET_H_

#include <sys/socket.h>
#include <sys/types.h>

#include <ostream>
#include <string>
#include <string_view>

#include "SharedFD.hpp"

class Socket : public SharedFD
{
  public:
    using addrinfo_t = struct addrinfo;
    using sockaddr_storage_t = struct sockaddr_storage;

    enum class ShutdownState
    {
      kShutRd = SHUT_RD,
      kShutWr = SHUT_WR,
      kShutRdWr = SHUT_RDWR
    };

    enum class Type
    {
      kNone = AF_UNSPEC,
      kIPv4 = AF_INET,
      kIPv6 = AF_INET6
    };

    Socket(void) = default;
    Socket(const Socket& other) = default;
    Socket(Socket&& other) noexcept = default;
    Socket& operator=(const Socket& rhs) = default;
    Socket& operator=(Socket&& rhs) noexcept = default;
    ~Socket(void) = default;

    static Socket CreateSocket(int domain, int type, int protocol);
    static Socket CreateSocket(const char* host, const char* service, int flags = 0, Type type = Type::kIPv6);
    static std::pair<Socket, Socket> SocketPair(int domain, int type, int protocol);
    static addrinfo_t* GetAddrInfo(addrinfo_t& hints, const char* host, const char* service);

    void Bind(void);

    void Listen(int backlog = SOMAXCONN);
    void Connect(void);
    Socket Accept4(int flags = 0);
    void SetSockOpt(int level, int optname, const void* optval, socklen_t optlen);
    void SetNonBlocking(bool is_non_blocking);
    ssize_t Recv(std::string& message, int flags = 0, std::size_t max_chunk_size = 1024);
    ssize_t Send(std::string_view message, std::size_t& leftover, int flags = 0, std::size_t max_chunk_size = 4096);
    bool Shutdown(ShutdownState how);
    std::string_view GetSocketInfo(void) const;
    friend std::ostream& operator<<(std::ostream& out, const Socket& socket);

  private:
    struct SocketAddress
    {
        struct sockaddr_storage addr_storage;
        socklen_t addrlen = sizeof(addr_storage);

        struct sockaddr* GetAddrPointer(void)
        {
          return reinterpret_cast<struct sockaddr*>(&addr_storage);
        }

        const sockaddr* GetAddrPointer(void) const
        {
          return reinterpret_cast<const struct sockaddr*>(&addr_storage);
        }

        socklen_t* GetAddrlenPointer(void)
        {
          return (&addrlen);
        }
    };

    Socket(int socket_fd, bool is_non_blocking = false);
    Socket(int socket_fd, const SocketAddress& address, bool is_non_blocking = false);
    Socket(int socket_fd, struct sockaddr* addr, socklen_t socklen);

    std::string UpdateNameInfo(void) const;
    void GetSockName(SocketAddress& address) const;

    std::string info_;
    SocketAddress address_{};
    bool is_non_blocking_{false};
};

#endif  // SOCKET_H_
