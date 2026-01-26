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
      default:
        return Status::INTERNAL_SERVER_ERROR;
    }
  }
  std::string_view ToReasonPhrase(Status status)
  {
    switch (status)
    {
      case Status::OK:
        return "OK";
      case Status::BAD_REQUEST:
        return "Bad Request";
      case Status::NOT_IMPLEMENTED:
        return "Not Implemented";
      case Status::PAYLOAD_TOO_LARGE:
        return "Payload Too Large";
      case Status::URI_TOO_LONG:
        return "URI Too Long";
      case Status::HTTP_VERSION_NOT_SUPPORTED:
        return "HTTP Version Not Supported";
      case Status::INTERNAL_SERVER_ERROR:
        return "Internal Server Error";
      default:
        return "Error";
    }
  }
}  // namespace HTTP
