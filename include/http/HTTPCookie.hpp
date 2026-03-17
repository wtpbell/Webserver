/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPCookie.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bewong <bewong@student.codam.nl>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 11:22:11 by bewong            #+#    #+#             */
/*   Updated: 2026/03/04 11:22:11 by bewong           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPCOOKIE_HPP
#define HTTPCOOKIE_HPP

#include <string>
#include <map>
#include "HTTPRequest.hpp"

namespace HTTP
{
  namespace cookie
  {
    bool ParseCookieHeader(std::string_view header, HTTPRequest::CookieMap* out = nullptr);
    bool AttachCookies(HTTPRequest& req);
  }  // namespace cookie
}  // namespace HTTP

#endif  // HTTPCOOKIE_HPP
