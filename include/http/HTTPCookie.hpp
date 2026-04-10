/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPCookie.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/04/02 09:56:23 by bewong        #+#    #+#                 */
/*   Updated: 2026/04/02 09:56:23 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPCOOKIE_HPP
#define HTTPCOOKIE_HPP

#include <string_view>

#include "HTTPRequest.hpp"

namespace HTTP
{
  namespace cookie
  {
    bool ParseCookieHeader(std::string_view header, HTTPRequest::CookieMap* out = nullptr);
    bool AttachCookies(HTTPRequest& request);
  }  // namespace cookie
}  // namespace HTTP

#endif  // HTTPCOOKIE_HPP
