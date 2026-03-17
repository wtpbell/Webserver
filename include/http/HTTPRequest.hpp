/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bewong <bewong@student.codam.nl>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/12 12:12:54 by bewong            #+#    #+#             */
/*   Updated: 2026/02/24 11:25:04 by bewong           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <map>
#include <string>
#include <string_view>

#include "http/HTTPMessage.hpp"
#include "http/HTTPTypes.hpp"

class HTTPRequest : public HTTPMessage
{
  public:
    using CookieMap = std::map<std::string, std::string, std::less<>>;

    HTTPRequest(void) = default;
    HTTPRequest(const HTTPRequest&) = default;
    HTTPRequest& operator=(const HTTPRequest&) = default;
    HTTPRequest(HTTPRequest&&) noexcept = default;
    HTTPRequest& operator=(HTTPRequest&&) noexcept = default;
    ~HTTPRequest(void) override = default;

    HTTP::Method GetMethod(void) const;
    std::string_view GetMethodString(void) const;

    std::string_view GetTarget(void) const;  // "/path/file?x=1"

    std::string_view GetRawPath(void) const;  // "/path/file" (encoded)
    std::string_view GetQuery(void) const;    // "x=1&y=2"

    std::string_view GetPath(void) const;  // decoded + normalized
    std::string_view GetHost(void) const;

    const CookieMap& GetCookies(void) const;
    bool HasCookie(std::string_view name) const;
    std::string_view GetCookieOr(std::string_view name, std::string_view def) const;

    void SetCookies(CookieMap&& cookies);
    void SetMethod(HTTP::Method method);
    bool SetTarget(std::string_view target);
    void SetMethod(std::string_view method);

    bool IsComplete(void) const;
    void SetComplete(bool complete);
    void Clear(void);

  private:
    bool NormalizePath(std::string_view in, std::string& out);
    bool ValidateRequest(const HTTPRequest& req);

    HTTP::Method method_ = HTTP::Method::UNSUPPORTED;
    std::string target_;  // "/path/file?x=1&y=2"
    std::string uri_;     // "/path/file"
    std::string query_;   // "x=1&y=2"
    bool isComplete_ = false;
    std::string path_;
    CookieMap cookies_;
};

#endif  // HTTPREQUEST_HPP
