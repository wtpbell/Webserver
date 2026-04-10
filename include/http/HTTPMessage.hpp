/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPMessage.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/15 16:20:44 by bewong        #+#    #+#                 */
/*   Updated: 2026/04/02 11:17:26 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPMESSAGE_HPP
#define HTTPMESSAGE_HPP

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "http/HTTPTypes.hpp"

class HTTPMessage
{
  public:
    HTTPMessage(void) = default;
    HTTPMessage(const HTTPMessage&) = default;
    HTTPMessage& operator=(const HTTPMessage&) = default;
    HTTPMessage(HTTPMessage&&) noexcept = default;
    HTTPMessage& operator=(HTTPMessage&&) noexcept = default;
    virtual ~HTTPMessage(void) = default;

    HTTPMessage(std::string ver, HTTP::Headers headers, std::string body);

    std::string_view GetVersion(void) const;
    const std::vector<std::string>* GetHeaderValuesOf(std::string_view name) const;
    std::string_view GetFirstHeaderValueOf(std::string_view name) const;
    const std::string& GetBody(void) const;

    void SetVersion(std::string ver);
    void SetBody(std::string&& body);
    void SetHeader(std::string_view name, std::string_view value);

    void AddHeader(std::string_view name, std::string_view value);
    void AppendBody(std::string_view body);
    void RemoveHeader(std::string_view name);

    std::size_t GetHeaderValueCountOf(std::string_view name) const;
    bool HasHeader(std::string_view name) const;
    std::optional<std::size_t> GetContentLength(void) const;
    bool IsChunked(void) const;
    const HTTP::Headers& GetHeaders(void) const;
    HTTP::Headers& GetHeaders(void);

  private:
    std::string version_ = std::string{HTTP::kVERSION};
    HTTP::Headers headers_;
    std::string body_;
};

#endif  // HTTPMESSAGE_HPP
