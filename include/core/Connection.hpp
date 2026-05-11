/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Connection.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/03/17 11:33:36 by jboon         #+#    #+#                 */
/*   Updated: 2026/05/03 21:07:28 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <deque>
#include <string>
#include <string_view>

#include "cgi/CGIRequest.hpp"
#include "cgi/CGIResponse.hpp"
#include "config/RouteView.hpp"
#include "http/HTTPParser.hpp"
#include "http/HTTPRequest.hpp"
#include "http/HTTPResponse.hpp"
#include "http/HTTPValidator.hpp"
#include "io/Socket.hpp"

class RequestContext;

class Connection
{
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

    struct PendingResponse
    {
        enum class State
        {
          kWaiting,
          kReady
        };

        PendingResponse(State state, int cgiFd, std::string wire, bool closeAfter)
            : state_(state), cgiFd_(cgiFd), wire_(wire), closeAfter_(closeAfter), remaining_(wire.length())
        {
        }

        State state_;
        int cgiFd_;         // valid only for CGI-backed items
        std::string wire_;  // valid when ready
        bool closeAfter_;
        std::size_t remaining_;
    };

    const Socket& GetSocket(void) const;
    void SetPeerClosed(bool close);
    bool HasPendingOutput(void) const;
    bool HasPauseReading(void) const;
    void Clear(void);
    State HandleRequest(RequestContext& requestContext);
    State HandleResponse(void);
    void UpdateCgiResponse(const int cgiFd, const cgi::CGIResponse& cgiResponse);
    void UpdateCgiErrorResponse(const int cgiFd, HTTP::Status errorStatus, const cgi::CGIRequest& cgiRequest,
                                RequestContext& requestContext);
    int RedirectCgiResponse(const int cgiFd, const std::string& localTarget, const std::string& hostname,
                            RequestContext& requestContext);

  private:
    enum class ReadResult
    {
      kOk,
      kClosed,
      kFatal
    };

    ReadResult ReadOnce(std::string& out);
    void InitBodyParser(HTTPParser& parser, const HTTPRequest& request);
    std::optional<ValidationResult> HandleHeadersDone(RequestContext& requestContext);
    bool CheckConnectionClose(const HTTPRequest& request, const HTTPResponse& response) const;
    void QueueResponse(const HTTPResponse& response, bool closeAfter);
    void QueueError(ValidationResult result);
    void QueueCgiResponse(const int cgiFd, bool closeAfter);
    State ProcessInput(std::string_view in, RequestContext& requestContext);
    Connection::State FailRequest(ValidationResult error);
    std::optional<ValidationResult> ValidatePartialRequest(HTTPRequest& request);
    void MatchRouteAndApplyLimits(RequestContext& requestContext, const HTTPRequest& request);
    std::optional<ValidationResult> ConfigureBodyStorage(HTTPRequest& request);
    std::optional<ValidationResult> AttachRequestCookies(HTTPRequest& request);

    Socket socket_;
    HTTPParser parser_;
    const RouteView* matchedRoute_;

    std::deque<PendingResponse> outputQueue_;  // queued responses (pipeline)
    bool closeAfterSend_ = false;
    bool peerClosed_ = false;
    bool isFirstResponse_ = true;
};

#endif  // CONNECTION_H_
