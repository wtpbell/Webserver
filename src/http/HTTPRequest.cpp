/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPRequest.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/12 12:14:12 by bewong        #+#    #+#                 */
/*   Updated: 2026/01/08 19:57:51 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "http/HTTPRequest.hpp"

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
  // TODO: will we enfore leading slash -> absolute paths, match RFC 9112
  if (in.empty() || in[0] != '/')
    return false;
  // preserve the trailing slash if the raw path end with '/'
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
    // const std::size_t len = segEnd - segStart;
    std::string_view segment = in.substr(segStart, segEnd - segStart);
    if (segment == "..")
    {
      if (segments.empty())
        return false;       // Reject paths that go above root
                            // for server reject it (403/400)
      segments.pop_back();  // /a/b/../c -> /a/b/c
    }
    else if (segment != ".")  // ignore /a/./b/ -> /a/b/
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
  const HTTP::Headers& headers = GetHeaders();
  auto it = headers.find("host");
  if (it == headers.end())
    return {};
  std::string_view host = it->second;
  auto colon = host.find(':');
  return (colon == std::string_view::npos) ? host : host.substr(0, colon);
}

/************************************************** Setter ***********************************************************/

void HTTPRequest::SetMethod(HTTP::Method method)
{
  method_ = method;
}

void HTTPRequest::SetTarget(std::string_view target)
{
  target_.assign(target);
  const size_t q = target_.find('?');
  std::string_view raw_path;
  query_.clear();

  if (q != std::string::npos)
  {
    raw_path = std::string_view(target_.data(), q);
    query_.assign(target_.data() + q + 1);
  }
  else
    raw_path = std::string_view(target_);
  // URL-decode path FIRST
  std::string decoded = HTTP::wire::URLDecode(raw_path);
  // Normalize decoded path
  std::string normalized;
  if (!NormalizePath(decoded, normalized))
    // signal parser error (400)
    return;
  uri_ = raw_path;  // optional: keep raw URI
  path_ = std::move(normalized);
}

void HTTPRequest::SetMethod(std::string_view method)
{
  method_ = HTTP::StringToMethod(method);
}

void HTTPRequest::SetComplete(bool complete)
{
  isComplete_ = complete;
}

bool HTTPRequest::IsComplete(void) const
{
  return isComplete_;
}

const HTTP::Headers& HTTPRequest::Headers() const
{
  return GetHeaders();
}
