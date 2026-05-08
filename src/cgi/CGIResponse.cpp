/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGIResponse.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/20 15:28:16 by jboon         #+#    #+#                 */
/*   Updated: 2026/04/26 21:20:01 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "cgi/CGIResponse.hpp"

#include <optional>
#include <ostream>
#include <string>
#include <string_view>

#include "http/HTTPUtils.hpp"
#include "string.hpp"

namespace cgi
{
  /* ============================ PUBLIC  ===================================== */

  CGIResponse::CGIResponse(const Status& status, const std::string& body) : status_(status), body_(body) {}

  const std::optional<std::string> CGIResponse::LocalRedirectTarget(void) const noexcept
  {
    auto it = headers_.find("Location");
    if (it == headers_.end() || it->second.front() != '/')
    {
      return std::nullopt;
    }

    std::string localTarget;
    if (HTTP::wire::NormalizePath(HTTP::wire::URLDecode(it->second), localTarget))
    {
      return localTarget;
    }
    return std::nullopt;
  }

  std::string CGIResponse::SerializeAsHttp(void) const
  {
    std::string message;
    message.reserve(1024 + body_.size());

    message.append("HTTP/1.1 ");
    message.append(std::to_string(status_.status_code)).append(" ").append(status_.reason);
    message.append("\r\n");

    if (headers_.count("Server") == 0)
    {
      message.append("Server: ").append("webserv/0.1").append("\r\n");
    }
    if (headers_.count("Date") == 0)
    {
      char buffer[256];
      message.append("Date: ").append(String::GMTCstring(buffer, sizeof(buffer))).append("\r\n");
    }

    for (const auto& [key, value] : headers_)
    {
      message.append(key).append(": ").append(value).append("\r\n");
    }

    for (const auto& cookie : cookies_)
      message.append("Set-Cookie: ").append(cookie).append("\r\n");

    if (!body_.empty())
    {
      if (headers_.count("Content-Type") == 0)
      {
        message.append("Content-Type: application/octet-stream\r\n");
      }
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
    if (body_.empty())
    {
      body_.append(status.reason).append("\n");
    }
  }

  std::ostream& operator<<(std::ostream& out, const CGIResponse& response)
  {
    return out << response.SerializeAsHttp();
  }

}  // namespace cgi
