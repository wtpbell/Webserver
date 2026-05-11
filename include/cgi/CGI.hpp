/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGI.hpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/06 14:56:14 by jboon         #+#    #+#                 */
/*   Updated: 2026/05/03 21:52:16 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_H_
#define CGI_H_

#include <filesystem>
#include <string_view>

#include "utils/Expected.hpp"
#include "cgi/CGIError.hpp"
#include "cgi/CGIProcess.hpp"
#include "cgi/CGIRequest.hpp"
#include "config/RouteView.hpp"
#include "config/ServerRegistry.hpp"
#include "config/ServerView.hpp"
#include "http/HTTPRequest.hpp"
#include "http/HTTPStatus.hpp"

namespace cgi
{
  using IpPort = ServerView::IpPort;
  using Path = std::filesystem::path;

  struct CGIRoute
  {
      std::string executable_;
      std::string script_;
      std::string resource_;
      std::string fullResource_;
  };

  HTTP::Status CGIErrorCodeToHTTPCode(const CGIErrorCode errorCode) noexcept;
  Expected<CGIRoute, CGIErrorCode> SetupCGIRoute(std::string_view target, const RouteView& route) noexcept;
  Expected<CGIProcess, CGIErrorCode> DispatchCGIHandler(const HTTPRequest& httpRequest, const RouteView& route,
                                                        const ServerRegistry& serverRegistry, IpPort ipPort,
                                                        std::string_view clientInfo);
  Expected<CGIProcess, CGIErrorCode> ExecuteCGI(CGIRequest&& cgiRequest);
}  // namespace cgi
#endif  // CGI_H_
