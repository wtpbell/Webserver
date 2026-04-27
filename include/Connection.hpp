/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Connection.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/03/17 11:33:36 by jboon         #+#    #+#                 */
/*   Updated: 2026/04/24 15:51:28 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <deque>
#include <string>
#include <string_view>

#include "config/RouteView.hpp"
#include "config/ServerRegistry.hpp"
#include "http/HTTPParser.hpp"
#include "http/HTTPRequest.hpp"
#include "http/HTTPResponse.hpp"
#include "http/HTTPValidator.hpp"
#include "http/SessionManager.hpp"
#include "io/Socket.hpp"
#include "router/Router.hpp"

class Connection
{
    using IpPort = ServerView::IpPort;

  public:
    Connection(Socket socket);
    Connection() = delete;
    Connection(const Connection& other) = delete;
    Connection(Connection&& other) noexcept = default;
    Connection& operator=(const Connection& rhs) = delete;
    Connection& operator=(Connection&& rhs) noexcept = default;
    ~Connection() = default;

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
    void InitBodyParser(HTTPParser& parser, const HTTPRequest& request);
    std::optional<ValidationResult> HandleHeadersDone(const ServerRegistry& serverRegistry);
    bool CheckConnectionClose(const HTTPRequest& request, const HTTPResponse& response) const;
    void QueueResponse(const HTTPResponse& response, bool closeAfter);
    void QueueError(ValidationResult result);
    State ProcessInput(std::string_view in, const Router& router, const ServerRegistry& serverRegistry,
                       SessionManager& sessionManager);
    Connection::State FailRequest(ValidationResult error);
    std::optional<ValidationResult> ValidatePartialRequest(HTTPRequest& request);
    void MatchRouteAndApplyLimits(const ServerRegistry& serverRegistry, const HTTPRequest& request);
    std::optional<ValidationResult> ConfigureBodyStorage(HTTPRequest& request);
    std::optional<ValidationResult> AttachRequestCookies(HTTPRequest& request);

    Socket socket_;
    HTTPParser parser_;
    IpPort ipport_;
    const RouteView* matchedRoute_;
    std::string matchedHost_;

    std::deque<std::string> outputQueue_;  // queued responses (pipeline)
    std::size_t remaining_ = 0;            // number of bytes already sent
    bool closeAfterSend_ = false;
    bool peerClosed_ = false;

#ifdef UNIT_TEST
  public:
    using TestIpPort = ServerView::IpPort;

    State TestProcessInput(std::string_view in, const Router& router, const ServerRegistry& serverRegistry,
                           SessionManager& sessionManager)
    {
      return ProcessInput(in, router, serverRegistry, sessionManager);
    }

    void TestSetIpPort(const ServerView::IpPort& ipport)
    {
      ipport_ = ipport;
    }

    const std::deque<std::string>& TestGetOutputQueue() const
    {
      return outputQueue_;
    }

    bool TestGetCloseAfterSend() const
    {
      return closeAfterSend_;
    }
#endif
};

#endif  // CONNECTION_H_
