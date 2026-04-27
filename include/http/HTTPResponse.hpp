/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPResponse.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/14 10:48:28 by bewong        #+#    #+#                 */
/*   Updated: 2026/04/24 16:35:43 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <cstdint>
#include <string>

#include "HTTPMessage.hpp"
#include "HTTPStatus.hpp"
#include "HTTPTypes.hpp"

class HTTPResponse : public HTTPMessage
{
  public:
    HTTPResponse() = default;
    HTTPResponse(const HTTPResponse& other) = default;
    HTTPResponse(HTTPResponse&& other) noexcept = default;
    HTTPResponse& operator=(const HTTPResponse& other) = default;
    HTTPResponse& operator=(HTTPResponse&& other) noexcept = default;
    ~HTTPResponse() override = default;

    HTTPResponse(HTTP::Status status, std::string body);
    HTTPResponse(HTTP::Status status, HTTP::Headers headers, std::string body);

    std::uint16_t GetStatusCode() const;
    const std::string& GetReason() const;

    void SetStatus(HTTP::Status status);

  private:
    HTTP::Status status_{HTTP::Status::kOk};
    std::string reason_{"Ok"};
};

#endif
