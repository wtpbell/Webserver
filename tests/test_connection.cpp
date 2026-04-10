#include <iostream>
#include <sstream>
#include <string>

#include "Connection.hpp"
#include "catch_amalgamated.hpp"
#include "config/Builder.hpp"
#include "config/Lexer.hpp"
#include "config/Parser.hpp"
#include "config/ServerRegistry.hpp"
#include "config/Validator.hpp"
#include "config/ValidatorIpPort.hpp"
#include "http/SessionManager.hpp"
#include "io/Socket.hpp"
#include "router/Router.hpp"

namespace
{

  ServerRegistry BuildRegistry(const std::string& raw)
  {
    std::stringstream buffer;
    auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());

    Lexer lexer(raw);
    Parser parser(lexer);
    ValidatorIpPort validatorIpPort;
    Validator validator(lexer, parser, validatorIpPort);
    Builder builder(lexer, parser, validatorIpPort);
    ServerRegistry registry(builder.BuildServerRegistry());
    lexer.PrintErrorMessages();

    std::string output = buffer.str();
    std::cerr.rdbuf(oldBuf);

    REQUIRE(output.empty() == true);
    REQUIRE(lexer.GetError() == false);
    REQUIRE(parser.GetError() == false);
    REQUIRE(validator.GetError() == false);
    REQUIRE(builder.GetError() == false);

    return registry;
  }

  ServerRegistry BuildBasicRegistry()
  {
    return BuildRegistry(
        "http {\n"
        "  server {\n"
        "    listen 8080;\n"
        "    server_name localhost;\n"
        "    location / {\n"
        "      root ./tests/test_files;\n"
        "    }\n"
        "    location /upload {\n"
        "      root ./tests/test_files;\n"
        "      allowed_methods POST;\n"
        "      client_max_body_size 8;\n"
        "    }\n"
        "  }\n"
        "}\n");
  }

  Connection MakeConnection()
  {
    Socket sock;
    return Connection(std::move(sock));
  }

  Connection::TestIpPort MakeIpPort()
  {
    Connection::TestIpPort ipport;
    ipport.ip = "::";
    ipport.port = "8080";
    return ipport;
  }

  std::string MakeGet(const std::string& target, const std::string& host = "localhost")
  {
    return "GET " + target +
           " HTTP/1.1\r\n"
           "Host: " +
           host +
           "\r\n"
           "\r\n";
  }

  std::string MakePost(const std::string& target, const std::string& body, const std::string& host = "localhost")
  {
    return "POST " + target +
           " HTTP/1.1\r\n"
           "Host: " +
           host +
           "\r\n"
           "Content-Length: " +
           std::to_string(body.size()) +
           "\r\n"
           "\r\n" +
           body;
  }

  std::string MakeBadRequest()
  {
    return "GARBAGE\r\n\r\n";
  }

}  // namespace

TEST_CASE("Connection - parse error queues error response", "[connection][error]")
{
  ServerRegistry registry = BuildBasicRegistry();
  Router router;
  SessionManager sessionManager;
  Connection conn = MakeConnection();
  conn.TestSetIpPort(MakeIpPort());

  Connection::State st = conn.TestProcessInput(MakeBadRequest(), router, registry, sessionManager);

  REQUIRE(st == Connection::State::kKeepAlive);
  REQUIRE_FALSE(conn.TestGetOutputQueue().empty());
  REQUIRE(conn.TestGetCloseAfterSend() == true);
}

TEST_CASE("Connection - pipelines two requests from one input buffer", "[connection][pipeline]")
{
  ServerRegistry registry = BuildBasicRegistry();
  Router router;
  SessionManager sessionManager;
  Connection conn = MakeConnection();
  conn.TestSetIpPort(MakeIpPort());

  const std::string input = MakeGet("/") + MakeGet("/");

  Connection::State st = conn.TestProcessInput(input, router, registry, sessionManager);

  REQUIRE(st == Connection::State::kKeepAlive);
  REQUIRE(conn.TestGetOutputQueue().size() == 2);
  REQUIRE(conn.TestGetOutputQueue().front().find("HTTP/1.1") != std::string::npos);

  std::deque<std::string>::const_iterator it = conn.TestGetOutputQueue().begin();
  ++it;
  REQUIRE(it != conn.TestGetOutputQueue().end());
  REQUIRE(it->rfind("\r\nHTTP/1.1", 0) == 0);
}

TEST_CASE("Connection - request Connection close is respected", "[connection][close]")
{
  ServerRegistry registry = BuildBasicRegistry();
  Router router;
  SessionManager sessionManager;
  Connection conn = MakeConnection();
  conn.TestSetIpPort(MakeIpPort());

  const std::string input =
      "GET / HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "Connection: close\r\n"
      "\r\n";

  Connection::State st = conn.TestProcessInput(input, router, registry, sessionManager);

  REQUIRE(st == Connection::State::kKeepAlive);
  REQUIRE(conn.TestGetOutputQueue().size() == 1);
  REQUIRE(conn.TestGetCloseAfterSend() == true);
  REQUIRE(conn.TestGetOutputQueue().front().find("Connection: close") != std::string::npos);
}

TEST_CASE("Connection - matched route body limit is applied", "[connection][body-limit]")
{
  ServerRegistry registry = BuildBasicRegistry();
  Router router;
  SessionManager sessionManager;
  Connection conn = MakeConnection();
  conn.TestSetIpPort(MakeIpPort());

  const std::string input = MakePost("/upload/file.txt", "hello world");

  Connection::State st = conn.TestProcessInput(input, router, registry, sessionManager);

  REQUIRE(st == Connection::State::kKeepAlive);
  REQUIRE(conn.TestGetOutputQueue().size() == 1);
  REQUIRE(conn.TestGetCloseAfterSend() == true);
  REQUIRE(conn.TestGetOutputQueue().front().find("413") != std::string::npos);
}
