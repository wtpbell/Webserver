/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPRequest.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/12 12:14:12 by bewong        #+#    #+#                 */
/*   Updated: 2026/04/26 13:50:28 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "http/HTTPRequest.hpp"

#include <string>
#include <string_view>
#include <vector>

#include "http/HTTPTypes.hpp"
#include "http/HTTPUtils.hpp"

/************************************************** Getter ***********************************************************/

HTTP::Method HTTPRequest::GetMethod() const
{
  return method_;
}

std::string_view HTTPRequest::GetMethodString() const
{
  return HTTP::MethodToString(GetMethod());
}

std::string_view HTTPRequest::GetTarget() const
{
  return target_;
}

std::string_view HTTPRequest::GetRawPath() const
{
  return uri_;
}

std::string_view HTTPRequest::GetQuery() const
{
  return query_;
}

std::string_view HTTPRequest::GetPath() const
{
  return path_;
}

std::string_view HTTPRequest::GetHost() const
{
  std::string_view host = GetFirstHeaderValueOf("host");
  if (host.empty())
    return {};

  if (host.front() == '[')
  {
    auto end = host.find(']');
    return end == std::string_view::npos ? std::string_view{} : host.substr(1, end - 1);
  }

  auto colon = host.find(':');
  return colon == std::string_view::npos ? host : host.substr(0, colon);
}

const std::optional<HTTPRequest::Path>& HTTPRequest::GetBodyFilePath() const
{
  return bodyFilePath_;
}

const HTTPRequest::CookieMap& HTTPRequest::GetCookies() const
{
  return cookies_;
}

std::string_view HTTPRequest::GetCookieOr(std::string_view name, std::string_view def) const
{
  auto it = cookies_.find(name);
  return it == cookies_.end() ? def : std::string_view(it->second);
}
/************************************************** Setter ***********************************************************/

void HTTPRequest::SetMethod(HTTP::Method method)
{
  method_ = method;
}

bool HTTPRequest::SetTarget(std::string_view target)
{
  target_.assign(target);
  const std::size_t q = target_.find('?');
  std::string_view raw_path;
  query_.clear();

  if (q != std::string::npos)
  {
    raw_path = std::string_view(target_.data(), q);
    query_.assign(target_.data() + q + 1);
  }
  else
    raw_path = std::string_view(target_);
  std::string decoded = HTTP::wire::URLDecode(raw_path);
  std::string normalized;
  if (!HTTP::wire::NormalizePath(decoded, normalized))
    return false;
  uri_ = raw_path;
  path_ = std::move(normalized);
  return true;
}

void HTTPRequest::SetMethod(std::string_view method)
{
  method_ = HTTP::StringToMethod(method);
}

void HTTPRequest::SetBodyFilePath(Path p)
{
  bodyFilePath_ = std::move(p);
}

void HTTPRequest::SetCookies(CookieMap&& cookies)
{
  cookies_ = std::move(cookies);
}

bool HTTPRequest::HasCookie(std::string_view name) const
{
  return cookies_.find(name) != cookies_.end();
}

void HTTPRequest::Clear()
{
  method_ = HTTP::Method::kUnsupported;
  target_.clear();
  uri_.clear();
  query_.clear();
  path_.clear();
  bodyFilePath_.reset();
}
