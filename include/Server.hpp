/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/18 12:41:29 by jboon         #+#    #+#                 */
/*   Updated: 2025/12/26 15:30:35 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_H_
#define SERVER_H_

#include <netdb.h>

#include <memory>
#include <unordered_map>

#include "EpollManager.hpp"
#include "io/Socket.hpp"

struct Connection;
struct HTTPMessage;

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
        // TODO: Remove pointer (incomplete type)
        std::unique_ptr<HTTPMessage*> request;
        std::unique_ptr<HTTPMessage*> response;
        std::size_t bytes_remaining;  // Without this member the Connection size is equal to 64 bytes

    };  // Current size 72 bytes

    using addrinfo_t = struct addrinfo;
    using sockaddr_storage_t = struct sockaddr_storage;
    using ConnectionIterator = std::unordered_map<int, Connection>::iterator;

    void Accept(EpollManager& manager, const struct epoll_event&);
    void HandleRequest(EpollManager& manager, const struct epoll_event&);
    void HandleResponse(EpollManager& manager, const struct epoll_event&);
    void CloseConnection(EpollManager& manager, ConnectionIterator it);

    Socket socket_;
    std::unordered_map<int, Connection> connections_;
};

#endif  // SERVER_H_
