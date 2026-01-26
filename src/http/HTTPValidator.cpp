/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPValidator.cpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/13 19:07:39 by bewong        #+#    #+#                 */
/*   Updated: 2026/01/13 19:07:39 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "http/HTTPValidator.hpp"

#include "string.hpp"

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

    if (String::starts_with(target, "http://") || String::starts_with(target, "https://"))
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
}  // namespace HTTP::validate

ValidationResult ValidateStartLine(const HTTPRequest& req)
{
  if (!HTTP::validate::IsValidVersion(req.GetVersion()))
    return ValidationResult::VersionNotSupported;
  if (!HTTP::validate::IsValidMethod(req.GetMethodString()))
    return ValidationResult::NotImplemented;
  if (!HTTP::validate::IsValidTarget(req.GetMethodString(), req.GetTarget()))
    return ValidationResult::BadRequest;
  return ValidationResult::OK;
}

ValidationResult ValidateHeader(const HTTPRequest& req)
{
  if (req.GetHeaderValueCountOf("host") != 1)
    return ValidationResult::BadRequest;
  for (const auto& [name, values] : req.Headers())
  {
    if (!HTTP::validate::IsValidHeaderName(name))
      return ValidationResult::BadRequest;
    if (name == "cookie")
    {
      if (values.size() != 1 || !HTTP::validate::IsValidHeaderValue(values[0]))
        return ValidationResult::BadRequest;
      continue;
    }
    for (const auto& v : values)
    {
      if (!HTTP::validate::IsValidHeaderValue(v))
        return ValidationResult::BadRequest;
    }
  }
  return ValidationResult::OK;
}

/*
    Handles multiple headers
    Handles comma-separated values
    Rejects unsupported encodings
*/
ValidationResult ValidateTransferEncoding(const HTTPRequest& req)
{
  const auto* vals = req.GetHeaderValuesOf("transfer-encoding");
  if (!vals || vals->empty())
    return ValidationResult::OK;
  std::vector<std::string> tokens;
  for (const auto& v : *vals)
  {
    if (!SplitCommaTokens(v, tokens))
      return ValidationResult::BadRequest;
  }
  if (tokens.size() != 1)
    return ValidationResult::BadRequest;
  return (tokens[0] == "chunked") ? ValidationResult::OK : ValidationResult::NotImplemented;
}

// may appear multiple times, but only if all values are identical
ValidationResult ValidateContentLength(const HTTPRequest& req)
{
  const auto* te = req.GetHeaderValuesOf("transfer-encoding");
  const auto* cl = req.GetHeaderValuesOf("content-length");

  const bool has_te = te && !te->empty();
  const bool has_cl = cl && !cl->empty();

  if (has_te && has_cl)
    return ValidationResult::BadRequest;
  if (!has_cl)
    return ValidationResult::OK;
  auto len = req.GetContentLength();
  if (!len)
    return ValidationResult::BadRequest;
  if (*len > HTTP::kMaxBodySize)
    return ValidationResult::PayloadTooLarge;
  return ValidationResult::OK;
}

ValidationResult ValidateRequest(const HTTPRequest& req)
{
  if (auto r = ValidateStartLine(req); r != ValidationResult::OK)
    return r;
  if (auto r = ValidateHeader(req); r != ValidationResult::OK)
    return r;
  if (auto r = ValidateTransferEncoding(req); r != ValidationResult::OK)
    return r;
  if (auto r = ValidateContentLength(req); r != ValidationResult::OK)
    return r;
  return ValidationResult::OK;
}
