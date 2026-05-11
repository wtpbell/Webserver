/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Router.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/02/13 10:04:39 by bewong        #+#    #+#                 */
/*   Updated: 2026/05/03 22:41:22 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "router/Router.hpp"

#include <cstdint>
#include <optional>
#include <string_view>
#include <variant>

#include "utils/Expected.hpp"
#include "cgi/CGI.hpp"
#include "cgi/CGIProcess.hpp"
#include "config/RouteView.hpp"
#include "config/ServerRegistry.hpp"
#include "config/ServerView.hpp"
#include "http/HTTPResponse.hpp"
#include "http/HTTPStatus.hpp"
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
    return request;
  }
}  // namespace

std::variant<std::monostate, HTTPResponse, cgi::CGIProcess> Router::Dispatch(const HTTPRequest& request,
                                                                             const RouteView& route,
                                                                             const ServerRegistry& serverRegistry,
                                                                             const ServerView::IpPort& ipPort,
                                                                             std::string_view clientInfo) const
{
  HTTPResponse res;
  if (route.returnRule.has_value())
  {
    res = MakeReturnResponse(*route.returnRule);
  }
  else
  {
    Expected<cgi::CGIProcess, cgi::CGIErrorCode> cgiProcess =
        cgi::DispatchCGIHandler(request, route, serverRegistry, ipPort, clientInfo);
    if (cgiProcess.HasValue())
    {
      return cgiProcess.ExtractValue();
    }
    else if (cgiProcess.GetError() != cgi::CGIErrorCode::kNone)
    {
      HTTP::Status httpCode{cgi::CGIErrorCodeToHTTPCode(cgiProcess.GetError())};
      if (route.errorPages.count(static_cast<uint16_t>(httpCode)) == 1)
      {
        return ApplyErrorPage(cgi::CGIErrorCodeToHTTPCode(cgiProcess.GetError()), route, serverRegistry, ipPort,
                              request.GetHost());
      }
      else
      {
        return HTTP::response::MakeError(httpCode);
      }
    }

    res = request_handler::HandleMethods(request, route);
  }

  if (res.GetStatusCode() >= 400 && route.errorPages.find(res.GetStatusCode()) != route.errorPages.end())
    return ApplyErrorPage(request, route, std::move(res), serverRegistry, ipPort, request.GetHost());

  return res;
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

  HTTPResponse page = request_handler::HandleMethods(internal, *internalRoute);
  if (page.GetStatusCode() >= 400)
    return res;

  page.SetStatus(static_cast<Status>(origCode));
  return page;
}

HTTPResponse Router::ApplyErrorPage(HTTP::Status statusCode, const RouteView& route,
                                    const ServerRegistry& serverRegistry, const ServerView::IpPort& ipPort,
                                    std::string_view hostName) const
{
  const uint16_t code = static_cast<uint16_t>(statusCode);
  const std::string& target = route.errorPages.find(code)->second;

  if (IsExternalURL(target))
    return Response::MakeRedirect(Status::kFound, target);

  std::optional<HTTPRequest> internalOpt = MakeInternalGet(target);
  if (!internalOpt)
    return Response::MakeError(Status::kInternalServerError);

  HTTPRequest internal = std::move(*internalOpt);
  const RouteView* internalRoute =
      serverRegistry.GetRouteView(ipPort.ip, ipPort.port, std::string(hostName), std::string(internal.GetPath()));

  if (internalRoute == nullptr)
    return Response::MakeError(HTTP::Status::kNotFound);

  HTTPResponse page = request_handler::HandleMethods(internal, *internalRoute);
  if (page.GetStatusCode() >= 400)
    return page;
  page.SetStatus(statusCode);
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
