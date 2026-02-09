/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/06 14:04:06 by bewong        #+#    #+#                 */
/*   Updated: 2026/01/28 09:55:33 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_H_
#define SERVER_H_

#include <netdb.h>

#include <deque>
#include <unordered_map>

#include "EpollManager.hpp"
#include "http/HTTPParser.hpp"
#include "http/HTTPRequest.hpp"
#include "http/HTTPResponse.hpp"
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
        std::deque<std::string> outputQueue;  // queued responses (pipeline)
        std::size_t remaining = 0;            // number of bytes already sent
        bool closeAfterSend = false;
        bool peerClosed = false;
    };  // Current size 72 bytes

    enum class ReadResult
    {
      Ok,
      Empty,
      Closed,
      Fatal
    };

    using addrinfo_t = struct addrinfo;
    using sockaddr_storage_t = struct sockaddr_storage;
    using ConnectionIterator = std::unordered_map<int, Connection>::iterator;

    ReadResult ReadOnce(Connection& c, std::string& out);
    void InitBodyParser(HTTPParser& parser, const HTTPRequest& req);
    void HandleHeadersDone(EpollManager& manager, ConnectionIterator it, Connection& connection, int client_fd);
    void EnableReadWrite(EpollManager& manager, int fd);
    void EnableReadOnly(EpollManager& manager, int fd);

    void Accept(EpollManager& manager, const struct epoll_event&);
    void HandleRequest(EpollManager& manager, const struct epoll_event&);
    void HandleResponse(EpollManager& manager, const struct epoll_event&);
    void CloseConnection(EpollManager& manager, ConnectionIterator it);
    void ParseRequest(std::string_view data);
    void FinishResponse(EpollManager& manager, ConnectionIterator it);
    bool CheckConnectionClose(const HTTPRequest* request, const HTTPResponse& response) const;
    HTTPResponse DispatchRequest(const HTTPRequest& req);
    void QueueResponse(EpollManager& manager, ConnectionIterator it, HTTPResponse resp, bool closeAfter);
    void QueueError(EpollManager& manager, ConnectionIterator it, ValidationResult result);
    Socket socket_;
    std::unordered_map<int, Connection> connections_;
};

#endif  // SERVER_H_
