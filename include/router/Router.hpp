/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Router.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/29 13:10:28 by bewong        #+#    #+#                 */
/*   Updated: 2026/01/29 13:10:28 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTER_HPP
#define ROUTER_HPP

#ifdef UNIT_TEST
#include <functional>
#endif

#include <string_view>

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

    HTTPResponse Dispatch(const HTTPRequest& request, const RouteView& route, const ServerRegistry& serverRegistry,
                          const ServerView::IpPort& ipPort, std::string_view hostName) const;

  private:
    HTTPResponse DispatchHandler(const HTTPRequest& request, const RouteView& route) const;
    HTTPResponse ApplyErrorPage(const HTTPRequest& request, const RouteView& route, HTTPResponse res,
                                const ServerRegistry& serverRegistry, const ServerView::IpPort& ipPort,
                                std::string_view hostName) const;
    HTTPResponse MakeReturnResponse(const RouteView::ReturnRule& rule) const;
    bool ShouldUseCgi(const HTTPRequest& request, const RouteView& route) const;

#ifdef UNIT_TEST
  public:
    using DispatchHook = std::function<HTTPResponse(const HTTPRequest&, const RouteView&)>;

    void SetDispatchHookForTest(DispatchHook hook)
    {
      dispatchHook_ = std::move(hook);
    }

  private:
    DispatchHook dispatchHook_;
#endif
};

#endif
