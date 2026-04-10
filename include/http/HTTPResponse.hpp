/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPResponse.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/14 10:48:28 by bewong        #+#    #+#                 */
/*   Updated: 2026/03/17 10:04:08 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <cstdint>
#include <string>

#include "HTTPMessage.hpp"
#include "HTTPStatus.hpp"

class HTTPResponse : public HTTPMessage
{
  public:
    HTTPResponse(void) = default;
    HTTPResponse(const HTTPResponse& other) = default;
    HTTPResponse(HTTPResponse&& other) noexcept = default;
    HTTPResponse& operator=(const HTTPResponse& other) = default;
    HTTPResponse& operator=(HTTPResponse&& other) noexcept = default;
    ~HTTPResponse(void) override = default;

    std::uint16_t GetStatusCode(void) const;
    const std::string& GetReason(void) const;

    void SetStatus(HTTP::Status status);

  private:
    HTTP::Status status_{HTTP::Status::kOk};
    std::string reason_{"kOk"};
};

#endif
