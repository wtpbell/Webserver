/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPResponse.cpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/14 10:48:15 by bewong        #+#    #+#                 */
/*   Updated: 2026/04/24 16:29:00 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "http/HTTPResponse.hpp"

#include <cstdint>
#include <string>

#include "http/HTTPStatus.hpp"

HTTPResponse::HTTPResponse(HTTP::Status status, std::string body)
    : HTTPMessage(std::string{HTTP::kVERSION}, HTTP::Headers{}, std::move(body))
{
  SetStatus(status);
}

HTTPResponse::HTTPResponse(HTTP::Status status, HTTP::Headers headers, std::string body)
    : HTTPMessage(std::string{HTTP::kVERSION}, std::move(headers), std::move(body))
{
  SetStatus(status);
}

std::uint16_t HTTPResponse::GetStatusCode(void) const
{
  return static_cast<std::uint16_t>(status_);
}

const std::string& HTTPResponse::GetReason(void) const
{
  return reason_;
}

void HTTPResponse::SetStatus(HTTP::Status status)
{
  status_ = status;
  reason_ = ToReasonPhrase(status);
}
