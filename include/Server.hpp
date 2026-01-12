/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/06 14:04:06 by bewong        #+#    #+#                 */
/*   Updated: 2026/01/08 17:03:20 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_H_
#define SERVER_H_

#include <netdb.h>

#include <memory>
#include <optional>
#include <unordered_map>

#include "EpollManager.hpp"
#include "http/HTTPParser.hpp"
#include "http/HTTPRequest.hpp"
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
    struct Connection
    {
        Connection(const Socket& s) : socket(s) {}

        void Clear(void);

        Socket socket;
        HTTPParser parser;
        std::optional<HTTPRequest> request;
    };  // Current size 72 bytes

    using addrinfo_t = struct addrinfo;
    using sockaddr_storage_t = struct sockaddr_storage;
    using ConnectionIterator = std::unordered_map<int, Connection>::iterator;

    void Accept(EpollManager& manager, const struct epoll_event&);
    void HandleRequest(EpollManager& manager, const struct epoll_event&);
    void HandleResponse(EpollManager& manager, const struct epoll_event&);
    void CloseConnection(EpollManager& manager, ConnectionIterator it);
    void ParseRequest(std::string_view data);

    Socket socket_;
    std::unordered_map<int, Connection> connections_;
};

#endif  // SERVER_H_
