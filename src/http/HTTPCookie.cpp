/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPCookie.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bewong <bewong@student.codam.nl>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 11:22:58 by bewong            #+#    #+#             */
/*   Updated: 2026/03/04 11:22:58 by bewong           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "http/HTTPCookie.hpp"

#include "http/HTTPValidator.hpp"
#include "string.hpp"

namespace HTTP
{
  namespace cookie
  {
    bool ParseCookieHeader(std::string_view header, HTTPRequest::CookieMap* out)
    {
      std::size_t pos = 0;
      while (pos < header.size())
      {
        std::size_t semi = header.find(';', pos);
        if (semi == std::string::npos)
          semi = header.size();
        std::string_view seg = String::Trim(header.substr(pos, semi - pos));
        pos = (semi == header.size()) ? semi : semi + 1;
        if (seg.empty())
          continue;
        std::size_t eq = seg.find('=');
        if (eq == std::string_view::npos)
          return false;
        std::string_view name = String::Trim(seg.substr(0, eq));
        std::string_view value = String::Trim(seg.substr(eq + 1));
        if (!HTTP::validate::IsValidHeaderName(name) || !HTTP::validate::IsValidCookieValue(value))
          return false;
        if (out)
          (*out)[std::string(name)] = std::string(value);
      }
      return true;
    }

    bool AttachCookies(HTTPRequest& req)
    {
      const auto* vals = req.GetHeaderValuesOf("cookie");
      if (!vals || vals->empty())
        return true;
      if (vals->size() != 1)
        return false;

      HTTPRequest::CookieMap cookies;
      if (!HTTP::cookie::ParseCookieHeader((*vals)[0], &cookies))
        return false;

      req.SetCookies(std::move(cookies));
      return true;
    }
  }  // namespace cookie
}  // namespace HTTP
