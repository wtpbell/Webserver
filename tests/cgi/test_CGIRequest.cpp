/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   test_CGIRequest.cpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/02/17 15:05:10 by jboon         #+#    #+#                 */
/*   Updated: 2026/02/17 15:05:11 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "catch_amalgamated.hpp"
#include "cgi/CGI.hpp"
#include "cgi/CGIRequest.hpp"
#include "http/HTTPParser.hpp"
#include "http/HTTPRequest.hpp"

namespace
{
  using namespace cgi;
  namespace FileSystem = std::filesystem;

  std::string CGIRequestEnvpToString(const CGIRequest& request)
  {
    std::string buffer;
    buffer.reserve(1024);

    const auto& envp = request.GetEnvp();
    for (const auto& item : envp)
      buffer.append(item).append("\n");
    return buffer;
  }

  HTTPRequest CreateHTTPRequest(std::string_view message)
  {
    HTTPParser parser;

    parser.Parse(message);
    const HTTPRequest& request = parser.GetRequest();
    if (parser.NeedsBodyDecision())
    {
      if (request.HasHeader("transfer-encoding"))
        parser.SetChunked();
      else if (request.GetContentLength())
        parser.SetContentLength(request.GetContentLength().value());
      else
        parser.SetNoBody();
      parser.Parse("");
    }

    REQUIRE(parser.ExitResult() == HTTPParser::ParseResult::Done);
    return request;
  }

  Path GetRoot(void)
  {
    try
    {
      return {FileSystem::current_path() / "tests/cgi-bin"};
    }
    catch (...)
    {
    }
    FAIL("Failed to retrieve root directory");
  }

}  // namespace

TEST_CASE("GET CGI request", "[cgi][CGIRequest]")
{
  const FileSystem::path root{GetRoot()};
  const FileSystem::path script{"cgi-bin/hello_world.sh"};
  const FileSystem::path full_path{root / "hello_world.sh"};
  constexpr std::string_view server_info{"127.0.0.1:8080"};
  constexpr std::string_view client_info{"127.0.0.2:9000"};
  constexpr std::string_view client_message{
      "GET /cgi-bin/hello_world.sh HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "User-Agent: webserv/Catch2\r\n"
      "Connection: keep-alive\r\n"
      "Multi-Value-Header: value_a\r\n"
      "Multi-Value-Header: value_b\r\n"
      "Multi-Value-Header: value_c\r\n"
      "\r\n"};
  constexpr std::string_view cgi_request_sv{
      "GATEWAY_INTERFACE=CGI/1.1\n"
      "SCRIPT_NAME=hello_world.sh\n"
      "QUERY_STRING=\n"
      "REMOTE_ADDR=127.0.0.2\n"
      "REMOTE_HOST=127.0.0.2\n"
      "REQUEST_METHOD=GET\n"
      "SERVER_NAME=[127.0.0.1]\n"
      "SERVER_PORT=8080\n"
      "SERVER_PROTOCOL=HTTP/1.1\n"
      "SERVER_SOFTWARE=webserv/0.1\n"
      "HTTP_MULTI_VALUE_HEADER=value_a, value_b, value_c\n"
      "HTTP_USER_AGENT=webserv/Catch2\n"
      "HTTP_CONNECTION=keep-alive\n"};

  const HTTPRequest& http_request{CreateHTTPRequest(client_message)};
  auto cgi_route{cgi::SetupCGIRoute(http_request.GetPath(), root, "/cgi-bin")};

  CAPTURE(http_request.GetPath(), root, cgi_route.GetError());
  REQUIRE(cgi_route.HasValue());

  CGIRequest cgi_request{http_request, cgi_route.GetValue(), server_info, client_info};
  REQUIRE(cgi_request.GetArgv().size() == 1);
  REQUIRE(cgi_request.GetArgv().at(0) == full_path.string());
  REQUIRE(cgi_request.GetEnvp().size() == 13);
  REQUIRE(CGIRequestEnvpToString(cgi_request) == cgi_request_sv);
  REQUIRE(cgi_request.GetLeftover() == 0);
  REQUIRE(cgi_request.GetBody().empty());
}

TEST_CASE("POST CGI request", "[cgi][CGIRequest]")
{
  const FileSystem::path root{GetRoot()};
  const FileSystem::path script{"cgi-bin/hello_world.sh"};
  const FileSystem::path full_path{root / "hello_world.sh"};
  constexpr std::string_view server_info{"127.0.0.1:8080"};
  constexpr std::string_view client_info{"127.0.0.2:9000"};
  constexpr std::string_view client_message{
      "POST /cgi-bin/hello_world.sh HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "User-Agent: webserv/Catch2\r\n"
      "Connection: keep-alive\r\n"
      "Multi-Value-Header: value_a\r\n"
      "Multi-Value-Header: value_b\r\n"
      "Multi-Value-Header: value_c\r\n"
      "Content-Type: text/plain; charset=utf-8\r\n"
      "Transfer-Encoding: chunked\r\n"
      "\r\n"
      "d\r\n"
      "Hello, World!\r\n"
      "80\r\n"
      "Sed sit amet tortor ac augue ultrices mollis id quis nulla. Aliquam sit amet libero pellentesque, congue urna "
      "in, dapibus morbi.\r\n"
      "0\r\n"
      "\r\n"};
  constexpr std::string_view cgi_request_sv{
      "GATEWAY_INTERFACE=CGI/1.1\n"
      "SCRIPT_NAME=hello_world.sh\n"
      "QUERY_STRING=\n"
      "CONTENT_LENGTH=141\n"
      "CONTENT_TYPE=text/plain; charset=utf-8\n"
      "REMOTE_ADDR=127.0.0.2\n"
      "REMOTE_HOST=127.0.0.2\n"
      "REQUEST_METHOD=POST\n"
      "SERVER_NAME=[127.0.0.1]\n"
      "SERVER_PORT=8080\n"
      "SERVER_PROTOCOL=HTTP/1.1\n"
      "SERVER_SOFTWARE=webserv/0.1\n"
      "HTTP_TRANSFER_ENCODING=chunked\n"  // TODO: should this be filtered out?
      "HTTP_MULTI_VALUE_HEADER=value_a, value_b, value_c\n"
      "HTTP_USER_AGENT=webserv/Catch2\n"
      "HTTP_CONNECTION=keep-alive\n"};
  constexpr std::string_view cgi_body{
      "Hello, World!Sed sit amet tortor ac augue ultrices mollis id quis nulla. Aliquam sit amet libero "
      "pellentesque, congue urna "
      "in, dapibus morbi."};

  const HTTPRequest& http_request{CreateHTTPRequest(client_message)};
  auto cgi_route{cgi::SetupCGIRoute(http_request.GetPath(), root, "/cgi-bin")};
  REQUIRE(cgi_route.HasValue());

  CGIRequest cgi_request{http_request, cgi_route.GetValue(), server_info, client_info};
  REQUIRE(cgi_request.GetArgv().size() == 1);
  REQUIRE(cgi_request.GetArgv().at(0) == full_path.string());
  REQUIRE(cgi_request.GetEnvp().size() == 16);
  REQUIRE(CGIRequestEnvpToString(cgi_request) == cgi_request_sv);
  REQUIRE(cgi_request.GetLeftover() == 141);
  REQUIRE(cgi_request.GetBody().length() == 141);
  REQUIRE(cgi_request.GetBody() == cgi_body);
}

TEST_CASE("CGI Request with query string", "[cgi][CGIRequest]")
{
  const FileSystem::path root{GetRoot()};
  const FileSystem::path script{"cgi-bin/hello_world.sh"};
  const FileSystem::path full_path{root / "hello_world.sh"};
  constexpr std::string_view server_info{"127.0.0.1:8080"};
  constexpr std::string_view client_info{"127.0.0.2:9000"};
  constexpr std::string_view client_message{
      "GET /cgi-bin/hello_world.sh?query=this&key=value&text=bla+bla+bla_bla HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "User-Agent: webserv/Catch2\r\n"
      "Connection: keep-alive\r\n"
      "Multi-Value-Header: value_a\r\n"
      "Multi-Value-Header: value_b\r\n"
      "Multi-Value-Header: value_c\r\n"
      "\r\n"};
  constexpr std::string_view cgi_request_sv{
      "GATEWAY_INTERFACE=CGI/1.1\n"
      "SCRIPT_NAME=hello_world.sh\n"
      "QUERY_STRING=query=this&key=value&text=bla+bla+bla_bla\n"
      "REMOTE_ADDR=127.0.0.2\n"
      "REMOTE_HOST=127.0.0.2\n"
      "REQUEST_METHOD=GET\n"
      "SERVER_NAME=[127.0.0.1]\n"
      "SERVER_PORT=8080\n"
      "SERVER_PROTOCOL=HTTP/1.1\n"
      "SERVER_SOFTWARE=webserv/0.1\n"
      "HTTP_MULTI_VALUE_HEADER=value_a, value_b, value_c\n"
      "HTTP_USER_AGENT=webserv/Catch2\n"
      "HTTP_CONNECTION=keep-alive\n"};

  const HTTPRequest& http_request{CreateHTTPRequest(client_message)};
  auto cgi_route{cgi::SetupCGIRoute(http_request.GetPath(), root, "/cgi-bin")};
  REQUIRE(cgi_route.HasValue());

  CGIRequest cgi_request{http_request, cgi_route.GetValue(), server_info, client_info};
  REQUIRE(cgi_request.GetArgv().size() == 1);
  REQUIRE(cgi_request.GetArgv().at(0) == full_path.string());  // TODO: Argv[0] should be full path to script...
  REQUIRE(cgi_request.GetEnvp().size() == 13);
  REQUIRE(CGIRequestEnvpToString(cgi_request) == cgi_request_sv);
  REQUIRE(cgi_request.GetLeftover() == 0);
  REQUIRE(cgi_request.GetBody().empty());
}

TEST_CASE("CGI request with all components", "[cgi][CGIRequest]")
{
  const FileSystem::path root{GetRoot()};
  const FileSystem::path script{"cgi-bin/hello_world.sh"};
  const FileSystem::path full_path{root / "hello_world.sh"};
  constexpr std::string_view server_info{"127.0.0.1:8080"};
  constexpr std::string_view client_info{"127.0.0.2:9000"};
  constexpr std::string_view client_message{
      "GET /cgi-bin/hello_world.sh?query=this&key=value&text=bla+bla+bla_bla HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "User-Agent: webserv/Catch2\r\n"
      "Connection: keep-alive\r\n"
      "Multi-Value-Header: value_a\r\n"
      "Transfer-Encoding: chunked\r\n"
      "Multi-Value-Header: value_b\r\n"
      "Multi-Value-Header: value_c\r\n"
      "\r\n"
      "1\r\n"
      "S\r\n"
      "64\r\n"
      "ed varius mauris ante, eget congue diam tincidunt non. Duis et justo eros. Sed ac elit non ipsum lao\r\n"
      "5\r\n"
      "reet \r\n"
      "6e\r\n"
      "placerat. Duis mattis, metus a elementum convallis, dui justo ultrices mauris, vitae malesuada nulla nunc non "
      "\r\n"
      "28\r\n"
      "ante. Nam lobortis, risus non tincidunt.\r\n"
      "0\r\n"
      "\r\n"};

  constexpr std::string_view cgi_request_sv{
      "GATEWAY_INTERFACE=CGI/1.1\n"
      "SCRIPT_NAME=hello_world.sh\n"
      "QUERY_STRING=query=this&key=value&text=bla+bla+bla_bla\n"
      "CONTENT_LENGTH=256\n"
      "CONTENT_TYPE=application/octet-stream\n"
      "REMOTE_ADDR=127.0.0.2\n"
      "REMOTE_HOST=127.0.0.2\n"
      "REQUEST_METHOD=GET\n"
      "SERVER_NAME=[127.0.0.1]\n"
      "SERVER_PORT=8080\n"
      "SERVER_PROTOCOL=HTTP/1.1\n"
      "SERVER_SOFTWARE=webserv/0.1\n"
      "HTTP_TRANSFER_ENCODING=chunked\n"  // TODO: should this be filtered out?
      "HTTP_MULTI_VALUE_HEADER=value_a, value_b, value_c\n"
      "HTTP_USER_AGENT=webserv/Catch2\n"
      "HTTP_CONNECTION=keep-alive\n"};

  constexpr std::string_view cgi_body{
      "Sed varius mauris ante, eget congue diam tincidunt non. Duis et justo eros. Sed ac elit non ipsum laoreet "
      "placerat. Duis mattis, metus a elementum convallis, dui justo ultrices mauris, vitae malesuada nulla nunc non "
      "ante. Nam lobortis, risus non tincidunt."};

  const HTTPRequest& http_request{CreateHTTPRequest(client_message)};
  auto cgi_route{cgi::SetupCGIRoute(http_request.GetPath(), root, "/cgi-bin")};
  REQUIRE(cgi_route.HasValue());

  CGIRequest cgi_request{http_request, cgi_route.GetValue(), server_info, client_info};
  REQUIRE(cgi_request.GetArgv().size() == 1);
  REQUIRE(cgi_request.GetArgv().at(0) == full_path.string());
  REQUIRE(cgi_request.GetEnvp().size() == 16);
  REQUIRE(CGIRequestEnvpToString(cgi_request) == cgi_request_sv);
  REQUIRE(cgi_request.GetLeftover() == 256);
  REQUIRE(cgi_request.GetBody() == cgi_body);
}
