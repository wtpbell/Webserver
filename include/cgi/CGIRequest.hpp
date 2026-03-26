/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGIRequest.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/20 15:28:34 by jboon         #+#    #+#                 */
/*   Updated: 2026/02/15 21:22:13 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIREQUEST_H_
#define CGIREQUEST_H_

#include <array>
#include <filesystem>
#include <vector>

#include "http/HTTPRequest.hpp"

#define CGI_SERVER_SOFTWARE "webserv/0.1"

namespace cgi
{
  class CGIRequest
  {
    public:
      using Path = std::filesystem::path;

      CGIRequest(void) = default;
      CGIRequest(const HTTPRequest& http_request, const struct CGIRoute& route, std::string_view server_info,
                 std::string_view client_info);
      CGIRequest(const CGIRequest& other) = default;
      CGIRequest(CGIRequest&& other) noexcept = default;
      ~CGIRequest(void) = default;

      CGIRequest& operator=(const CGIRequest& rhs) = default;
      CGIRequest& operator=(CGIRequest&& rhs) noexcept = default;

      const std::string& GetBody(void) const noexcept;
      const std::vector<std::string>& GetArgv(void) const noexcept;
      const std::vector<std::string>& GetEnvp(void) const noexcept;
      std::size_t& GetLeftover(void) noexcept;

    private:
      static std::array<std::string_view, 7> filter_;

      std::vector<std::string> argv_;
      std::vector<std::string> envp_;
      std::string body_;
      std::size_t leftover_{0};
  };
}  // namespace cgi

#endif  // CGIREQUEST_H_
