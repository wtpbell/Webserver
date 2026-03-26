/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGIRequest.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/20 15:28:10 by jboon         #+#    #+#                 */
/*   Updated: 2026/02/24 08:39:09 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "cgi/CGIRequest.hpp"

#include <algorithm>
#include <array>
#include <string>
#include <string_view>
#include <vector>

#include "cgi/CGI.hpp"
#include "http/HTTPRequest.hpp"
#include "string.hpp"

namespace cgi
{
  namespace
  {
    std::string_view GetAddressSegment(std::string_view socket_info)
    {
      std::size_t port_delimiter{socket_info.find_last_of(':')};
      if (port_delimiter == std::string_view::npos)
        return socket_info;
      return socket_info.substr(0, port_delimiter);
    }

    std::string_view GetPortNumber(std::string_view socket_info)
    {
      std::size_t port_delimiter{socket_info.find_last_of(':')};
      if (port_delimiter == std::string_view::npos)
        return {"UNKNOWN"};
      return socket_info.substr(port_delimiter + 1);
    }
  }  // namespace

  std::array<std::string_view, 7> CGIRequest::filter_ = {
      "content-length",     "content-type",        "authorization", "www-authenticate",
      "proxy-authenticate", "proxy-authorization", "host"};

  // TODO: Not all meta-variables are properly set yet (will be resolved next PR)
  CGIRequest::CGIRequest(const HTTPRequest& http_request, const struct CGIRoute& route, std::string_view server_info,
                         std::string_view client_info)
      : body_(http_request.GetBody()), leftover_(body_.length())
  {
    argv_.emplace_back(route.script_);

    envp_.emplace_back("GATEWAY_INTERFACE=CGI/1.1");
    envp_.emplace_back("SCRIPT_NAME=" + route.script_.filename().string());

    if (!route.resource_.empty())
    {
      envp_.emplace_back("PATH_INFO=" + route.resource_.string());
      envp_.emplace_back("PATH_TRANSLATED=" + route.full_resource_.string());
    }

    const std::string_view query_string{http_request.GetQuery()};
    if (query_string.empty())
      envp_.emplace_back("QUERY_STRING=");
    else
      envp_.emplace_back("QUERY_STRING=" + std::string(query_string));

    if (!body_.empty())
    {
      envp_.emplace_back("CONTENT_LENGTH=" + std::to_string(body_.length()));
      std::string_view content_type{http_request.GetFirstHeaderValueOf("content-type")};

      if (!content_type.empty())
        envp_.emplace_back("CONTENT_TYPE=" + std::string(content_type));
      else
        envp_.emplace_back("CONTENT_TYPE=application/octet-stream");
    }

    {
      envp_.emplace_back("REMOTE_ADDR=" + std::string{GetAddressSegment(client_info)});
      // TODO: should be fully qualified domain name of the client, however I'm skipping this because it requires access
      // to the (next PR) socket class REMOTE_HOST   = "" | hostname | hostnumber
      envp_.emplace_back("REMOTE_HOST=" + std::string{GetAddressSegment(client_info)});
      envp_.emplace_back("REQUEST_METHOD=" + std::string{http_request.GetMethodString()});
    }

    {
      // TODO: SERVER_NAME should be hostname (set in configuration, next PR)
      // server-name = hostname | ipv4-address | ( "[" ipv6-address "]" )
      envp_.emplace_back("SERVER_NAME=[" + std::string{GetAddressSegment(server_info)} + "]");
      envp_.emplace_back("SERVER_PORT=" + std::string{GetPortNumber(server_info)});
      envp_.emplace_back("SERVER_PROTOCOL=" + std::string{http_request.GetVersion()});
      envp_.emplace_back("SERVER_SOFTWARE=" CGI_SERVER_SOFTWARE);
    }

    {
      std::string env_var;
      const HTTP::Headers& http_headers = http_request.GetHeaders();
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
}  // namespace cgi
