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
    OK = 200,
    CREATED = 201,
    NO_CONTENT = 204,

    BAD_REQUEST = 400,
    UNAUTHORIZED = 401,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    PAYLOAD_TOO_LARGE = 413,
    URI_TOO_LONG = 414,
    UNSUPPORTED_MEDIA_TYPE = 415,
    RANGE_NOT_SATISFIABLE = 416,

    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501,
    BAD_GATEWAY = 502,
    SERVICE_UNAVAILABLE = 503,
    GATEWAY_TIMEOUT = 504,
    HTTP_VERSION_NOT_SUPPORTED = 505
  };

  Status ToHTTPStatus(ValidationResult vr);
  std::string_view ToReasonPhrase(Status status);
}  // namespace HTTP

#endif
