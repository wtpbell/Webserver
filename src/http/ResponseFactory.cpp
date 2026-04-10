/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ResponseFactory.cpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/03/17 10:04:35 by bewong        #+#    #+#                 */
/*   Updated: 2026/03/17 10:04:35 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "http/ResponseFactory.hpp"

#include <string>

#include "http/HTTPStatus.hpp"
#include "http/HTTPUtils.hpp"

namespace HTTP::response
{
  HTTPResponse MakeText(HTTP::Status st, std::string body, std::string_view content_type)
  {
    HTTPResponse r;
    r.SetStatus(st);
    r.SetHeader(kDefaultCTHeader, content_type.empty() ? kDefaultCT : content_type);
    r.SetBody(std::move(body));
    return r;
  }

  HTTPResponse MakeHTML(HTTP::Status st, std::string body)
  {
    HTTPResponse r;
    r.SetStatus(st);
    r.SetHeader(kDefaultCTHeader, kDefaultHTMLCT);
    r.SetBody(std::move(body));
    return r;
  }

  HTTPResponse MakeFile(HTTP::Status st, const std::string& filepath, std::string body)
  {
    HTTPResponse r;
    r.SetStatus(st);
    r.SetHeader(kDefaultCTHeader, HTTP::wire::GetMimeType(filepath));
    r.SetBody(std::move(body));

    auto lm = HTTP::wire::GetLastModifiedHttpDate(filepath);
    if (!lm.empty())
      r.SetHeader("last-modified", lm);

    return r;
  }

  HTTPResponse MakeEmpty(HTTP::Status st)
  {
    HTTPResponse r;
    r.SetStatus(st);
    r.SetBody("");
    return r;
  }

  HTTPResponse MakeError(HTTP::Status st)
  {
    HTTPResponse r;
    r.SetStatus(st);
    r.SetHeader(kDefaultCTHeader, kDefaultCT);
    r.SetBody(std::to_string(static_cast<int>(st)) + " " + std::string(r.GetReason()) + "\n");
    return r;
  }

  HTTPResponse MakeRedirect(HTTP::Status st, std::string location)
  {
    HTTPResponse r;
    r.SetStatus(st);
    r.SetHeader("Location", std::move(location));
    r.SetHeader(kDefaultCTHeader, kDefaultCT);
    r.SetBody(std::string(r.GetReason()) + "\n");  // optional but helpful
    return r;
  }

  HTTPResponse MakeRedirectPermanent(std::string location)
  {
    return MakeRedirect(HTTP::Status::kMovedPermanently, std::move(location));
  }
}  // namespace HTTP::response
