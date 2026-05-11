/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   RequestContext.cpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/05/03 12:48:40 by jboon         #+#    #+#                 */
/*   Updated: 2026/05/03 21:17:42 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "core/RequestContext.hpp"

#include <sys/epoll.h>

#include <string>
#include <string_view>

#include "core/ConnectionRegistry.hpp"
#include "core/EpollManager.hpp"
#include "cgi/CGIProcess.hpp"
#include "config/RouteView.hpp"
#include "config/ServerRegistry.hpp"
#include "http/HTTPRequest.hpp"
#include "http/HTTPResponse.hpp"
#include "http/ResponseFactory.hpp"
#include "http/SessionManager.hpp"
#include "router/Router.hpp"

RequestContext::RequestContext(const IpPort& ipPort, const Router& router, const ServerRegistry& serverRegistry,
                               SessionManager& sessionManager, ConnectionRegistry& connectionRegistry,
                               EpollManager& epollManager, EpollManager::EventCallback cgiCallback)
    : ipPort_(ipPort),
      router_(router),
      serverRegistry_(serverRegistry),
      sessionManager_(sessionManager),
      connectionRegistry_(connectionRegistry),
      epollManager_(epollManager),
      cgiCallback_(cgiCallback)
{
}

RequestContext::VariantResponse RequestContext::Dispatch(const HTTPRequest& request, const RouteView& route,
                                                         std::string_view clientInfo)
{
  return router_.Dispatch(request, route, serverRegistry_, ipPort_, clientInfo);
}

RequestContext::VariantResponse RequestContext::Dispatch(std::string_view target, std::string_view hostname,
                                                         std::string_view clientInfo)
{
  HTTPRequest httpRequest;
  httpRequest.SetMethod(HTTP::Method::kGet);
  if (!httpRequest.SetTarget(target))
  {
    return HTTP::response::MakeError(HTTP::Status::kBadRequest);
  }

  const RouteView* route = serverRegistry_.GetRouteView(ipPort_.ip, ipPort_.port, hostname, httpRequest.GetPath());
  if (route == nullptr)
  {
    return HTTP::response::MakeError(HTTP::Status::kNotFound);
  }
  return Dispatch(httpRequest, *route, clientInfo);
}

HTTPResponse RequestContext::DispatchError(HTTP::Status httpStatus, const RouteView& route, const IpPort& serverIpPort,
                                           std::string_view hostname)
{
  if (route.errorPages.count(static_cast<int>(httpStatus)) == 1)
  {
    return router_.ApplyErrorPage(httpStatus, route, serverRegistry_, serverIpPort, hostname);
  }
  return HTTP::response::MakeError(httpStatus);
}

SessionManager& RequestContext::GetSessionManager(void) noexcept
{
  return sessionManager_;
}

const RouteView* RequestContext::GetRouteView(std::string_view hostname, std::string_view target) const noexcept
{
  return serverRegistry_.GetRouteView(ipPort_.ip, ipPort_.port, hostname, target);
}

bool RequestContext::RegisterProcess(int connectionFd, cgi::CGIProcess&& cgiProcess)
{
  ConnectionRegistry::ConnectionData* connectionData = connectionRegistry_.FindConnection(connectionFd);
  if (connectionData == nullptr)
  {
    return false;
  }

  constexpr uint32_t events = EPOLLERR | EPOLLIN | EPOLLOUT;
  const int cgiFd = cgiProcess.GetSocket().GetFD();
  if (epollManager_.AddFd(cgiFd, events, cgiCallback_) != EpollManager::Result::kOk)
  {
    return false;
  }
  if (!connectionRegistry_.EmplaceCGIProcess(*connectionData, std::move(cgiProcess)))
  {
    epollManager_.RemoveFd(cgiFd);
    return false;
  }
  return true;
}

const std::string& RequestContext::GetIp(void) const noexcept
{
  return ipPort_.ip;
}

const std::string& RequestContext::GetPort(void) const noexcept
{
  return ipPort_.port;
}
