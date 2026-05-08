/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Router.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/29 13:10:28 by bewong        #+#    #+#                 */
/*   Updated: 2026/05/03 22:41:00 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <string_view>
#include <variant>

#include "cgi/CGIProcess.hpp"
#include "config/RouteView.hpp"
#include "config/ServerRegistry.hpp"
#include "http/HTTPRequest.hpp"
#include "http/HTTPResponse.hpp"

class Router
{
  public:
    Router() = default;
    Router(const Router& other) = delete;
    Router(Router&& other) noexcept = default;
    Router& operator=(const Router& other) = delete;
    Router& operator=(Router&& other) noexcept = default;
    ~Router() = default;

    std::variant<std::monostate, HTTPResponse, cgi::CGIProcess> Dispatch(const HTTPRequest& request,
                                                                         const RouteView& route,
                                                                         const ServerRegistry& serverRegistry,
                                                                         const ServerView::IpPort& ipPort,
                                                                         std::string_view clientInfo) const;
    HTTPResponse ApplyErrorPage(HTTP::Status statusCode, const RouteView& route, const ServerRegistry& serverRegistry,
                                const ServerView::IpPort& ipPort, std::string_view hostName) const;

  private:
    HTTPResponse ApplyErrorPage(const HTTPRequest& request, const RouteView& route, HTTPResponse res,
                                const ServerRegistry& serverRegistry, const ServerView::IpPort& ipPort,
                                std::string_view hostName) const;
    HTTPResponse MakeReturnResponse(const RouteView::ReturnRule& rule) const;
};

#endif
