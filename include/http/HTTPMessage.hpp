/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPMessage.hpp                                   :+:    :+:            */
/*   HTTPMessager.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/02 15:29:51 by bewong        #+#    #+#                 */
/*   Updated: 2025/12/02 15:29:51 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPMESSAGE_HPP
#define HTTPMESSAGE_HPP

#include <optional>
#include <string>
#include <string_view>

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
    std::string_view GetHeader(std::string_view name) const;
    const std::string& GetBody(void) const;

    void SetVersion(std::string ver);
    void SetBody(const std::string& body);
    void SetHeader(std::string_view name, std::string_view value);

    void AddHeader(std::string_view name, std::string_view value);
    void AppendBody(std::string_view body);
    void RemoveHeader(std::string_view name);

    bool HasHeader(std::string_view name) const;
    std::optional<std::size_t> GetContentLength(void) const;
    bool IsChunked(void) const;

  protected:
    const HTTP::Headers& GetHeaders(void) const;
    HTTP::Headers& GetHeaders(void);

  private:
    std::string version_ = std::string{HTTP::kVERSION};
    HTTP::Headers headers_;
    std::string body_;
};

#endif  // HTTPMESSAGE_HPP
