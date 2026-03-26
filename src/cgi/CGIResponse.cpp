/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGIResponse.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/20 15:28:16 by jboon         #+#    #+#                 */
/*   Updated: 2026/03/17 00:24:35 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "cgi/CGIResponse.hpp"

#include <ostream>
#include <string>
#include <string_view>

#include "string.hpp"

namespace cgi
{
  /* ============================ PUBLIC  ===================================== */

  /* A Local redirect must be handled by the server as it was a regular HTTP request from the client */
  bool CGIResponse::IsLocalRedirect(void) const noexcept
  {
    auto it = headers_.find("Location");
    if (it == headers_.end())
    {
      return false;
    }
    return it->second.front() == '/';
  }

  const std::string& CGIResponse::LocalTarget(void) const noexcept
  {
    auto it = headers_.find("Location");
    if (it == headers_.end())
    {
      return kEmpty_;
    }
    return it->second;
  }

  std::string CGIResponse::SerializeAsHttp(void) const
  {
    std::string message;
    message.reserve(1024 + body_.size());

    message.append("HTTP/1.1 ");
    message.append(std::to_string(status_.status_code)).append(" ").append(status_.reason);
    message.append("\r\n");

    if (headers_.count("Server") == 0)
      message.append("Server: ").append("webserve/0.1").append("\r\n");
    if (headers_.count("Date") == 0)
    {
      char buffer[256];
      message.append("Date: ").append(String::GMTCstring(buffer, sizeof(buffer))).append("\r\n");
    }

    for (const auto& [key, value] : headers_)
      message.append(key).append(": ").append(value).append("\r\n");

    for (const auto& cookie : cookies_)
      message.append("Set-Cookie: ").append(cookie).append("\r\n");

    if (!body_.empty())
    {
      message.append("Content-Length: ").append(std::to_string(body_.length())).append("\r\n");
      message.append("\r\n");
      message.append(body_);
    }
    else
    {
      message.append("\r\n");
    }

    return message;
  }

  void CGIResponse::EmplaceHeader(std::string&& key, std::string&& value) noexcept
  {
    auto it = headers_.find(key);
    if (it == headers_.end())
    {
      headers_.emplace(std::move(key), std::move(value));
    }
    else
    {
      it->second.append(", ").append(value);
    }
  }

  void CGIResponse::AddCookie(std::string&& cookie)
  {
    cookies_.emplace_back(std::move(cookie));
  }

  void CGIResponse::SetBody(std::string_view body)
  {
    body_ = body;
  }

  void CGIResponse::SetStatus(const Status& status)
  {
    status_ = status;
  }

  std::ostream& operator<<(std::ostream& out, const CGIResponse& response)
  {
    return out << response.SerializeAsHttp();
  }

  /* ============================ PRIVATE ===================================== */

  const std::string CGIResponse::kEmpty_{};
}  // namespace cgi
