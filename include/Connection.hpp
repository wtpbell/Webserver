/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Connection.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/03/17 11:33:36 by jboon         #+#    #+#                 */
/*   Updated: 2026/04/01 20:54:54 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <deque>
#include <string>

#include "config/ServerRegistry.hpp"
#include "http/HTTPParser.hpp"
#include "http/HTTPRequest.hpp"
#include "http/HTTPResponse.hpp"
#include "http/HTTPValidator.hpp"
#include "http/SessionManager.hpp"
#include "io/Socket.hpp"

// TODO: replace with actual Router
class Router
{
};

class Connection
{
    using IpPort = ServerView::IpPort;

  public:
    Connection(const Socket& s) : socket_(s) {}
    Connection(void) = delete;
    Connection(const Connection& other) = default;
    Connection(Connection&& other) noexcept = default;
    Connection& operator=(const Connection& rhs) = default;
    Connection& operator=(Connection&& rhs) noexcept = default;
    ~Connection(void) = default;

    enum class State
    {
      kKeepAlive,
      kClose,
      kError
    };

    const Socket& GetSocket(void) const;
    void SetPeerClosed(bool close);
    bool HasPendingOutput(void) const;
    void Clear(void);
    State HandleRequest(const IpPort& ipport, const Router& router, const ServerRegistry& serverRegistry,
                        SessionManager& session_manager);
    State HandleResponse(void);

  private:
    enum class ReadResult
    {
      kOk,
      kClosed,
      kFatal
    };

    ReadResult ReadOnce(std::string& out);
    void InitBodyParser(HTTPParser& parser, const HTTPRequest& req);
    void HandleHeadersDone(void);
    bool CheckConnectionClose(const HTTPRequest* request, const HTTPResponse& response) const;
    HTTPResponse DispatchRequest(const HTTPRequest& req);
    void QueueResponse(const HTTPResponse& resp, bool closeAfter);
    void QueueError(ValidationResult result);

    Socket socket_;
    HTTPParser parser_;
    std::deque<std::string> outputQueue_;  // queued responses (pipeline)
    std::size_t remaining_ = 0;            // number of bytes already sent
    bool closeAfterSend_ = false;
    bool peerClosed_ = false;
};

#endif  // CONNECTION_H_
