/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/18 12:41:29 by jboon         #+#    #+#                 */
/*   Updated: 2025/12/14 11:28:24 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_H_
#define SERVER_H_

#include <netdb.h>

#include <memory>
#include <string>
#include <unordered_map>

#include "EpollManager.hpp"

struct Connection;
struct HTTPMessage;

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

    void ListenAndRegister(EpollManager& manager, int max_connections = SOMAXCONN);
    bool Recv(int fd, std::string& msg, std::size_t max_chunk_size = 1024);
    bool Send(int fd, const std::string& msg, std::size_t& leftover, std::size_t max_chunk_size = 4096);

  private:
    struct Connection
    {
        // TODO: Remove pointer (incomplete type)
        std::unique_ptr<HTTPMessage*> request;
        std::unique_ptr<HTTPMessage*> response;
        std::string client_info_;
        std::size_t bytes_remaining;  // Without this member the Connection size is equal to 64 bytes

        void Clear(void);
    };  // Current size 72 bytes

    using addrinfo_t = struct addrinfo;
    using sockaddr_storage_t = struct sockaddr_storage;
    using ConnectionIterator = std::unordered_map<int, Connection>::iterator;

    addrinfo_t* GetAddrinfo(std::string_view service);
    int SocketAndBind(const addrinfo_t* result);

    void Accept(EpollManager& manager, const struct epoll_event&);
    void HandleRequest(EpollManager& manager, const struct epoll_event&);
    void HandleResponse(EpollManager& manager, const struct epoll_event&);
    void CloseConnection(EpollManager& manager, ConnectionIterator it);

    int socket_fd_ = -1;
    std::string socket_info_;
    std::unordered_map<int, Connection> connections_;
};

#endif  // SERVER_H_
