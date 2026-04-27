/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Router.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/02/13 10:04:39 by bewong        #+#    #+#                 */
/*   Updated: 2026/02/13 10:04:39 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "router/Router.hpp"

#include <optional>
#include <string_view>

#include "config/RouteView.hpp"
#include "config/ServerRegistry.hpp"
#include "config/ServerView.hpp"
#include "http/ResponseFactory.hpp"
#include "router/RequestHandler.hpp"

// server
// {
//   root / var / www;

//   location / images
//   {
//     root / data / images;
//   }
// }

// 1. request : / images / logo.png
// -> the request starts with /images match with /images
// -> root = /data/images
// -> final path: /data/images/logo.png

// 2. request : /index.html
// -> .index.html doesnt start with /images
// -> no location match -> router falls back to root
// -> final path: /var/www/index.html

// 3. request : /foo/bar.txt
// -> no location match
// -> use server root
// -> final path: /var/www/foo/bar.txt

namespace
{
  namespace Response = HTTP::response;
  using Status = HTTP::Status;

  bool MatchLocationPrefix(std::string_view path, std::string_view prefix)
  {
    if (prefix == "/")
      return true;
    if (path.size() < prefix.size() || path.compare(0, prefix.size(), prefix) != 0)
      return false;
    return (path.size() == prefix.size() || path[prefix.size()] == '/');
  }

  bool IsExternalURL(const std::string& s)
  {
    return s.rfind("http://", 0) == 0 || s.rfind("https://", 0) == 0;
  }

  std::optional<HTTPRequest> MakeInternalGet(std::string_view uri)
  {
    HTTPRequest request;

    request.SetMethod(HTTP::Method::kGet);
    if (!request.SetTarget(uri))
      return std::nullopt;
    request.SetComplete(true);
    return request;
  }
}  // namespace

HTTPResponse Router::DispatchHandler(const HTTPRequest& request, const RouteView& route) const
{
#ifdef UNIT_TEST
  if (dispatchHook_)
  {
    const std::string_view remainder = request_handler::ComputeRouteTail(request.GetPath(), route.locationPrefix);
    return dispatchHook_(request, route, remainder);
  }
#endif

  const std::string_view remainder = request_handler::ComputeRouteTail(request.GetPath(), route.locationPrefix);
  if (remainder.empty())
    return Response::MakeError(Status::kInternalServerError);

  if (ShouldUseCgi(request, route))
    return request_handler::HandleCgi(request, route, remainder);

  return request_handler::HandleMethods(request, route, remainder);
}

HTTPResponse Router::Dispatch(const HTTPRequest& request, const RouteView& route, const ServerRegistry& serverRegistry,
                              const ServerView::IpPort& ipPort, std::string_view hostName) const
{
  HTTPResponse res;

  if (route.returnRule.has_value())
    res = MakeReturnResponse(*route.returnRule);
  else
    res = DispatchHandler(request, route);

  if (res.GetStatusCode() >= 400 && route.errorPages.find(res.GetStatusCode()) != route.errorPages.end())
    return ApplyErrorPage(request, route, std::move(res), serverRegistry, ipPort, hostName);

  return res;
}

bool Router::ShouldUseCgi(const HTTPRequest& request, const RouteView& /*route*/) const
{
  return MatchLocationPrefix(request.GetPath(), "/cgi-bin");
}

// If there's no matching error_page mapping for this status code, return original response.
HTTPResponse Router::ApplyErrorPage(const HTTPRequest& request, const RouteView& route, HTTPResponse res,
                                    const ServerRegistry& serverRegistry, const ServerView::IpPort& ipPort,
                                    std::string_view hostName) const
{
  const std::uint16_t origCode = res.GetStatusCode();
  const std::string& target = route.errorPages.find(origCode)->second;

  if (IsExternalURL(target))
    return Response::MakeRedirect(Status::kFound, target);

  std::optional<HTTPRequest> internalOpt = MakeInternalGet(target);
  if (!internalOpt)
    return Response::MakeError(Status::kInternalServerError);

  HTTPRequest internal = std::move(*internalOpt);
  if (internal.GetPath() == request.GetPath())
    return res;

  const RouteView* internalRoute =
      serverRegistry.GetRouteView(ipPort.ip, ipPort.port, std::string(hostName), std::string(internal.GetPath()));

  if (internalRoute == nullptr)
    return res;

  HTTPResponse page = DispatchHandler(internal, *internalRoute);
  if (page.GetStatusCode() >= 400)
    return res;

  page.SetStatus(static_cast<Status>(origCode));
  return page;
}

HTTPResponse Router::MakeReturnResponse(const RouteView::ReturnRule& rule) const
{
  const auto st = static_cast<Status>(rule.code);

  if (rule.code >= 300 && rule.code < 400)
  {
    if (!rule.target.empty())
      return Response::MakeRedirect(st, rule.target);
    return Response::MakeEmpty(st);
  }

  if (rule.code >= 400)
    return Response::MakeError(st);

  return Response::MakeEmpty(st);
}
