/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPMessage.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/09 17:28:14 by bewong        #+#    #+#                 */
/*   Updated: 2026/01/08 19:57:43 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "http/HTTPMessage.hpp"

#include <cassert>
#include <charconv>      // std::from_chars
#include <system_error>  // std::errc

#include "string.hpp"

HTTPMessage::HTTPMessage(std::string ver, HTTP::Headers headers, std::string body)
    : version_(std::move(ver)), headers_(std::move(headers)), body_(std::move(body))
{
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

std::string_view HTTPMessage::GetHeader(std::string_view name) const
{
  auto it = headers_.find(String::ToLower(name));
  if (it == headers_.end())
    return {};
  return it->second;
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
  headers_[String::ToLower(name)] = std::string(value);
}

/************************************************** Helper ***********************************************************/
// append with comma for “list headers”, but not for set-cookie for later use
void HTTPMessage::AddHeader(std::string_view name, std::string_view value)
{
  std::string key = String::ToLower(name);

  auto [it, inserted] = headers_.try_emplace(key, std::string(value));
  if (!inserted)
  {
    if (key == "set-cookie")
    {
      // Preserve multiple Set-Cookie headers
      it->second.append("\n");
      it->second.append(value);
    }
    else
    {
      it->second.append(", ");
      it->second.append(value);
    }
  }
}

void HTTPMessage::AppendBody(std::string_view body)
{
  body_.append(body);
}

void HTTPMessage::RemoveHeader(std::string_view name)
{
  headers_.erase(String::ToLower(name));
}

bool HTTPMessage::HasHeader(std::string_view name) const
{
  return headers_.find(String::ToLower(name)) != headers_.end();
}

// use std:::from_chars instead of std::stol as it wont throw exceptions
std::optional<std::size_t> HTTPMessage::GetContentLength(void) const
{
  auto it = headers_.find("content-length");
  if (it == headers_.end())
    return std::nullopt;
  std::string_view val = String::Trim(it->second);
  if (val.empty())
    return std::nullopt;
  std::size_t res{};
  const char* first = val.data();
  const char* last = val.data() + val.size();
  auto [ptr, ec] = std::from_chars(first, last, res, 10);
  // ec != std::errc() => invalid or out of range
  // ptr != last       => trailing junk like "123abc"
  if (ec != std::errc() || ptr != last)
    return std::nullopt;
  return res;
}

bool HTTPMessage::IsChunked(void) const
{
  auto it = headers_.find("transfer-encoding");
  if (it == headers_.end())
    return false;
  return it->second.find("chunked") != std::string::npos;
}
