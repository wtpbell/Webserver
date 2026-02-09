/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPMessage.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/09 17:28:14 by bewong        #+#    #+#                 */
/*   Updated: 2026/02/06 17:24:18 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "http/HTTPMessage.hpp"

#include <cassert>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "http/HTTPTypes.hpp"
#include "http/HTTPValidator.hpp"
#include "string.hpp"

HTTPMessage::HTTPMessage(std::string ver, HTTP::Headers headers, std::string body)
    : version_(std::move(ver)), headers_(std::move(headers)), body_(std::move(body))
{
#ifndef NDEBUG
  for (const auto& [name, _] : headers_)
    assert(HTTP::validate::IsValidHeaderName(name));
#endif
}

/************************************************** Getter ***********************************************************/
std::string_view HTTPMessage::GetVersion(void) const
{
  return version_;
}

const HTTP::Headers& HTTPMessage::GetHeaders(void) const
{
  return headers_;
}

HTTP::Headers& HTTPMessage::GetHeaders(void)
{
  return headers_;
}

const std::vector<std::string>* HTTPMessage::GetHeaderValuesOf(std::string_view name) const
{
  auto it = headers_.find(String::ToLower(name));
  if (it == headers_.end())
    return nullptr;
  return &it->second;
}

std::string_view HTTPMessage::GetFirstHeaderValueOf(std::string_view name) const
{
  auto vals = GetHeaderValuesOf(name);
  if (!vals || vals->empty())
    return {};
  return (*vals)[0];
}

const std::string& HTTPMessage::GetBody(void) const
{
  return body_;
}

/************************************************** Setter ***********************************************************/
void HTTPMessage::SetVersion(std::string ver)
{
  version_ = std::move(ver);
}
void HTTPMessage::SetBody(const std::string& body)
{
  body_ = body;
}
void HTTPMessage::SetHeader(std::string_view name, std::string_view value)
{
  auto& vec = headers_[String::ToLower(name)];
  vec.clear();
  vec.emplace_back(String::Trim(value));
}

/************************************************** Helper ***********************************************************/
void HTTPMessage::AddHeader(std::string_view name, std::string_view value)
{
  headers_[String::ToLower(name)].emplace_back(String::Trim(value));
}

void HTTPMessage::AppendBody(std::string_view body)
{
  body_.append(body);
}

void HTTPMessage::RemoveHeader(std::string_view name)
{
  headers_.erase(String::ToLower(name));
}

std::size_t HTTPMessage::GetHeaderValueCountOf(std::string_view name) const
{
  auto it = headers_.find(String::ToLower(name));
  return (it == headers_.end()) ? 0 : it->second.size();
}

bool HTTPMessage::HasHeader(std::string_view name) const
{
  return headers_.find(String::ToLower(name)) != headers_.end();
}

std::optional<std::size_t> HTTPMessage::GetContentLength(void) const
{
  auto vals = GetHeaderValuesOf("content-length");
  if (!vals || vals->empty())
    return std::nullopt;

  std::optional<std::size_t> first;
  for (const auto& val : *vals)
  {
    std::size_t n{};
    if (!String::ConvertToNumber(val, n, 10))
      return std::nullopt;
    if (!first)
      first = n;
    else if (*first != n)
      return std::nullopt;
  }
  return first;
}

bool HTTPMessage::IsChunked(void) const
{
  auto vals = GetHeaderValuesOf("transfer-encoding");
  if (!vals)
    return false;

  for (const auto& v : *vals)
  {
    if (String::ToLower(v) == "chunked")
      return true;
  }
  return false;
}
