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
      case ValidationResult::kOk:
        return Status::kOk;
      case ValidationResult::kBadRequest:
        return Status::kBadRequest;
      case ValidationResult::kURITooLong:
        return Status::kURITooLong;
      case ValidationResult::kPayloadTooLarge:
        return Status::kPayloadTooLarge;
      case ValidationResult::kNotImplemented:
        return Status::kNotImplemented;
      case ValidationResult::kVersionNotSupported:
        return Status::kVersionNotSupported;
      case ValidationResult::kInternalServerError:
        return Status::kInternalServerError;
    }
    assert(false && "Invalid validation result");
    __builtin_unreachable();
  }

  std::string_view ToReasonPhrase(Status status)
  {
    switch (status)
    {
      case Status::kOk:
        return "OK";
      case Status::kCreated:
        return "Created";
      case Status::kAccepted:
        return "Accepted";
      case Status::kNoContent:
        return "No Content";
      case Status::kMovedPermanently:
        return "Moved Permanently";
      case Status::kFound:
        return "Found";

      case Status::kBadRequest:
        return "Bad Request";
      case Status::kUnauthorized:
        return "Unauthorized";
      case Status::kForbidden:
        return "Forbidden";
      case Status::kNotFound:
        return "Not Found";
      case Status::kMethodNotAllowed:
        return "Method Not Allowed";
      case Status::kPayloadTooLarge:
        return "Payload Too Large";
      case Status::kURITooLong:
        return "URI Too Long";
      case Status::kUnsupportedMediaType:
        return "Unsupported Media Type";
      case Status::kRangeNotSatisfiable:
        return "Range Not Satisfiable";

      case Status::kInternalServerError:
        return "Internal Server Error";
      case Status::kNotImplemented:
        return "Not Implemented";
      case Status::kBadGateway:
        return "Bad Gateway";
      case Status::kServiceUnavailable:
        return "Service Unavailable";
      case Status::kGatewayTimeout:
        return "Gateway Timeout";
      case Status::kVersionNotSupported:
        return "HTTP Version Not Supported";
    }
    assert(false && "Invalid status");
    __builtin_unreachable();
  }

}  // namespace HTTP
