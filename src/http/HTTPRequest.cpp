/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPRequest.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/12 12:14:12 by bewong        #+#    #+#                 */
/*   Updated: 2026/04/02 15:57:44 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "http/HTTPRequest.hpp"

#include <string>
#include <string_view>
#include <vector>

#include "http/HTTPTypes.hpp"
#include "http/HTTPUtils.hpp"

// | Input      | Normalized |
// | ---------- | ---------- |
// | //         | /          | collapse repeated /
// | /./        | /          | remove . segments
// | /a/./b     | /a/b       |
// | /a/b/../c  | /a/c       |resolve .. by popping
// | /a/b/..    | /a         |
// | /a/../../x | reject     | if .. go above root -> false

bool HTTPRequest::NormalizePath(std::string_view in, std::string& out)
{
  out.clear();
  if (in.empty() || in[0] != '/')
    return false;
  std::vector<std::string_view> segments;
  segments.reserve(8);
  std::size_t segStart = 1;
  while (segStart < in.size())
  {
    while (segStart < in.size() && in[segStart] == '/')
      ++segStart;
    if (segStart >= in.size())
      break;
    std::size_t segEnd = segStart;
    while (segEnd < in.size() && in[segEnd] != '/')
      ++segEnd;
    std::string_view segment = in.substr(segStart, segEnd - segStart);
    if (segment == "..")
    {
      if (segments.empty())
        return false;
      segments.pop_back();
    }
    else if (segment != ".")
      segments.push_back(segment);
    segStart = segEnd;
  }
  out = "/";
  for (std::size_t k = 0; k < segments.size(); ++k)
  {
    out.append(segments[k]);
    if (k + 1 < segments.size())
      out.push_back('/');
  }
  const bool keepTrailing = (in.size() > 1 && in.back() == '/');
  if (keepTrailing && out.size() > 1)
  {
    out.push_back('/');
  }
  return true;
}

/************************************************** Getter ***********************************************************/

HTTP::Method HTTPRequest::GetMethod(void) const
{
  return method_;
}

std::string_view HTTPRequest::GetMethodString(void) const
{
  return HTTP::MethodToString(GetMethod());
}

std::string_view HTTPRequest::GetTarget(void) const
{
  return target_;
}

std::string_view HTTPRequest::GetRawPath(void) const
{
  return uri_;
}

std::string_view HTTPRequest::GetQuery(void) const
{
  return query_;
}

std::string_view HTTPRequest::GetPath(void) const
{
  return path_;
}

std::string_view HTTPRequest::GetHost(void) const
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

const std::optional<HTTPRequest::Path>& HTTPRequest::GetBodyFilePath(void) const
{
  return bodyFilePath_;
}
const HTTPRequest::CookieMap& HTTPRequest::GetCookies(void) const
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
  if (!NormalizePath(decoded, normalized))
    return false;
  uri_ = raw_path;
  path_ = std::move(normalized);
  return true;
}

void HTTPRequest::SetMethod(std::string_view method)
{
  method_ = HTTP::StringToMethod(method);
}

void HTTPRequest::SetComplete(bool complete)
{
  isComplete_ = complete;
}

void HTTPRequest::SetBodyFilePath(Path p)
{
  bodyFilePath_ = std::move(p);
}

void HTTPRequest::SetCookies(CookieMap&& cookies)
{
  cookies_ = std::move(cookies);
}

bool HTTPRequest::IsComplete(void) const
{
  return isComplete_;
}

bool HTTPRequest::HasCookie(std::string_view name) const
{
  return cookies_.find(name) != cookies_.end();
}

void HTTPRequest::Clear(void)
{
  method_ = HTTP::Method::kUnsupported;
  target_.clear();
  uri_.clear();
  query_.clear();
  isComplete_ = false;
  path_.clear();
  bodyFilePath_.reset();
}
