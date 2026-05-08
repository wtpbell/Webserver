/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGIResponse.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/20 15:28:24 by jboon         #+#    #+#                 */
/*   Updated: 2026/04/27 10:29:53 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIRESPONSE_H_
#define CGIRESPONSE_H_

#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace cgi
{
  struct Status
  {
      int status_code;
      std::string reason;
  };

  class CGIResponse
  {
    public:
      CGIResponse(void) = default;
      CGIResponse(const Status& status, const std::string& body);
      CGIResponse(const CGIResponse&) = default;
      CGIResponse(CGIResponse&&) noexcept = default;
      ~CGIResponse(void) = default;

      CGIResponse& operator=(const CGIResponse&) = default;
      CGIResponse& operator=(CGIResponse&&) noexcept = default;

      const std::optional<std::string> LocalRedirectTarget(void) const noexcept;
      std::string SerializeAsHttp(void) const;
      void EmplaceHeader(std::string&& key, std::string&& value) noexcept;
      void AddCookie(std::string&& cookie);
      void SetBody(std::string_view body);
      void SetStatus(const Status& status);

      friend std::ostream& operator<<(std::ostream& out, const CGIResponse& response);

    private:
      using CGIHeaders = std::unordered_map<std::string, std::string>;

      CGIHeaders headers_;
      std::vector<std::string> cookies_;
      Status status_{200, "OK"};
      std::string body_;

#ifdef UNIT_TEST
    public:
      const Status& GetStatus(void) const noexcept
      {
        return status_;
      }

      const std::string& GetBody(void) const noexcept
      {
        return body_;
      }

      const CGIHeaders& GetHeaders(void) const noexcept
      {
        return headers_;
      }

      const std::vector<std::string>& GetCookies(void) const noexcept
      {
        return cookies_;
      }
#endif
  };
}  // namespace cgi
#endif  // CGIRESPONSE_H_
