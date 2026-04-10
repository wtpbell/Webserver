/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ResponseFactory.hpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/03/17 10:04:14 by bewong        #+#    #+#                 */
/*   Updated: 2026/03/17 10:04:14 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSEFACTORY_HPP
#define RESPONSEFACTORY_HPP

#include <string>
#include <string_view>

#include "http/HTTPResponse.hpp"
#include "http/HTTPStatus.hpp"

inline constexpr std::string_view kDefaultCT = "text/plain; charset=utf-8";
inline constexpr std::string_view kDefaultHTMLCT = "text/html; charset=utf-8";
inline constexpr std::string_view kDefaultCTHeader = "Content-Type";

namespace HTTP
{
  namespace response
  {
    HTTPResponse MakeText(HTTP::Status st, std::string body, std::string_view ct = kDefaultCT);
    HTTPResponse MakeHTML(HTTP::Status st, std::string body);
    HTTPResponse MakeEmpty(HTTP::Status st);
    HTTPResponse MakeFile(HTTP::Status st, const std::string& path, std::string body);
    HTTPResponse MakeError(HTTP::Status st);
    HTTPResponse MakeRedirect(HTTP::Status st, std::string location);
    HTTPResponse MakeRedirectPermanent(std::string location);
  }  // namespace response

}  // namespace HTTP

#endif
