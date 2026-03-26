/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGIResponse.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/20 15:28:24 by jboon         #+#    #+#                 */
/*   Updated: 2026/03/17 00:04:04 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIRESPONSE_H_
#define CGIRESPONSE_H_

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
      CGIResponse(const CGIResponse&) = default;
      CGIResponse(CGIResponse&&) = default;
      ~CGIResponse(void) = default;

      CGIResponse& operator=(const CGIResponse&) = default;
      CGIResponse& operator=(CGIResponse&&) = default;

      bool IsLocalRedirect(void) const noexcept;
      const std::string& LocalTarget(void) const noexcept;
      std::string SerializeAsHttp(void) const;
      void EmplaceHeader(std::string&& key, std::string&& value) noexcept;
      void AddCookie(std::string&& cookie);
      void SetBody(std::string_view body);
      void SetStatus(const Status& status);

      friend std::ostream& operator<<(std::ostream& out, const CGIResponse& response);

    private:
      using CGIHeaders = std::unordered_map<std::string, std::string>;

      static const std::string kEmpty_;

      CGIHeaders headers_;
      std::vector<std::string> cookies_;
      std::string body_;
      Status status_{200, "OK"};

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
