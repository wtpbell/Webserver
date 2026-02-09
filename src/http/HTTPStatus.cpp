/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPStatus.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/13 19:07:24 by bewong        #+#    #+#                 */
/*   Updated: 2026/01/13 19:07:24 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "http/HTTPStatus.hpp"

#include <cassert>
#include <string_view>

#include "http/HTTPValidator.hpp"

namespace HTTP
{
  Status ToHTTPStatus(ValidationResult vr)
  {
    switch (vr)
    {
      case ValidationResult::OK:
        return Status::OK;
      case ValidationResult::BadRequest:
        return Status::BAD_REQUEST;
      case ValidationResult::URITooLong:
        return Status::URI_TOO_LONG;
      case ValidationResult::PayloadTooLarge:
        return Status::PAYLOAD_TOO_LARGE;
      case ValidationResult::NotImplemented:
        return Status::NOT_IMPLEMENTED;
      case ValidationResult::VersionNotSupported:
        return Status::HTTP_VERSION_NOT_SUPPORTED;
    }
    assert(false && "Invalid validation result");
    __builtin_unreachable();
  }

  std::string_view ToReasonPhrase(Status status)
  {
    switch (status)
    {
      case Status::OK:
        return "OK";
      case Status::CREATED:
        return "Created";
      case Status::NO_CONTENT:
        return "No Content";

      case Status::BAD_REQUEST:
        return "Bad Request";
      case Status::UNAUTHORIZED:
        return "Unauthorized";
      case Status::FORBIDDEN:
        return "Forbidden";
      case Status::NOT_FOUND:
        return "Not Found";
      case Status::METHOD_NOT_ALLOWED:
        return "Method Not Allowed";
      case Status::PAYLOAD_TOO_LARGE:
        return "Payload Too Large";
      case Status::URI_TOO_LONG:
        return "URI Too Long";
      case Status::UNSUPPORTED_MEDIA_TYPE:
        return "Unsupported Media Type";
      case Status::RANGE_NOT_SATISFIABLE:
        return "Range Not Satisfiable";

      case Status::INTERNAL_SERVER_ERROR:
        return "Internal Server Error";
      case Status::NOT_IMPLEMENTED:
        return "Not Implemented";
      case Status::BAD_GATEWAY:
        return "Bad Gateway";
      case Status::SERVICE_UNAVAILABLE:
        return "Service Unavailable";
      case Status::GATEWAY_TIMEOUT:
        return "Gateway Timeout";
      case Status::HTTP_VERSION_NOT_SUPPORTED:
        return "HTTP Version Not Supported";
    }
    assert(false && "Invalid status");
    __builtin_unreachable();
  }

}  // namespace HTTP
