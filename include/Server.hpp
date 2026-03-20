/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/06 14:04:06 by bewong        #+#    #+#                 */
/*   Updated: 2026/03/17 14:34:13 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_H_
#define SERVER_H_

#include <netdb.h>

#include "Connection.hpp"
#include "EpollManager.hpp"
#include "http/SessionManager.hpp"
#include "io/Socket.hpp"

class Server
{
  public:
    Server(const char* service);
    ~Server(void);
    Server(void) = delete;
    Server(const Server& other) = delete;
    Server(Server&& other) noexcept = delete;
    Server& operator=(const Server& rhs) = delete;
    Server& operator=(Server&& rhs) noexcept = delete;

    void ListenAndRegister(EpollManager& manager, int max_connections = SOMAXCONN);

  private:
    using addrinfo_t = struct addrinfo;
    using sockaddr_storage_t = struct sockaddr_storage;
    using ConnectionIterator = std::unordered_map<int, Connection>::iterator;

    void Accept(EpollManager& manager, const struct epoll_event& event);
    void HandleConnection(EpollManager& mananger, const struct epoll_event& event);
    void CloseConnection(EpollManager& manager, ConnectionIterator connection);

    Socket socket_;
    // TODO: routing_table
    SessionManager sessionManager_;
    std::unordered_map<int, Connection> connections_;
};

#endif  // SERVER_H_
