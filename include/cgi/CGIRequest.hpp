/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGIRequest.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/20 15:28:34 by jboon         #+#    #+#                 */
/*   Updated: 2026/04/25 23:26:39 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIREQUEST_H_
#define CGIREQUEST_H_

#include <array>
#include <filesystem>
#include <vector>

#include "config/RouteView.hpp"
#include "config/ServerView.hpp"
#include "http/HTTPRequest.hpp"

#define CGI_SERVER_SOFTWARE "webserv/0.1"

namespace cgi
{
  class CGIRequest
  {
    public:
      using Path = std::filesystem::path;
      using IpPort = ServerView::IpPort;

      CGIRequest(void) = delete;
      CGIRequest(const HTTPRequest& httpRequest, const struct CGIRoute& cgiRoute, const IpPort& ipPortServer,
                 std::string_view clientInfo, const RouteView& route);
      CGIRequest(const CGIRequest& other) = default;
      CGIRequest(CGIRequest&& other) noexcept = default;
      ~CGIRequest(void) = default;

      CGIRequest& operator=(const CGIRequest& rhs) = delete;
      CGIRequest& operator=(CGIRequest&& rhs) noexcept = default;

      const std::string& GetBody(void) const noexcept;
      const std::string& GetExecutable(void) const noexcept;
      const std::vector<std::string>& GetArgv(void) const noexcept;
      const std::vector<std::string>& GetEnvp(void) const noexcept;
      std::size_t& GetLeftover(void) noexcept;
      bool IsClosedConnection(void) const noexcept;
      const RouteView& GetRouteView(void) const noexcept;
      const IpPort& GetIpPortServer(void) const noexcept;
      const std::string& GetHostname(void) const noexcept;

    private:
      static std::array<std::string_view, 7> filter_;

      std::string executable_;
      std::vector<std::string> argv_;
      std::vector<std::string> envp_;
      std::string body_;
      std::size_t leftover_{0};
      bool isClosedConnection_{false};
      const RouteView* routeView_;
      IpPort ipPortServer_;
      std::string hostname_;
  };
}  // namespace cgi

#endif  // CGIREQUEST_H_
