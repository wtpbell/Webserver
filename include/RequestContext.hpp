/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   RequestContext.hpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/05/03 12:44:48 by jboon         #+#    #+#                 */
/*   Updated: 2026/05/03 20:47:16 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTCONTEXT_H_
#define REQUESTCONTEXT_H_

#include <string>
#include <string_view>
#include <variant>

#include "ConnectionRegistry.hpp"
#include "EpollManager.hpp"
#include "cgi/CGIProcess.hpp"
#include "config/RouteView.hpp"
#include "config/ServerRegistry.hpp"
#include "http/HTTPRequest.hpp"
#include "http/HTTPResponse.hpp"
#include "http/SessionManager.hpp"
#include "router/Router.hpp"

class RequestContext
{
    using IpPort = ServerView::IpPort;
    using VariantResponse = std::variant<std::monostate, HTTPResponse, cgi::CGIProcess>;

  public:
    RequestContext(const IpPort& ipPort, const Router& router, const ServerRegistry& serverRegistry,
                   SessionManager& sessionManager, ConnectionRegistry& connectionRegistry, EpollManager& epollManager,
                   EpollManager::EventCallback cgiCallback);

    VariantResponse Dispatch(const HTTPRequest& request, const RouteView& route, std::string_view clientInfo);
    VariantResponse Dispatch(std::string_view target, std::string_view hostname, std::string_view clientInfo);
    HTTPResponse DispatchError(HTTP::Status httpStatus, const RouteView& route, const IpPort& serverIpPort,
                               std::string_view hostname);
    SessionManager& GetSessionManager(void) noexcept;
    const RouteView* GetRouteView(std::string_view hostname, std::string_view target) const noexcept;
    bool RegisterProcess(int connectionFd, cgi::CGIProcess&& cgiProcess);
    const std::string& GetIp(void) const noexcept;
    const std::string& GetPort(void) const noexcept;

  private:
    const IpPort& ipPort_;
    const Router& router_;
    const ServerRegistry& serverRegistry_;
    SessionManager& sessionManager_;
    ConnectionRegistry& connectionRegistry_;
    EpollManager& epollManager_;
    EpollManager::EventCallback cgiCallback_;
};

#endif  // REQUESTCONTEXT_H_
