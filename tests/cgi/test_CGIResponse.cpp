/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   test_CGIResponse.cpp                               :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/02/17 15:05:13 by jboon         #+#    #+#                 */
/*   Updated: 2026/02/17 15:05:14 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <string>

#include "catch_amalgamated.hpp"
#include "cgi/CGIResponse.hpp"

namespace
{
  using namespace cgi;
  using Catch::Matchers::ContainsSubstring;
  using Catch::Matchers::EndsWith;
  using Catch::Matchers::Matches;
  using Catch::Matchers::StartsWith;

  const std::string kRegexDateTime{
      "[\\s\\S]*(Date: (Mon|Tue|Wed|Thu|Fri|Sat|Sun), (0[1-9]|[12][0-9]|3[01]) "
      "(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) [0-9]{4} ([01][0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9]) "
      "GMT\\r\\n)[\\s\\S]*"};
  const std::string kMessageFormat{"^(.+\r\n)(.*:.*\r\n)+\r\n[\\w!,\\s]*$"};
}  // namespace

TEST_CASE("Pure header response", "[cgi][CGIResponse]")
{
  CGIResponse response;
  response.EmplaceHeader("Content-Type", "text/plain; charset=utf-8");
  response.EmplaceHeader("Server", "cgi-server");
  response.EmplaceHeader("Location", "https://google.com");
  response.EmplaceHeader("Random-Header", "Random-Value");
  std::string http_message{response.SerializeAsHttp()};

  REQUIRE_FALSE(http_message.empty());
  REQUIRE_THAT(http_message, Matches(kMessageFormat));
  REQUIRE_THAT(http_message, StartsWith("HTTP/1.1 200 OK\r\n"));
  REQUIRE_THAT(http_message, ContainsSubstring("Content-Type: text/plain; charset=utf-8\r\n"));
  REQUIRE_THAT(http_message, ContainsSubstring("Server: cgi-server\r\n"));
  REQUIRE_THAT(http_message, ContainsSubstring("Location: https://google.com\r\n"));
  REQUIRE_THAT(http_message, ContainsSubstring("Random-Header: Random-Value\r\n"));
  REQUIRE_THAT(http_message, Matches(kRegexDateTime));
  REQUIRE_THAT(http_message, EndsWith("\r\n"));
  REQUIRE_FALSE(response.IsLocalRedirect());
}

TEST_CASE("404 Serialized Response", "[cgi][CGIResponse]")
{
  CGIResponse response;
  response.SetStatus({404, "Page not found"});
  response.EmplaceHeader("Content-Type", "text/plain; charset=utf-8");
  response.EmplaceHeader("Server", "cgi-server");
  response.EmplaceHeader("Location", "https://google.com");
  response.EmplaceHeader("Random-Header", "Random-Value");
  std::string http_message{response.SerializeAsHttp()};

  REQUIRE_FALSE(http_message.empty());
  REQUIRE_THAT(http_message, Matches(kMessageFormat));
  REQUIRE_THAT(http_message, StartsWith("HTTP/1.1 404 Page not found\r\n"));
  REQUIRE_THAT(http_message, ContainsSubstring("Content-Type: text/plain; charset=utf-8\r\n"));
  REQUIRE_THAT(http_message, ContainsSubstring("Server: cgi-server\r\n"));
  REQUIRE_THAT(http_message, ContainsSubstring("Location: https://google.com\r\n"));
  REQUIRE_THAT(http_message, ContainsSubstring("Random-Header: Random-Value\r\n"));
  REQUIRE_THAT(http_message, Matches(kRegexDateTime));
  REQUIRE_THAT(http_message, EndsWith("\r\n"));
  REQUIRE_FALSE(response.IsLocalRedirect());
}

TEST_CASE("Local redirection", "[cgi][CGIResponse]")
{
  CGIResponse response;
  response.SetStatus({302, "Found"});
  response.EmplaceHeader("Location", "/go-here-instead");
  std::string http_message{response.SerializeAsHttp()};

  REQUIRE_FALSE(http_message.empty());
  REQUIRE_THAT(http_message, Matches(kMessageFormat));
  REQUIRE_THAT(http_message, StartsWith("HTTP/1.1 302 Found\r\n"));
  REQUIRE_THAT(http_message, ContainsSubstring("Server: webserve/0.1\r\n"));
  REQUIRE_THAT(http_message, ContainsSubstring("Location: /go-here-instead\r\n"));
  REQUIRE_THAT(http_message, Matches(kRegexDateTime));
  REQUIRE_THAT(http_message, EndsWith("\r\n"));

  REQUIRE(response.IsLocalRedirect());
  REQUIRE(response.LocalTarget() == "/go-here-instead");
}

TEST_CASE("Non local redirection", "[cgi][CGIResponse]")
{
  CGIResponse response;
  response.SetStatus({302, "Found"});
  response.EmplaceHeader("Location", "https://localhost/go-here-instead");
  std::string http_message{response.SerializeAsHttp()};

  REQUIRE_FALSE(http_message.empty());
  REQUIRE_THAT(http_message, Matches(kMessageFormat));
  REQUIRE_THAT(http_message, StartsWith("HTTP/1.1 302 Found\r\n"));
  REQUIRE_THAT(http_message, ContainsSubstring("Server: webserve/0.1\r\n"));
  REQUIRE_THAT(http_message, ContainsSubstring("Location: https://localhost/go-here-instead\r\n"));
  REQUIRE_THAT(http_message, Matches(kRegexDateTime));
  REQUIRE_THAT(http_message, EndsWith("\r\n"));

  REQUIRE_FALSE(response.IsLocalRedirect());
}

TEST_CASE("Set Body", "[cgi][CGIResponse]")
{
  CGIResponse response;
  response.EmplaceHeader("Content-Type", "text/plain; charset=utf-8");
  response.EmplaceHeader("Server", "cgi-server");
  response.EmplaceHeader("Location", "https://google.com");
  response.EmplaceHeader("Random-Header", "Random-Value");
  response.SetBody("Hello, world!");
  std::string http_message{response.SerializeAsHttp()};

  REQUIRE_FALSE(http_message.empty());
  REQUIRE_THAT(http_message, Matches(kMessageFormat));
  REQUIRE_THAT(http_message, StartsWith("HTTP/1.1 200 OK\r\n"));
  REQUIRE_THAT(http_message, ContainsSubstring("Content-Type: text/plain; charset=utf-8\r\n"));
  REQUIRE_THAT(http_message, ContainsSubstring("Server: cgi-server\r\n"));
  REQUIRE_THAT(http_message, ContainsSubstring("Location: https://google.com\r\n"));
  REQUIRE_THAT(http_message, ContainsSubstring("Random-Header: Random-Value\r\n"));
  REQUIRE_THAT(http_message, ContainsSubstring("Content-Length: 13\r\n"));
  REQUIRE_THAT(http_message, Matches(kRegexDateTime));
  REQUIRE_THAT(http_message, EndsWith("\r\nHello, world!"));
  REQUIRE_FALSE(response.IsLocalRedirect());
}

TEST_CASE("Empty serialization of response", "[cgi][CGIResponse]")
{
  CGIResponse response;
  std::string http_message{response.SerializeAsHttp()};

  REQUIRE_FALSE(http_message.empty());
  REQUIRE_THAT(http_message, Matches(kMessageFormat));
  REQUIRE_THAT(http_message, StartsWith("HTTP/1.1 200 OK\r\n"));
  REQUIRE_THAT(http_message, ContainsSubstring("Server: webserve/0.1\r\n"));
  REQUIRE_THAT(http_message, Matches(kRegexDateTime));
  REQUIRE_THAT(http_message, EndsWith("\r\n"));
  REQUIRE_FALSE(response.IsLocalRedirect());
}

TEST_CASE("reponse with cookies", "[cgi][CGIResponse]")
{
  CGIResponse response;
  response.EmplaceHeader("Content-Type", "text/plain; charset=utf-8");
  response.EmplaceHeader("Server", "cgi-server");
  response.EmplaceHeader("Location", "https://google.com");
  response.EmplaceHeader("Random-Header", "Random-Value");
  response.AddCookie("cookie1=value1");
  response.AddCookie("cookie2=value2");
  response.AddCookie("cookie3=value3");
  std::string http_message{response.SerializeAsHttp()};

  REQUIRE_FALSE(http_message.empty());
  REQUIRE_THAT(http_message, Matches(kMessageFormat));
  REQUIRE_THAT(http_message, StartsWith("HTTP/1.1 200 OK\r\n"));
  REQUIRE_THAT(http_message, ContainsSubstring("Content-Type: text/plain; charset=utf-8\r\n"));
  REQUIRE_THAT(http_message, ContainsSubstring("Server: cgi-server\r\n"));
  REQUIRE_THAT(http_message, ContainsSubstring("Location: https://google.com\r\n"));
  REQUIRE_THAT(http_message, ContainsSubstring("Random-Header: Random-Value\r\n"));
  REQUIRE_THAT(http_message, ContainsSubstring("Set-Cookie: cookie1=value1\r\n"));
  REQUIRE_THAT(http_message, ContainsSubstring("Set-Cookie: cookie2=value2\r\n"));
  REQUIRE_THAT(http_message, ContainsSubstring("Set-Cookie: cookie3=value3\r\n"));
  REQUIRE_THAT(http_message, Matches(kRegexDateTime));
  REQUIRE_THAT(http_message, EndsWith("\r\n"));
  REQUIRE_FALSE(response.IsLocalRedirect());
}
