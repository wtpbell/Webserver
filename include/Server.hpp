/* *4************************************************************************* */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/06 14:04:06 by bewong        #+#    #+#                 */
/*   Updated: 2026/03/31 10:30:14 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_H_
#define SERVER_H_

#include <netdb.h>

#include <string>
#include <unordered_map>

#include "Connection.hpp"
#include "EpollManager.hpp"
#include "config/ServerRegistry.hpp"
#include "config/ServerView.hpp"
#include "http/SessionManager.hpp"
#include "io/Socket.hpp"
#include "router/Router.hpp"

class Server
{
    using IpPort = ServerView::IpPort;

  public:
    Server(void) = delete;
    Server(const IpPort& ipport, Socket::Type type, const ServerRegistry& serverRegistry);
    ~Server(void);
    Server(const Server& other) = delete;
    Server(Server&& other) noexcept = default;
    Server& operator=(const Server& rhs) = delete;
    Server& operator=(Server&& rhs) noexcept = delete;

    void RegisterFD(EpollManager& manager);

  private:
    using addrinfo_t = struct addrinfo;
    using sockaddr_storage_t = struct sockaddr_storage;
    using ConnectionIterator = std::unordered_map<int, Connection>::iterator;

    std::string ExtractHostName(const HTTPRequest& request) const;
    void Accept(EpollManager& manager, const struct epoll_event& event);
    void HandleConnection(EpollManager& mananger, const struct epoll_event& event);
    void CloseConnection(EpollManager& manager, ConnectionIterator connection);

    IpPort ipPort_;
    Socket socket_;
    const ServerRegistry& serverRegistry_;
    Router router_;
    SessionManager sessionManager_;
    std::unordered_map<int, Connection> connections_;
};
#endif  // SERVER_H_
