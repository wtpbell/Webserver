/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPStatus.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/13 19:06:15 by bewong        #+#    #+#                 */
/*   Updated: 2026/01/13 19:06:15 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPSTATUS_HPP
#define HTTPSTATUS_HPP

#include <cstdint>
#include <string_view>

#include "http/HTTPValidator.hpp"

namespace HTTP
{
  enum class Status : std::uint16_t
  {
    kOk = 200,
    kCreated = 201,
    kAccepted = 202,
    kNoContent = 204,

    kMovedPermanently = 301,
    kFound = 302,

    kBadRequest = 400,
    kUnauthorized = 401,
    kForbidden = 403,
    kNotFound = 404,
    kMethodNotAllowed = 405,
    kPayloadTooLarge = 413,
    kURITooLong = 414,
    kUnsupportedMediaType = 415,
    kRangeNotSatisfiable = 416,

    kInternalServerError = 500,
    kNotImplemented = 501,
    kBadGateway = 502,
    kServiceUnavailable = 503,
    kGatewayTimeout = 504,
    kVersionNotSupported = 505
  };

  Status ToHTTPStatus(ValidationResult vr);
  std::string_view ToReasonPhrase(Status status);
}  // namespace HTTP

#endif
