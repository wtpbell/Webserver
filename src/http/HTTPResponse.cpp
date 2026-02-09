/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPResponse.cpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/14 10:48:15 by bewong        #+#    #+#                 */
/*   Updated: 2026/02/06 17:31:03 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "http/HTTPResponse.hpp"

#include <cstdint>
#include <string>

#include "http/HTTPStatus.hpp"

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
