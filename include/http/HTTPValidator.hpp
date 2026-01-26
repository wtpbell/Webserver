/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPValidator.hpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/13 19:06:37 by bewong        #+#    #+#                 */
/*   Updated: 2026/01/13 19:06:37 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPVALIDATOR_HPP
#define HTTPVALIDATOR_HPP

#include "http/HTTPRequest.hpp"

enum class ValidationResult
{
  OK,
  BadRequest,
  NotImplemented,
  PayloadTooLarge,
  VersionNotSupported,
  URITooLong,
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
      return HTTP::StringToMethod(method) != HTTP::Method::UNSUPPORTED;
    }

    bool IsValidTarget(std::string_view method, std::string_view target);

    bool IsValidHeaderName(std::string_view name);

    bool IsValidPath(std::string_view target);

    bool IsValidHeaderValue(std::string_view value);

  }  // namespace validate
}  // namespace HTTP

ValidationResult ValidateRequest(const HTTPRequest& req);
ValidationResult ValidateStartLine(const HTTPRequest& req);
ValidationResult ValidateHeader(const HTTPRequest& req);
ValidationResult ValidateTransferEncoding(const HTTPRequest& req);
ValidationResult ValidateContentLength(const HTTPRequest& req);

#endif  // HTTPVALIDATOR_HPP
