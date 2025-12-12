/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/18 12:41:29 by jboon         #+#    #+#                 */
/*   Updated: 2025/12/01 17:09:10 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_H_
#define SERVER_H_

#include <netdb.h>

#include <string>
#include <unordered_map>

#include "Connection.hpp"
#include "EpollManager.hpp"

class Server
{
  public:
    Server(std::string_view service);
    ~Server(void);
    Server(void) = delete;
    Server(const Server& other) = delete;
    Server(Server&& other) noexcept = delete;
    Server& operator=(const Server& rhs) = delete;
    Server& operator=(Server&& rhs) noexcept = delete;

    void ListenAndRegister(EpollManager& manager, int max_connections);

  private:
    using addrinfo_t = struct addrinfo;
    using sockaddr_storage_t = struct sockaddr_storage;

    addrinfo_t* GetAddrinfo(std::string_view service);
    int SocketAndBind(const addrinfo_t* result);

    void Accept(EpollManager& manager, const struct epoll_event&);
    void Recv(EpollManager& manager, const struct epoll_event&);
    void Send(EpollManager& manager, const struct epoll_event&);

    int socket_fd_ = -1;
    std::string socket_info_;
    std::unordered_map<int, Connection> connections_;
};

#endif  // SERVER_H_
