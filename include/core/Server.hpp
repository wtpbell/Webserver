/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/06 14:04:06 by bewong        #+#    #+#                 */
/*   Updated: 2026/05/03 21:14:25 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_H_
#define SERVER_H_

#include <netdb.h>

#include "core/ConnectionRegistry.hpp"
#include "core/EpollManager.hpp"
#include "core/RequestContext.hpp"
#include "config/ServerRegistry.hpp"
#include "config/ServerView.hpp"
#include "http/SessionManager.hpp"
#include "io/Socket.hpp"
#include "router/Router.hpp"

class Server
{
    using IpPort = ServerView::IpPort;

  public:
    Server() = delete;
    Server(const IpPort& ipport, Socket::Type type, const ServerRegistry& serverRegistry, EpollManager& epollManager);
    ~Server();
    Server(const Server& other) = delete;
    Server(Server&& other) noexcept = default;
    Server& operator=(const Server& rhs) = delete;
    Server& operator=(Server&& rhs) noexcept = delete;

  private:
    using addrinfo_t = struct addrinfo;
    using sockaddr_storage_t = struct sockaddr_storage;

    void Accept(EpollManager& manager, const struct epoll_event& event);
    void HandleConnection(EpollManager& mananger, const struct epoll_event& event);
    void HandleCGI(EpollManager& manager, const struct epoll_event& event);
    void HandleTimeout(EpollManager& manager, const struct epoll_event& event);
    void ErrorCloseCgiProcess(EpollManager& manager, Connection& connection, cgi::CGIProcess& cgiProcess);
    void CloseConnection(EpollManager& manager, ConnectionRegistry::ConnectionData& connectionData);

    IpPort ipPort_;
    Socket socket_;
    const ServerRegistry& serverRegistry_;
    Router router_;
    SessionManager sessionManager_;
    ConnectionRegistry connectionRegistry_;
    RequestContext requestContext_;
};
#endif  // SERVER_H_
