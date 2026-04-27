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
#include "http/HTTPTypes.hpp"
#include "http/HTTPUtils.hpp"
#include "http/ResponseFactory.hpp"

namespace HTTP::response
{
  namespace
  {
    HTTP::Headers MakeHeaders(std::string_view name, std::string_view value)
    {
      HTTP::Headers headers;
      headers.emplace(std::string{name}, std::vector<std::string>{std::string{value}});
      return headers;
    }

    HTTPResponse MakeTypedResponse(HTTP::Status st, std::string body, std::string_view contentType)
    {
      return HTTPResponse(st, MakeHeaders(kDefaultCTHeader, contentType.empty() ? kDefaultCT : contentType),
                          std::move(body));
    }
  }  // namespace

  HTTPResponse MakeText(HTTP::Status st, std::string body, std::string_view content_type)
  {
    return MakeTypedResponse(st, std::move(body), content_type);
  }

  HTTPResponse MakeHTML(HTTP::Status st, std::string body)
  {
    return MakeTypedResponse(st, std::move(body), kDefaultHTMLCT);
  }

  HTTPResponse MakeFile(HTTP::Status st, const std::string& filepath, std::string body)
  {
    HTTPResponse r = MakeTypedResponse(st, std::move(body), HTTP::wire::GetMimeType(filepath));

    const std::string last_modified = HTTP::wire::GetLastModifiedHttpDate(filepath);
    if (!last_modified.empty())
      r.SetHeader("Last-Modified", last_modified);

    return r;
  }

  HTTPResponse MakeEmpty(HTTP::Status st)
  {
    return HTTPResponse(st, "");
  }

  HTTPResponse MakeError(HTTP::Status st)
  {
    return HTTPResponse(st, MakeHeaders(kDefaultCTHeader, kDefaultCT),
                        std::to_string(static_cast<int>(st)) + " " + std::string(HTTP::ToReasonPhrase(st)) + "\n");
  }

  HTTPResponse MakeRedirect(HTTP::Status st, std::string location)
  {
    HTTPResponse r(st, std::string(HTTP::ToReasonPhrase(st)) + "\n");
    r.SetHeader(kDefaultCTHeader, kDefaultCT);
    r.SetHeader("Location", location);
    return r;
  }

  HTTPResponse MakeRedirectPermanent(std::string location)
  {
    return MakeRedirect(HTTP::Status::kMovedPermanently, std::move(location));
  }
}  // namespace HTTP::response
