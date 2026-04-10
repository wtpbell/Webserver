/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPValidator.hpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/13 19:06:37 by bewong        #+#    #+#                 */
/*   Updated: 2026/02/06 17:19:59 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPVALIDATOR_HPP
#define HTTPVALIDATOR_HPP

#include <string_view>

#include "http/HTTPRequest.hpp"

enum class ValidationResult
{
  kOk,
  kBadRequest,
  kNotImplemented,
  kPayloadTooLarge,
  kVersionNotSupported,
  kURITooLong,
  kInternalServerError
};

namespace HTTP
{
  namespace validate
  {
    inline bool IsValidVersion(std::string_view version)
    {
      return version == kVERSION;
    }

    inline bool IsValidMethod(std::string_view method)
    {
      return HTTP::StringToMethod(method) != HTTP::Method::kUnsupported;
    }

    bool IsValidTarget(std::string_view method, std::string_view target);

    bool IsValidHeaderName(std::string_view name);

    bool IsValidPath(std::string_view target);

    bool IsValidHeaderValue(std::string_view value);

    bool IsValidCookieValue(std::string_view value);
    bool IsValidCookieHeader(std::string_view header);
    bool IsValidSid(std::string_view id);

  }  // namespace validate
}  // namespace HTTP

ValidationResult ValidateRequest(const HTTPRequest& request);
ValidationResult ValidateStartLine(const HTTPRequest& request);
ValidationResult ValidateHeader(const HTTPRequest& request);
ValidationResult ValidateCookies(const HTTPRequest& request);
ValidationResult ValidateTransferEncoding(const HTTPRequest& request);
ValidationResult ValidateContentLength(const HTTPRequest& request);

#endif  // HTTPVALIDATOR_HPP
