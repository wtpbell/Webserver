/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPValidator.cpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/13 19:07:39 by bewong        #+#    #+#                 */
/*   Updated: 2026/04/02 11:53:26 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "http/HTTPValidator.hpp"

#include <string>
#include <string_view>
#include <vector>

#include "http/HTTPCookie.hpp"
#include "http/HTTPTypes.hpp"
#include "utils/string.hpp"

namespace
{
  bool SplitCommaTokens(std::string_view v, std::vector<std::string>& out)
  {
    std::size_t pos = 0;

    while (pos < v.size())
    {
      const std::size_t start = pos;
      while (pos < v.size() && v[pos] != ',')
        ++pos;
      auto token = String::ToLower(String::Trim(v.substr(start, pos - start)));
      if (token.empty())
        return false;
      out.push_back(token);
      if (pos == v.size() - 1 && v[pos] == ',')
        return false;
      if (pos < v.size() && v[pos] == ',')
        ++pos;
    }

    return true;
  }
}  // namespace

namespace HTTP::validate
{
  bool IsValidHeaderName(std::string_view name)
  {
    if (name.empty())
      return false;
    for (unsigned char c : name)
    {
      if (HTTP_TCHAR.find(c) == std::string_view::npos)
        return false;
    }
    return true;
  }

  bool IsValidPath(std::string_view target)
  {
    if (target.empty() || target[0] != '/')
      return false;
    if (target.find('\0') != std::string_view::npos)
      return false;
    return true;
  }

  bool IsValidTarget(std::string_view method, std::string_view target)
  {
    if (method == "OPTIONS" && target == "*")
      return true;

    if (String::StartsWith(target, "http://") || String::StartsWith(target, "https://"))
      return false;

    return IsValidPath(target);
  }

  // Reject CR/LF and other CTLs (0x00-0x1F, 0x7F) except HTAB is sometimes tolerated.
  // We'll reject everything < 0x20 plus DEL, including HTAB for strictness,
  bool IsValidHeaderValue(std::string_view value)
  {
    for (unsigned char c : value)
    {
      if (c < 0x20 || c == 0x7F)
        return false;
    }
    return true;
  }

  // From RFC 6265: cookie-value = *cookie-octet / ( DQUOTE *cookie-octet DQUOTE )
  bool IsValidCookieValue(std::string_view value)
  {
    for (unsigned char c : value)
    {
      if (c == ',' || c == ';' || c == ' ' || c == '"' || c == '\\')
        return false;
    }
    return true;
  }

  /*
  Accept:
    session_id=abc123
    a=b; c=d
    a=b; c=
    a=b;;c=d (because empty segment is skipped)
  */
  bool IsValidCookieHeader(std::string_view header)
  {
    return HTTP::cookie::ParseCookieHeader(header, nullptr);
  }

  bool IsValidSid(std::string_view id)
  {
    if (id.size() != 32)
      return false;
    for (unsigned char c : id)
      if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')))
        return false;
    return true;
  }

}  // namespace HTTP::validate

ValidationResult ValidateStartLine(const HTTPRequest& request)
{
  if (!HTTP::validate::IsValidVersion(request.GetVersion()))
    return ValidationResult::kVersionNotSupported;
  if (!HTTP::validate::IsValidMethod(request.GetMethodString()))
    return ValidationResult::kNotImplemented;
  if (!HTTP::validate::IsValidTarget(request.GetMethodString(), request.GetTarget()))
    return ValidationResult::kBadRequest;
  return ValidationResult::kOk;
}

ValidationResult ValidateHeader(const HTTPRequest& request)
{
  if (request.GetHeaderValueCountOf("host") != 1)
    return ValidationResult::kBadRequest;
  for (const auto& [name, values] : request.GetHeaders())
  {
    if (!HTTP::validate::IsValidHeaderName(name))
      return ValidationResult::kBadRequest;
    for (const auto& v : values)
    {
      if (!HTTP::validate::IsValidHeaderValue(v))
        return ValidationResult::kBadRequest;
    }
  }
  return ValidationResult::kOk;
}

/*
    cookie-header = "Cookie:" OWS cookie-string OWS
    cookie-string = cookie-pair *( ";" SP cookie-pair )
    cookie-pair   = cookie-name "=" cookie-value
    e.g Cookie: SID=31d4d96e407aad42; lang=en-US
*/
ValidationResult ValidateCookies(const HTTPRequest& request)
{
  const auto* vals = request.GetHeaderValuesOf("cookie");
  if (!vals || vals->empty())
    return ValidationResult::kOk;
  if (vals->size() != 1)
    return ValidationResult::kBadRequest;
  if (!HTTP::validate::IsValidCookieHeader((*vals)[0]))
    return ValidationResult::kBadRequest;
  return ValidationResult::kOk;
}

/*
    Handles multiple headers
    Handles comma-separated values
    Rejects unsupported encodings
*/
ValidationResult ValidateTransferEncoding(const HTTPRequest& request)
{
  const auto* vals = request.GetHeaderValuesOf("transfer-encoding");
  if (!vals || vals->empty())
    return ValidationResult::kOk;
  std::vector<std::string> tokens;
  for (const auto& v : *vals)
  {
    if (!SplitCommaTokens(v, tokens))
      return ValidationResult::kBadRequest;
  }
  if (tokens.size() != 1)
    return ValidationResult::kBadRequest;
  return (tokens[0] == "chunked") ? ValidationResult::kOk : ValidationResult::kNotImplemented;
}

// may appear multiple times, but only if all values are identical
ValidationResult ValidateContentLength(const HTTPRequest& request)
{
  const auto* te = request.GetHeaderValuesOf("transfer-encoding");
  const auto* cl = request.GetHeaderValuesOf("content-length");

  const bool has_te = te && !te->empty();
  const bool has_cl = cl && !cl->empty();

  if (has_te && has_cl)
    return ValidationResult::kBadRequest;
  if (!has_cl)
    return ValidationResult::kOk;
  auto len = request.GetContentLength();
  if (!len)
    return ValidationResult::kBadRequest;
  if (*len > HTTP::kMaxBodySize)
    return ValidationResult::kPayloadTooLarge;
  return ValidationResult::kOk;
}

ValidationResult ValidateRequest(const HTTPRequest& request)
{
  if (auto r = ValidateStartLine(request); r != ValidationResult::kOk)
    return r;
  if (auto r = ValidateHeader(request); r != ValidationResult::kOk)
    return r;
  if (auto r = ValidateCookies(request); r != ValidationResult::kOk)
    return r;
  if (auto r = ValidateTransferEncoding(request); r != ValidationResult::kOk)
    return r;
  if (auto r = ValidateContentLength(request); r != ValidationResult::kOk)
    return r;
  return ValidationResult::kOk;
}
