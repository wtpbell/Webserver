/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGIRequest.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/20 15:28:10 by jboon         #+#    #+#                 */
/*   Updated: 2026/04/25 23:28:25 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "cgi/CGIRequest.hpp"

#include <algorithm>
#include <array>
#include <string>
#include <string_view>
#include <vector>

#include "cgi/CGI.hpp"
#include "config/RouteView.hpp"
#include "http/HTTPRequest.hpp"
#include "utils/string.hpp"

namespace cgi
{
  namespace
  {
    std::string_view GetFileName(std::string_view path)
    {
      std::size_t divider = path.rfind('/');
      if (divider == path.npos)
      {
        return path;
      }
      return path.substr(divider + 1);
    }

    std::string GetServerName(std::string_view hostname, std::string_view ip)
    {
      if (!hostname.empty())
      {
        ip = hostname;
      }
      if (ip.find(":") != ip.npos)
      {
        return std::string("[").append(ip).append("]");
      }
      return std::string(ip);
    }

    std::string GetAddressSegment(std::string_view socket_info)
    {
      std::size_t port_delimiter{socket_info.find_last_of(':')};
      if (port_delimiter == std::string_view::npos)
        return std::string(socket_info);
      std::string_view ip{socket_info.substr(0, port_delimiter)};
      return GetServerName({}, ip);
    }
  }  // namespace

  std::array<std::string_view, 7> CGIRequest::filter_ = {
      "content-length",     "content-type",        "authorization", "www-authenticate",
      "proxy-authenticate", "proxy-authorization", "host"};

  CGIRequest::CGIRequest(const HTTPRequest& httpRequest, const struct CGIRoute& cgiRoute, const IpPort& ipPortServer,
                         std::string_view clientInfo, const RouteView& route)
      : body_(httpRequest.GetBody()),  // TODO: memorysink or filesink
        leftover_(body_.length()),
        routeView_(&route),
        ipPortServer_(ipPortServer),
        hostname_(httpRequest.GetHost())
  {
    isClosedConnection_ = String::IsCloseToken(httpRequest.GetFirstHeaderValueOf("connection"));
    executable_ = cgiRoute.executable_;
    argv_.emplace_back(executable_);
    if (executable_ != cgiRoute.script_)
    {
      argv_.emplace_back(cgiRoute.script_);
    }

    envp_.emplace_back("GATEWAY_INTERFACE=CGI/1.1");
    envp_.emplace_back("SCRIPT_NAME=" + std::string(GetFileName(cgiRoute.script_)));

    if (!cgiRoute.resource_.empty() && cgiRoute.resource_ != "/")
    {
      envp_.emplace_back("PATH_INFO=" + cgiRoute.resource_);
      envp_.emplace_back("PATH_TRANSLATED=" + cgiRoute.fullResource_);
    }
    else
    {
      envp_.emplace_back("PATH_INFO=");
      envp_.emplace_back("PATH_TRANSLATED=");
    }

    const std::string_view query_string{httpRequest.GetQuery()};
    if (query_string.empty())
      envp_.emplace_back("QUERY_STRING=");
    else
      envp_.emplace_back("QUERY_STRING=" + std::string(query_string));

    if (!body_.empty())
    {
      envp_.emplace_back("CONTENT_LENGTH=" + std::to_string(body_.length()));

      std::string_view content_type{httpRequest.GetFirstHeaderValueOf("content-type")};
      if (!content_type.empty())
        envp_.emplace_back("CONTENT_TYPE=" + std::string(content_type));
      else
        envp_.emplace_back("CONTENT_TYPE=application/octet-stream");
    }

    {
      envp_.emplace_back("REMOTE_ADDR=" + GetAddressSegment(clientInfo));
      envp_.emplace_back("REMOTE_HOST=" + GetAddressSegment(clientInfo));
      envp_.emplace_back("REQUEST_METHOD=" + std::string{httpRequest.GetMethodString()});
    }

    {
      envp_.emplace_back("SERVER_NAME=" + GetServerName(hostname_, ipPortServer.ip));
      envp_.emplace_back("SERVER_PORT=" + ipPortServer.port);
      envp_.emplace_back("SERVER_PROTOCOL=" + std::string{httpRequest.GetVersion()});
      envp_.emplace_back("SERVER_SOFTWARE=" CGI_SERVER_SOFTWARE);
    }

    {
      std::string env_var;
      const HTTP::Headers& http_headers = httpRequest.GetHeaders();
      for (auto& [header, values] : http_headers)
      {
        if (std::find(filter_.begin(), filter_.end(), header) != filter_.end())
          continue;

        env_var.reserve(header.length() + values.size() * 8);
        env_var.append("HTTP_").append(header);
        String::ToUpper(String::ReplaceOccurrence(env_var, '-', '_'));
        env_var.append("=");

        if (!values.empty())
        {
          for (const auto& value : values)
            env_var.append(value).append(", ");
          env_var.resize(env_var.size() - 2);
        }
        envp_.emplace_back(std::move(env_var));
      }
    }
  }

  const std::string& CGIRequest::GetBody(void) const noexcept
  {
    return body_;
  }

  const std::string& CGIRequest::GetExecutable(void) const noexcept
  {
    return executable_;
  }

  const std::vector<std::string>& CGIRequest::GetArgv(void) const noexcept
  {
    return argv_;
  }

  const std::vector<std::string>& CGIRequest::GetEnvp(void) const noexcept
  {
    return envp_;
  }

  std::size_t& CGIRequest::GetLeftover(void) noexcept
  {
    return leftover_;
  }

  bool CGIRequest::IsClosedConnection(void) const noexcept
  {
    return isClosedConnection_;
  }

  const RouteView& CGIRequest::GetRouteView(void) const noexcept
  {
    return *routeView_;
  }

  const IpPort& CGIRequest::GetIpPortServer(void) const noexcept
  {
    return ipPortServer_;
  }

  const std::string& CGIRequest::GetHostname(void) const noexcept
  {
    return hostname_;
  }

}  // namespace cgi
