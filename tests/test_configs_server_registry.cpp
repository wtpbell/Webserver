#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

#include "catch_amalgamated.hpp"
#include "config/Lexer.hpp"
#include "config/Parser.hpp"
#include "config/Validator.hpp"
#include "config/ValidatorIpPort.hpp"
#include "config/Builder.hpp"
#include "config/ServerRegistry.hpp"

// PUBLIC FUNCTIONS

TEST_CASE("GetServerViewCount()", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name example0.com;\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name example1.com;\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8082;\n"
                    "    server_name example2.com;\n"
                    "\n"
                    "  }\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(serverRegistry.GetServerViewCount() == 3);
}

TEST_CASE("GetServerCount()", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name example0.com;\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name example1.com;\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name example2.com;\n"
                    "\n"
                    "  }\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(serverRegistry.GetServerCount() == 1);
}

TEST_CASE("GetServerView()", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name example0.com;\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name example1.com;\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8082;\n"
                    "    server_name example2.com;\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name example3.com;\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name example4.com;\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8082;\n"
                    "    server_name example5.com;\n"
                    "\n"
                    "  }\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(serverRegistry.GetServerViewCount() == 6);

  const ServerView& serverView0(serverRegistry.GetServerView(0));
  const ServerView& serverView1(serverRegistry.GetServerView(1));
  const ServerView& serverView2(serverRegistry.GetServerView(2));
  const ServerView& serverView3(serverRegistry.GetServerView(3));
  const ServerView& serverView4(serverRegistry.GetServerView(4));
  const ServerView& serverView5(serverRegistry.GetServerView(5));

  REQUIRE(serverView0.hostNames.size() == 1);
  REQUIRE(serverView0.hostNames.at(0) == "example0.com");
  REQUIRE(serverView0.ipPort.ip == "::");
  REQUIRE(serverView0.ipPort.port == "8080");

  REQUIRE(serverView1.hostNames.size() == 1);
  REQUIRE(serverView1.hostNames.at(0) == "example1.com");
  REQUIRE(serverView1.ipPort.ip == "::");
  REQUIRE(serverView1.ipPort.port == "8081");
  
  REQUIRE(serverView2.hostNames.size() == 1);
  REQUIRE(serverView2.hostNames.at(0) == "example2.com");
  REQUIRE(serverView2.ipPort.ip == "::");
  REQUIRE(serverView2.ipPort.port == "8082");
  
  REQUIRE(serverView3.hostNames.size() == 1);
  REQUIRE(serverView3.hostNames.at(0) == "example3.com");
  REQUIRE(serverView3.ipPort.ip == "::");
  REQUIRE(serverView3.ipPort.port == "8080");
  
  REQUIRE(serverView4.hostNames.size() == 1);
  REQUIRE(serverView4.hostNames.at(0) == "example4.com");
  REQUIRE(serverView4.ipPort.ip == "::");
  REQUIRE(serverView4.ipPort.port == "8081");
  
  REQUIRE(serverView5.hostNames.size() == 1);
  REQUIRE(serverView5.hostNames.at(0) == "example5.com");
  REQUIRE(serverView5.ipPort.ip == "::");
  REQUIRE(serverView5.ipPort.port == "8082");
}

TEST_CASE("GetServerViewMap()", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name example0.com;\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name example1.com;\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8082;\n"
                    "    server_name example2.com;\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name example3.com;\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name example4.com;\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8082;\n"
                    "    server_name example5.com;\n"
                    "\n"
                    "  }\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(serverRegistry.GetServerViewCount() == 6);
  REQUIRE(serverRegistry.GetServerCount() == 3);

  std::size_t i = 0;
  for (const auto& serverData : serverRegistry.GetServerViewMap())
  {
    if (i == 0)
    {
      REQUIRE(serverData.first.ip == "::");
      REQUIRE(serverData.first.port == "8080");
      REQUIRE(serverData.second.size() == 2);
      REQUIRE(serverData.second[0]->hostNames.size() == 1);
      REQUIRE(serverData.second[0]->hostNames[0] == "example0.com");
      REQUIRE(serverData.second[1]->hostNames.size() == 1);
      REQUIRE(serverData.second[1]->hostNames[0] == "example3.com");
      REQUIRE(serverData.second[0]->ipPort.ip == "::");
      REQUIRE(serverData.second[0]->ipPort.port == "8080");
      REQUIRE(serverData.second[1]->ipPort.ip == "::");
      REQUIRE(serverData.second[1]->ipPort.port == "8080");
    }
    else if (i == 1)
    {
      REQUIRE(serverData.first.ip == "::");
      REQUIRE(serverData.first.port == "8081");
      REQUIRE(serverData.second.size() == 2);
      REQUIRE(serverData.second[0]->hostNames.size() == 1);
      REQUIRE(serverData.second[0]->hostNames[0] == "example1.com");
      REQUIRE(serverData.second[1]->hostNames.size() == 1);
      REQUIRE(serverData.second[1]->hostNames[0] == "example4.com");
      REQUIRE(serverData.second[0]->ipPort.ip == "::");
      REQUIRE(serverData.second[0]->ipPort.port == "8081");
      REQUIRE(serverData.second[1]->ipPort.ip == "::");
      REQUIRE(serverData.second[1]->ipPort.port == "8081");
    }
    else if (i == 2)
    {
      REQUIRE(serverData.first.ip == "::");
      REQUIRE(serverData.first.port == "8082");
      REQUIRE(serverData.second.size() == 2);
      REQUIRE(serverData.second[0]->hostNames.size() == 1);
      REQUIRE(serverData.second[0]->hostNames[0] == "example2.com");
      REQUIRE(serverData.second[1]->hostNames.size() == 1);
      REQUIRE(serverData.second[1]->hostNames[0] == "example5.com");
      REQUIRE(serverData.second[0]->ipPort.ip == "::");
      REQUIRE(serverData.second[0]->ipPort.port == "8082");
      REQUIRE(serverData.second[1]->ipPort.ip == "::");
      REQUIRE(serverData.second[1]->ipPort.port == "8082");
    }
    ++i;
  }
}

TEST_CASE("GetRouteView()", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name example0.com ex0.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location /test {\n"
                    "      root /test;\n"
                    "\n"
                    "    }\n"
                    "    location /test/ {\n"
                    "      root /test/;\n"
                    "    }\n"
                    "\n"
                    "    location /teeest/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /tessst/test/test {\n"
                    "\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    listen 127.0.0.35:8084;\n"
                    "    server_name example1.com ex1.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "    location t {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1 {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /teeest1/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /tessst1/test/test {\n"
                    "\n"
                    "    }\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    location /test2{\n"
                    "    }\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8082;\n"
                    "    server_name example2.com;\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8083;\n"
                    "    location /test3{\n"
                    "    }\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8084;\n"
                    "    server_name example2.com;\n"
                    "    location /not-default {\n"
                    "    }\n"
                    "  }\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(serverRegistry.GetServerViewCount() == 6);

  // at least one parameter does not match -> nullptr
  // ip address no match
  const RouteView* routeView = serverRegistry.GetRouteView("::abc", "8080", "ex0.com", "/test");
  REQUIRE(routeView == nullptr);

  // port num no match
  routeView = serverRegistry.GetRouteView("::", "8099", "ex0.com", "/test");
  REQUIRE(routeView == nullptr);

  // hostname no match and target path no match in default server
  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "/noMatch");
  REQUIRE(routeView == nullptr);

  // target path no match
  routeView = serverRegistry.GetRouteView("::", "8080", "ex0.com", "/noMatch");
  REQUIRE(routeView == nullptr);

  // target path is not path segment of location prefix
  routeView = serverRegistry.GetRouteView("::", "8080", "ex0.com", "/testtest");
  REQUIRE(routeView == nullptr);

  // target path empty
  routeView = serverRegistry.GetRouteView("::", "8080", "example.com", "");
  REQUIRE(routeView == nullptr);

  // target path no match with default location prefix
  routeView = serverRegistry.GetRouteView("::", "8082", "example2.com", "noMatch");
  REQUIRE(routeView == nullptr);


  // valid
  // location parameter and request URI identical: valid
  routeView = serverRegistry.GetRouteView("::", "8080", "ex0.com", "/tessst/test/test");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->locationPrefix == "/tessst/test/test");

  // location parameter and request URI identical, both ending in '/': valid
  routeView = serverRegistry.GetRouteView("::", "8080", "ex0.com", "/test/");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->locationPrefix == "/test/");
  REQUIRE(routeView->root == "/test/");

  // location parameter and request URI identical, neither ending in '/': valid
  routeView = serverRegistry.GetRouteView("::", "8080","ex0.com", "/test");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->locationPrefix == "/test");
  REQUIRE(routeView->root == "/test");

  // location parameter is prefix of request URI, location parameter ending in '/': valid
  routeView = serverRegistry.GetRouteView("::", "8080", "ex0.com", "/test/test");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->locationPrefix == "/test/");

  // location parameter is prefix and path segment of request URI: valid
  routeView = serverRegistry.GetRouteView("127.0.0.35", "8084", "example1.com", "/tessst1/test/test/test/test");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->locationPrefix == "/tessst1/test/test");

  // minimal match location parameter and request URI (location param is one char): '/': valid
  routeView = serverRegistry.GetRouteView("127.0.0.35", "8084", "ex1.com", "/abc");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->locationPrefix == "/");

  // minimal match location parameter and request URI (both one char): '/': valid
  routeView = serverRegistry.GetRouteView("127.0.0.35", "8084", "ex1.com", "/");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->locationPrefix == "/");

  // minimal match location parameter and request URI (one char): 't': valid
  routeView = serverRegistry.GetRouteView("127.0.0.35", "8084", "ex1.com", "t/test");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->locationPrefix == "t");

  // minimal match location parameter and request URI (both one char): 't': valid
  routeView = serverRegistry.GetRouteView("127.0.0.35", "8084", "ex1.com", "t");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->locationPrefix == "t");

  // anonymous server, match in first anonymous server: valid
  routeView = serverRegistry.GetRouteView("::", "8081", "", "/test2");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->locationPrefix == "/test2");

  // anonymous server, match in second anonymous server: valid
  routeView = serverRegistry.GetRouteView("::", "8083", "", "/test3");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->locationPrefix == "/test3");

  // no location block in server, target == default location prefix
  routeView = serverRegistry.GetRouteView("::", "8082", "example2.com", "/");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->locationPrefix == "/");

  // two homonymous servers, match in second server: valid
  routeView = serverRegistry.GetRouteView("::", "8084", "example2.com", "/not-default");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->locationPrefix == "/not-default");
}

// Bell's fantastic test case

TEST_CASE("GetRouteView() keeps listener scope", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "  server {\n"
                    "    listen 127.0.0.1:8080;\n"
                    "    server_name example.com;\n"
                    "    location /api {\n"
                    "      root ./www_a;\n"
                    "    }\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 127.0.0.1:9090;\n"
                    "    server_name example.com;\n"
                    "    location /api {\n"
                    "      root ./www_b;\n"
                    "    }\n"
                    "  }\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  const RouteView* route8080 = serverRegistry.GetRouteView("127.0.0.1", "8080", "example.com", "/api/users");
  REQUIRE(route8080 != nullptr);
  REQUIRE(route8080->locationPrefix == "/api");
  REQUIRE(route8080->root == std::filesystem::path("./www_a"));

  const RouteView* route9090 = serverRegistry.GetRouteView("127.0.0.1", "9090", "example.com", "/api/users");
  REQUIRE(route9090 != nullptr);
  REQUIRE(route9090->locationPrefix == "/api");
  REQUIRE(route9090->root == std::filesystem::path("./www_b"));
}

TEST_CASE("GetRouteView() implicit default server", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080 default_server;\n"
                    "    server_name server0;\n"
                    "    location /server0path0 {\n"
                    "      root server0root0;\n"
                    "    }\n"
                    "    location /server0path1 {\n"
                    "      root server0root1;\n"
                    "    }\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name server1;\n"
                    "    location /server1path0 {\n"
                    "      root server1root0;\n"
                    "    }\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name server2;\n"
                    "      location /server2path0{\n"
                    "        root server2root0;\n"
                    "      }\n"
                    "      location /server2path1{\n"
                    "        root server2root1;\n"
                    "      }\n"
                    "      location /server2path2{\n"
                    "        root server2root2;\n"
                    "      }\n"
                    "\n"
                    "  }\n"
                    "}\n";

  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  
  REQUIRE(serverRegistry.GetServerViewCount() == 3);
  REQUIRE(serverRegistry.GetServerCount() == 1);

  // name matches
  const RouteView* routeView = serverRegistry.GetRouteView("::", "8080", "server0", "/server0path0");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server0root0");

  routeView = serverRegistry.GetRouteView("::", "8080", "server0", "/server0path1");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server0root1");

  routeView = serverRegistry.GetRouteView("::", "8080", "server1", "/server1path0");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server1root0");

  routeView = serverRegistry.GetRouteView("::", "8080", "server2", "/server2path0");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server2root0");

  routeView = serverRegistry.GetRouteView("::", "8080", "server2", "/server2path1");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server2root1");

  routeView = serverRegistry.GetRouteView("::", "8080", "server2", "/server2path2");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server2root2");

  routeView = serverRegistry.GetRouteView("::", "8080", "server2", "noMatch");
  REQUIRE(routeView == nullptr);


  // name does not match
  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "/server0path0");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server0root0");

  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "/server0path1");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server0root1");

  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "/server1path0");
  REQUIRE(routeView == nullptr);

  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "/server2path0");
  REQUIRE(routeView == nullptr);

  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "/server2path1");
  REQUIRE(routeView == nullptr);

  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "/server2path2");
  REQUIRE(routeView == nullptr);

  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "noMatch");
  REQUIRE(routeView == nullptr);
}

TEST_CASE("GetRouteView() explicit default server 1", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080 default_server;\n"
                    "    server_name server0;\n"
                    "    location /server0path0 {\n"
                    "      root server0root0;\n"
                    "    }\n"
                    "    location /server0path1 {\n"
                    "      root server0root1;\n"
                    "    }\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name server1;\n"
                    "    location /server1path0 {\n"
                    "      root server1root0;\n"
                    "    }\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name server2;\n"
                    "      location /server2path0{\n"
                    "        root server2root0;\n"
                    "      }\n"
                    "      location /server2path1{\n"
                    "        root server2root1;\n"
                    "      }\n"
                    "      location /server2path2{\n"
                    "        root server2root2;\n"
                    "      }\n"
                    "\n"
                    "  }\n"
                    "}\n";

  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  
  REQUIRE(serverRegistry.GetServerViewCount() == 3);
  REQUIRE(serverRegistry.GetServerCount() == 1);

  // name matches
  const RouteView* routeView = serverRegistry.GetRouteView("::", "8080", "server0", "/server0path0");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server0root0");

  routeView = serverRegistry.GetRouteView("::", "8080", "server0", "/server0path1");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server0root1");

  routeView = serverRegistry.GetRouteView("::", "8080", "server1", "/server1path0");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server1root0");

  routeView = serverRegistry.GetRouteView("::", "8080", "server2", "/server2path0");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server2root0");

  routeView = serverRegistry.GetRouteView("::", "8080", "server2", "/server2path1");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server2root1");

  routeView = serverRegistry.GetRouteView("::", "8080", "server2", "/server2path2");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server2root2");

  routeView = serverRegistry.GetRouteView("::", "8080", "server2", "noMatch");
  REQUIRE(routeView == nullptr);


  // name does not match
  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "/server0path0");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server0root0");

  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "/server0path1");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server0root1");

  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "/server1path0");
  REQUIRE(routeView == nullptr);

  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "/server2path0");
  REQUIRE(routeView == nullptr);

  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "/server2path1");
  REQUIRE(routeView == nullptr);

  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "/server2path2");
  REQUIRE(routeView == nullptr);

  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "noMatch");
  REQUIRE(routeView == nullptr);
}

TEST_CASE("GetRouteView() explicit default server 2", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name server0;\n"
                    "    location /server0path0 {\n"
                    "      root server0root0;\n"
                    "    }\n"
                    "    location /server0path1 {\n"
                    "      root server0root1;\n"
                    "    }\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8080 default_server;\n"
                    "    server_name server1;\n"
                    "    location /server1path0 {\n"
                    "      root server1root0;\n"
                    "    }\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name server2;\n"
                    "      location /server2path0{\n"
                    "        root server2root0;\n"
                    "      }\n"
                    "      location /server2path1{\n"
                    "        root server2root1;\n"
                    "      }\n"
                    "      location /server2path2{\n"
                    "        root server2root2;\n"
                    "      }\n"
                    "\n"
                    "  }\n"
                    "}\n";

  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  
  REQUIRE(serverRegistry.GetServerViewCount() == 3);
  REQUIRE(serverRegistry.GetServerCount() == 1);

  // name matches
  const RouteView* routeView = serverRegistry.GetRouteView("::", "8080", "server0", "/server0path0");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server0root0");

  routeView = serverRegistry.GetRouteView("::", "8080", "server0", "/server0path1");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server0root1");

  routeView = serverRegistry.GetRouteView("::", "8080", "server1", "/server1path0");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server1root0");

  routeView = serverRegistry.GetRouteView("::", "8080", "server2", "/server2path0");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server2root0");

  routeView = serverRegistry.GetRouteView("::", "8080", "server2", "/server2path1");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server2root1");

  routeView = serverRegistry.GetRouteView("::", "8080", "server2", "/server2path2");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server2root2");

  routeView = serverRegistry.GetRouteView("::", "8080", "server2", "noMatch");
  REQUIRE(routeView == nullptr);


  // name does not match
  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "/server0path0");
  REQUIRE(routeView == nullptr);

  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "/server0path1");
  REQUIRE(routeView == nullptr);

  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "/server1path0");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server1root0");

  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "/server2path0");
  REQUIRE(routeView == nullptr);

  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "/server2path1");
  REQUIRE(routeView == nullptr);

  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "/server2path2");
  REQUIRE(routeView == nullptr);

  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "noMatch");
  REQUIRE(routeView == nullptr);
}

TEST_CASE("GetRouteView() explicit default server 3", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name server0;\n"
                    "    location /server0path0 {\n"
                    "      root server0root0;\n"
                    "    }\n"
                    "    location /server0path1 {\n"
                    "      root server0root1;\n"
                    "    }\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name server1;\n"
                    "    location /server1path0 {\n"
                    "      root server1root0;\n"
                    "    }\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8080 default_server;\n"
                    "    server_name server2;\n"
                    "      location /server2path0{\n"
                    "        root server2root0;\n"
                    "      }\n"
                    "      location /server2path1{\n"
                    "        root server2root1;\n"
                    "      }\n"
                    "      location /server2path2{\n"
                    "        root server2root2;\n"
                    "      }\n"
                    "\n"
                    "  }\n"
                    "}\n";

  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  
  REQUIRE(serverRegistry.GetServerViewCount() == 3);
  REQUIRE(serverRegistry.GetServerCount() == 1);

  // name matches
  const RouteView* routeView = serverRegistry.GetRouteView("::", "8080", "server0", "/server0path0");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server0root0");

  routeView = serverRegistry.GetRouteView("::", "8080", "server0", "/server0path1");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server0root1");

  routeView = serverRegistry.GetRouteView("::", "8080", "server1", "/server1path0");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server1root0");

  routeView = serverRegistry.GetRouteView("::", "8080", "server2", "/server2path0");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server2root0");

  routeView = serverRegistry.GetRouteView("::", "8080", "server2", "/server2path1");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server2root1");

  routeView = serverRegistry.GetRouteView("::", "8080", "server2", "/server2path2");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server2root2");

  routeView = serverRegistry.GetRouteView("::", "8080", "server2", "noMatch");
  REQUIRE(routeView == nullptr);


  // name does not match
  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "/server0path0");
  REQUIRE(routeView == nullptr);

  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "/server0path1");
  REQUIRE(routeView == nullptr);

  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "/server1path0");
  REQUIRE(routeView == nullptr);

  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "/server2path0");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server2root0");

  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "/server2path1");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server2root1");

  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "/server2path2");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->root == "server2root2");

  routeView = serverRegistry.GetRouteView("::", "8080", "noMatch", "noMatch");
  REQUIRE(routeView == nullptr);
}

TEST_CASE("ServerRegistry move constructor", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name example0.com;\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name example1.com;\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8082;\n"
                    "    server_name example2.com;\n"
                    "\n"
                    "  }\n"
                    "}\n";

  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  
  const void* addressServers = static_cast<const void*>(serverRegistry.GetServersData());
  const void* addressValue = static_cast<const void*>(serverRegistry.GetAddressValue({"::", "8080"}));
  ServerRegistry serverRegistryMoveConstructed(std::move(serverRegistry));
  const void* addressServersMoveConstructed = static_cast<const void*>(serverRegistryMoveConstructed.GetServersData());
  const void* addressValueMoveConstructed = static_cast<const void*>(serverRegistryMoveConstructed.GetAddressValue({"::", "8080"}));

  REQUIRE(addressServers == addressServersMoveConstructed);
  REQUIRE(addressValue == addressValueMoveConstructed);
  REQUIRE(serverRegistry.GetServerViewCount() == 0);
  REQUIRE(serverRegistryMoveConstructed.GetServerViewCount() == 3);
}

// INDIVIDUAL DIRECTIVES

TEST_CASE("listen", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    listen 9090;\n"
                    "    listen 120.0.0.64:8081;\n"
                    "    listen 120.0.0.64;\n"
                    "    listen [120::120]:8082;\n"
                    "    listen [120::120];\n"
                    "    server_name example0.com;\n"
                    "  }\n"
                    "  server {\n"
                    "    server_name example1.com;\n"
                    "  }\n"
                    "\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(serverRegistry.GetServerViewCount() == 7);

  const ServerView& serverView0(serverRegistry.GetServerView(0));
  const ServerView& serverView1(serverRegistry.GetServerView(1));
  const ServerView& serverView2(serverRegistry.GetServerView(2));
  const ServerView& serverView3(serverRegistry.GetServerView(3));
  const ServerView& serverView4(serverRegistry.GetServerView(4));
  const ServerView& serverView5(serverRegistry.GetServerView(5));
  const ServerView& serverView6(serverRegistry.GetServerView(6));

  REQUIRE(serverView0.ipPort.ip == "::");
  REQUIRE(serverView0.ipPort.port == "8081");
  REQUIRE(serverView1.ipPort.ip == "::");
  REQUIRE(serverView1.ipPort.port == "9090");
  REQUIRE(serverView2.ipPort.ip == "120.0.0.64");
  REQUIRE(serverView2.ipPort.port == "8081");
  REQUIRE(serverView3.ipPort.ip == "120.0.0.64");
  REQUIRE(serverView3.ipPort.port == "8080");
  REQUIRE(serverView4.ipPort.ip == "120::120");
  REQUIRE(serverView4.ipPort.port == "8082");
  REQUIRE(serverView5.ipPort.ip == "120::120");
  REQUIRE(serverView5.ipPort.port == "8080");
  REQUIRE(serverView6.ipPort.ip == "::");
  REQUIRE(serverView6.ipPort.port == "8080");
}

TEST_CASE("listen invalid - duplicate hostname IP:port combinations", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen [120::0ABC]:8082;\n"
                    "    listen 8080;\n"
                    "    listen 9090;\n"
                    "    listen 120.0.0.64:8081;\n"
                    "    listen 120.0.0.64;\n"
                    "    listen [0120::abc]:8082;\n"
                    "    listen [120::0001];\n"
                    "    server_name example0.com;\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 120.0.0.64:8081;\n"
                    "    server_name example1.com;\n"
                    "  }\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == true);

  REQUIRE(serverRegistry.GetServerViewCount() == 8);

  const ServerView& serverView0(serverRegistry.GetServerView(0));
  const ServerView& serverView1(serverRegistry.GetServerView(1));
  const ServerView& serverView2(serverRegistry.GetServerView(2));
  const ServerView& serverView3(serverRegistry.GetServerView(3));
  const ServerView& serverView4(serverRegistry.GetServerView(4));
  const ServerView& serverView5(serverRegistry.GetServerView(5));
  const ServerView& serverView6(serverRegistry.GetServerView(6));
  const ServerView& serverView7(serverRegistry.GetServerView(7));

  REQUIRE(serverView0.ipPort.ip == "120::abc");
  REQUIRE(serverView0.ipPort.port == "8082");
  REQUIRE(serverView1.ipPort.ip == "::");
  REQUIRE(serverView1.ipPort.port == "8080");
  REQUIRE(serverView2.ipPort.ip == "::");
  REQUIRE(serverView2.ipPort.port == "9090");
  REQUIRE(serverView3.ipPort.ip == "120.0.0.64");
  REQUIRE(serverView3.ipPort.port == "8081");
  REQUIRE(serverView4.ipPort.ip == "120.0.0.64");
  REQUIRE(serverView4.ipPort.port == "8080");
  REQUIRE(serverView5.ipPort.ip == "120::abc");
  REQUIRE(serverView5.ipPort.port == "8082");
  REQUIRE(serverView6.ipPort.ip == "120::1");
  REQUIRE(serverView6.ipPort.port == "8080");
  REQUIRE(serverView7.ipPort.ip == "120.0.0.64");
  REQUIRE(serverView7.ipPort.port == "8081");

  std::set<std::size_t> errorsIdx{20,26};
  for (size_t i = 0; i < lexer.GetSizeTokenList(); ++i)
  {
    if (errorsIdx.count(i) != 0)
    {
      CAPTURE(i);
      REQUIRE(lexer.GetTokenError(i) == true);
      REQUIRE(lexer.GetTokenErrorMessage(i).empty() == false);
    }
    else
    {
      CAPTURE(i);
      REQUIRE(lexer.GetTokenError(i) == false);
      REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
    }
  }
}

TEST_CASE("listen - ipv6 normalization", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen [::]:8081;\n"
                    "    server_name example0.com;\n"
                    "  }\n"
                    "  server {\n"
                    "    listen [::]:8082;\n"
                    "    server_name example1.com;\n"
                    "  }\n"
                    "  server {\n"
                    "    listen [120:120:120:120::120]:8081;\n"
                    "    server_name example2.com;\n"
                    "  }\n"
                    "  server {\n"
                    "    listen [120:120:120:120:0:0:0:120]:8082;\n"
                    "    server_name example3.com;\n"
                    "  }\n"
                    "  server {\n"
                    "    listen [120:120:120:120:120:120::120]:8081;\n"
                    "    server_name example4.com;\n"
                    "  }\n"
                    "  server {\n"
                    "    listen [120:120:120:120:120:120:0:120]:8082;\n"
                    "    server_name example5.com;\n"
                    "  }\n"
                    "  server {\n"
                    "    listen [120:120:120:120:120:120::]:8081;\n"
                    "    server_name example6.com;\n"
                    "  }\n"
                    "  server {\n"
                    "    listen [120:120:120:120:120:120:0:0]:8082;\n"
                    "    server_name example7.com;\n"
                    "  }\n"
                    "  server {\n"
                    "    listen [120:120:120:120:120:120:120:0]:8081;\n"
                    "    server_name example8.com;\n"
                    "  }\n"
                    "  server {\n"
                    "    listen [0:120:120:120:120:120:120:120]:8082;\n"
                    "    server_name example9.com;\n"
                    "  }\n"
                    "\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == true);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(serverRegistry.GetServerViewCount() == 10);

  const ServerView& serverView0(serverRegistry.GetServerView(0));
  const ServerView& serverView1(serverRegistry.GetServerView(1));
  const ServerView& serverView2(serverRegistry.GetServerView(2));
  const ServerView& serverView3(serverRegistry.GetServerView(3));
  const ServerView& serverView4(serverRegistry.GetServerView(4));
  const ServerView& serverView5(serverRegistry.GetServerView(5));
  const ServerView& serverView6(serverRegistry.GetServerView(6));
  const ServerView& serverView7(serverRegistry.GetServerView(7));
  const ServerView& serverView8(serverRegistry.GetServerView(8));
  const ServerView& serverView9(serverRegistry.GetServerView(9));

  REQUIRE(serverView0.ipPort.ip == "::");
  REQUIRE(serverView0.ipPort.port == "8081");
  REQUIRE(serverView1.ipPort.ip == "::");
  REQUIRE(serverView1.ipPort.port == "8082");
  REQUIRE(serverView2.ipPort.ip == "120:120:120:120::120");
  REQUIRE(serverView2.ipPort.port == "8081");
  REQUIRE(serverView3.ipPort.ip == "120:120:120:120::120");
  REQUIRE(serverView3.ipPort.port == "8082");
  REQUIRE(serverView4.ipPort.ip == "120:120:120:120:120:120:0:120");
  REQUIRE(serverView4.ipPort.port == "8081");
  REQUIRE(serverView5.ipPort.ip == "120:120:120:120:120:120:0:120");
  REQUIRE(serverView5.ipPort.port == "8082");
  REQUIRE(serverView6.ipPort.ip == "120:120:120:120:120:120::");
  REQUIRE(serverView6.ipPort.port == "8081");
  REQUIRE(serverView7.ipPort.ip == "120:120:120:120:120:120::");
  REQUIRE(serverView7.ipPort.port == "8082");
  REQUIRE(serverView8.ipPort.ip == "120:120:120:120:120:120:120:0");
  REQUIRE(serverView8.ipPort.port == "8081");
  REQUIRE(serverView9.ipPort.ip == "0:120:120:120:120:120:120:120");
  REQUIRE(serverView9.ipPort.port == "8082");
}

TEST_CASE("listen invalid - duplicate hostname IP:port combinations after ipv6 normalization", "[ServerRegistry]")
{
  std::string raw = "http {\n"
                    "  server {\n"
                    "    listen 120.0.0.120:8081;\n"
                    "    listen [120::120]:8082;\n"
                    "    listen [120:120:0:0:0:0:0:120]:8082;\n"
                    "    listen [0:0:0:0:0:0:0:0]:8082;\n"
                    "    listen [0:0:0:120:0:0:0:0]:8082;\n"
                    "    listen [0:0:0:0:120:0:0:0]:8082;\n"
                    "    listen [0000:000:00:0:0:0:0:120]:8082;\n"
                    "    listen [::];\n"
                    "    listen [0ABC:0:0:0:0:0:0:0]:8082;\n"
                    "    listen [0ABC:DEFF:000A:A000:0:0:0:0]:8082;\n"
                    "    listen [0ABC:0:000:A000:0:0:0:F]:8082;\n"
                    "    listen [0ABC:0:000:0:A000:0:0:F]:8082;\n"
                    "    server_name test.com;\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 120.0.0.120:8081;\n"
                    "    listen [120:0:0:0:0:0:0:120]:8082;\n"
                    "    listen [120:120::120]:8082;\n"
                    "    listen [::]:8082;\n"
                    "    listen [0:0:0:120::]:8082;\n"
                    "    listen [::120:0:0:0]:8082;\n"
                    "    listen [::120]:8082;\n"
                    "    listen [::];\n"
                    "    listen [abc::]:8082;\n"
                    "    listen [0abc:deff:000a:a000:0000:000:00:0]:8082;\n"
                    "    listen [0abC:0:000:a000:0:0:0:f]:8082;\n"
                    "    listen [0abc:0:000:0:a000:0:0:f]:8082;\n"
                    "    server_name test.com;\n"
                    "  }\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == true);

  REQUIRE(serverRegistry.GetServerViewCount() == 24);

  std::set<std::size_t> errorsIdx{47, 50, 53, 56, 59, 62, 65, 68, 71, 74, 77, 80, 83};
  for (size_t i = 0; i < lexer.GetSizeTokenList(); ++i)
  {
    if (errorsIdx.count(i) != 0)
    {
      CAPTURE(i);
      REQUIRE(lexer.GetTokenError(i) == true);
      REQUIRE(lexer.GetTokenErrorMessage(i).empty() == false);
    }
    else
    {
      CAPTURE(i);
      REQUIRE(lexer.GetTokenError(i) == false);
      REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
    }
  }
}

// jboon's fantastic test case

TEST_CASE("Servers with default listen values - duplicate hostname IP:port combinations", "[ServerRegistry]")
{
  std::string raw =
      "\n"
      "http {\n"
      "\n"
      "  server {\n"
      "    location /a {\n"
      "     root /root/a;\n"
      "    }\n"
      "  }\n"
      "  server {\n"
      "    location /b {\n"
      "     root /root/b;\n"
      "    }\n"
      "  }\n"
      "  server {\n"
      "    location /c {\n"
      "     root /root/c;\n"
      "    }\n"
      "  }\n"
      "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == true);

  std::set<std::size_t> errorsIdx{21, 31};
  for (size_t i = 0; i < lexer.GetSizeTokenList(); ++i)
  {
    if (errorsIdx.count(i) != 0)
    {
      CAPTURE(i);
      REQUIRE(lexer.GetTokenError(i) == true);
      REQUIRE(lexer.GetTokenErrorMessage(i).empty() == false);
    }
    else
    {
      CAPTURE(i);
      REQUIRE(lexer.GetTokenError(i) == false);
      REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
    }
  }
}

TEST_CASE("server_name - valid", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name name0 name1 name2;\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8082;\n"
                    "    server_name name3 name4 name5;\n"
                    "  }\n"
                    "  server {\n"
                    "  }\n"
                    "\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);


  REQUIRE(serverRegistry.GetServerViewCount() == 3);

  const ServerView& serverView0(serverRegistry.GetServerView(0));
  const ServerView& serverView1(serverRegistry.GetServerView(1));
  const ServerView& serverView2(serverRegistry.GetServerView(2));

  REQUIRE(serverView0.hostNames.size() == 3);
  REQUIRE(serverView0.hostNames.at(0) == "name0");
  REQUIRE(serverView0.hostNames.at(1) == "name1");
  REQUIRE(serverView0.hostNames.at(2) == "name2");
  REQUIRE(serverView1.hostNames.size() == 3);
  REQUIRE(serverView1.hostNames.at(0) == "name3");
  REQUIRE(serverView1.hostNames.at(1) == "name4");
  REQUIRE(serverView1.hostNames.at(2) == "name5");
  REQUIRE(serverView2.hostNames.size() == 1);
  REQUIRE(serverView2.hostNames.at(0) == "");
}

TEST_CASE("server_name - invalid", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name name0 name0 name2;\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8082;\n"
                    "    server_name name3 name4 name4;\n"
                    "  }\n"
                    "  server {\n"
                    "  }\n"
                    "\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == true);

  REQUIRE(serverRegistry.GetServerViewCount() == 3);

  const ServerView& serverView0(serverRegistry.GetServerView(0));
  const ServerView& serverView1(serverRegistry.GetServerView(1));
  const ServerView& serverView2(serverRegistry.GetServerView(2));

  REQUIRE(serverView0.hostNames.size() == 3);
  REQUIRE(serverView0.hostNames.at(0) == "name0");
  REQUIRE(serverView0.hostNames.at(1) == "name0");
  REQUIRE(serverView0.hostNames.at(2) == "name2");
  REQUIRE(serverView1.hostNames.size() == 3);
  REQUIRE(serverView1.hostNames.at(0) == "name3");
  REQUIRE(serverView1.hostNames.at(1) == "name4");
  REQUIRE(serverView1.hostNames.at(2) == "name4");
  REQUIRE(serverView2.hostNames.size() == 1);
  REQUIRE(serverView2.hostNames.at(0) == "");

  std::set<std::size_t> errorsIdx{9,21};
  for (size_t i = 0; i < lexer.GetSizeTokenList(); ++i)
  {
    if (errorsIdx.count(i) != 0)
    {
      CAPTURE(i);
      REQUIRE(lexer.GetTokenError(i) == true);
      REQUIRE(lexer.GetTokenErrorMessage(i).empty() == false);
    }
    else
    {
      CAPTURE(i);
      REQUIRE(lexer.GetTokenError(i) == false);
      REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
    }
  }
}

TEST_CASE("root", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name name0 name1 name2;\n"
                    "    root serverRoot0;\n"
                    "      location locPref00 {\n"
                    "      }\n"
                    "      location locPref01 {\n"
                    "        root locationRoot01;\n"
                    "      }\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name name3 name4 name5;\n"
                    "    root serverRoot1;\n"
                    "    location locPref1 {\n"
                    "      root locationRoot1;\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(serverRegistry.GetServerViewCount() == 2);

  const ServerView& serverView0(serverRegistry.GetServerView(0));
  const ServerView& serverView1(serverRegistry.GetServerView(1));

  REQUIRE(serverView0.routes.at(0).locationPrefix == "locPref00");
  REQUIRE(serverView0.routes.at(0).root == "serverRoot0");
  REQUIRE(serverView0.routes.at(1).locationPrefix == "locPref01");
  REQUIRE(serverView0.routes.at(1).root == "locationRoot01");
  REQUIRE(serverView1.routes.at(0).locationPrefix == "locPref1");
  REQUIRE(serverView1.routes.at(0).root == "locationRoot1");
}

TEST_CASE("index", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "  index httpIndex;\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name name0;\n"
                    "    index serverIndex0;\n"
                    "      location locPref00 {\n"
                    "      }\n"
                    "      location locPref01 {\n"
                    "        index locationIndex01;\n"
                    "      }\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name name1;\n"
                    "    location locPref1 {\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(serverRegistry.GetServerViewCount() == 2);

  const ServerView& serverView0(serverRegistry.GetServerView(0));
  const ServerView& serverView1(serverRegistry.GetServerView(1));

  REQUIRE(serverView0.routes.at(0).locationPrefix == "locPref00");
  REQUIRE(serverView0.routes.at(0).index == "serverIndex0");
  REQUIRE(serverView0.routes.at(1).locationPrefix == "locPref01");
  REQUIRE(serverView0.routes.at(1).index == "locationIndex01");
  REQUIRE(serverView1.routes.at(0).locationPrefix == "locPref1");
  REQUIRE(serverView1.routes.at(0).index == "httpIndex");
}

TEST_CASE("alias", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name name0;\n"
                    "      location locPref00 {\n"
                    "      }\n"
                    "      location locPref01 {\n"
                    "        alias locationAlias01;\n"
                    "      }\n"
                    "  }\n"
                    "\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(serverRegistry.GetServerViewCount() == 1);

  const ServerView& serverView0(serverRegistry.GetServerView(0));

  REQUIRE(serverView0.routes.at(0).locationPrefix == "locPref00");
  REQUIRE(serverView0.routes.at(0).alias.has_value() == false);
  REQUIRE(serverView0.routes.at(1).locationPrefix == "locPref01");
  REQUIRE(serverView0.routes.at(1).alias == "locationAlias01");
}

TEST_CASE("client_max_body_size", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "  client_max_body_size 8m;\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name name0;\n"
                    "    client_max_body_size 32K;\n"
                    "      location locPref00 {\n"
                    "      }\n"
                    "      location locPref01 {\n"
                    "        client_max_body_size 1G;\n"
                    "      }\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name name1;\n"
                    "    location locPref1 {\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(serverRegistry.GetServerViewCount() == 2);

  const ServerView& serverView0(serverRegistry.GetServerView(0));
  const ServerView& serverView1(serverRegistry.GetServerView(1));

  REQUIRE(serverView0.routes.at(0).locationPrefix == "locPref00");
  REQUIRE(serverView0.routes.at(0).clientMaxBody == 32 * 1024);
  REQUIRE(serverView0.routes.at(1).locationPrefix == "locPref01");
  REQUIRE(serverView0.routes.at(1).clientMaxBody == 1 * 1024 * 1024 * 1024);
  REQUIRE(serverView1.routes.at(0).locationPrefix == "locPref1");
  REQUIRE(serverView1.routes.at(0).clientMaxBody == 8 * 1024 * 1024);
}

TEST_CASE("error_page", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "  error_page 301 /httpErrorPage;\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name name0;\n"
                    "    error_page 401 /serverErrorPage;\n"
                    "      location locPref00 {\n"
                    "      }\n"
                    "      location locPref01 {\n"
                    "        error_page 500 http://locationErrorPage;\n"
                    "      }\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name name1;\n"
                    "    location locPref1 {\n"
                    "      error_page 301 https://locationErrorPage;\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(serverRegistry.GetServerViewCount() == 2);

  const ServerView& serverView0(serverRegistry.GetServerView(0));
  const ServerView& serverView1(serverRegistry.GetServerView(1));

  REQUIRE(serverView0.routes.at(0).locationPrefix == "locPref00");
  REQUIRE(serverView0.routes.at(0).errorPages.size() == 2);
  REQUIRE(serverView0.routes.at(0).errorPages.at(301) == "/httpErrorPage");
  REQUIRE(serverView0.routes.at(0).errorPages.at(401) == "/serverErrorPage");
  REQUIRE(serverView0.routes.at(1).locationPrefix == "locPref01");
  REQUIRE(serverView0.routes.at(1).errorPages.size() == 3);
  REQUIRE(serverView0.routes.at(1).errorPages.at(301) == "/httpErrorPage");
  REQUIRE(serverView0.routes.at(1).errorPages.at(401) == "/serverErrorPage");
  REQUIRE(serverView0.routes.at(1).errorPages.at(500) == "http://locationErrorPage");
  REQUIRE(serverView1.routes.at(0).locationPrefix == "locPref1");
  REQUIRE(serverView1.routes.at(0).errorPages.size() == 1);
  REQUIRE(serverView1.routes.at(0).errorPages.at(301) == "https://locationErrorPage");
}

TEST_CASE("return", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name name0;\n"
                    "    return 301 /new.com;\n"
                    "      location locPref00 {\n"
                    "      }\n"
                    "      location locPref01 {\n"
                    "        return 301 http://new01.com;\n"
                    "      }\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name name1;\n"
                    "    location locPref10 {\n"
                    "      return http://temporary.com;\n"
                    "    }\n"
                    "    location locPref11 {\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(serverRegistry.GetServerViewCount() == 2);

  const ServerView& serverView0(serverRegistry.GetServerView(0));
  const ServerView& serverView1(serverRegistry.GetServerView(1));

  REQUIRE(serverView0.routes.at(0).locationPrefix == "locPref00");
  REQUIRE(serverView0.routes.at(0).returnRule.has_value() == true);
  REQUIRE(serverView0.routes.at(0).returnRule->code == 301);
  REQUIRE(serverView0.routes.at(0).returnRule->target == "/new.com");
  REQUIRE(serverView0.routes.at(1).locationPrefix == "locPref01");
  REQUIRE(serverView0.routes.at(1).returnRule.has_value() == true);
  REQUIRE(serverView0.routes.at(1).returnRule->code == 301);
  REQUIRE(serverView0.routes.at(1).returnRule->target == "http://new01.com");
  REQUIRE(serverView1.routes.at(0).locationPrefix == "locPref10");
  REQUIRE(serverView1.routes.at(0).returnRule.has_value() == true);
  REQUIRE(serverView1.routes.at(0).returnRule->code == 302);
  REQUIRE(serverView1.routes.at(0).returnRule->target == "http://temporary.com");
  REQUIRE(serverView1.routes.at(1).locationPrefix == "locPref11");
  REQUIRE(serverView1.routes.at(1).returnRule.has_value() == false);
}

TEST_CASE("allowed_methods", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name name0;\n"
                    "    allowed_methods POST;\n"
                    "      location locPref00 {\n"
                    "      }\n"
                    "      location locPref01 {\n"
                    "        allowed_methods DELETE;\n"
                    "      }\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name name1;\n"
                    "    location locPref10 {\n"
                    "      allowed_methods GET POST DELETE;\n"
                    "    }\n"
                    "    location locPref11 {\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(serverRegistry.GetServerViewCount() == 2);

  const ServerView& serverView0(serverRegistry.GetServerView(0));
  const ServerView& serverView1(serverRegistry.GetServerView(1));

  REQUIRE(serverView0.routes.at(0).locationPrefix == "locPref00");
  REQUIRE(serverView0.routes.at(0).allowedMask == RouteView::MethodMask::kPost);
  REQUIRE(serverView0.routes.at(1).locationPrefix == "locPref01");
  REQUIRE(serverView0.routes.at(1).allowedMask == RouteView::MethodMask::kDelete);
  REQUIRE(serverView1.routes.at(0).locationPrefix == "locPref10");
  REQUIRE(serverView1.routes.at(0).allowedMask == (RouteView::MethodMask::kGet | RouteView::MethodMask::kPost | RouteView::MethodMask::kDelete));
  REQUIRE(serverView1.routes.at(1).locationPrefix == "locPref11");
  REQUIRE(serverView1.routes.at(1).allowedMask == RouteView::MethodMask::kGet);
}

TEST_CASE("autoindex http default (off)", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name name0;\n"
                    "    autoindex on;\n"
                    "      location locPref00 {\n"
                    "      }\n"
                    "      location locPref01 {\n"
                    "        autoindex off;\n"
                    "      }\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name name1;\n"
                    "    location locPref10 {\n"
                    "    }\n"
                    "    location locPref11 {\n"
                    "      autoindex on;\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(serverRegistry.GetServerViewCount() == 2);

  const ServerView& serverView0(serverRegistry.GetServerView(0));
  const ServerView& serverView1(serverRegistry.GetServerView(1));

  REQUIRE(serverView0.routes.at(0).locationPrefix == "locPref00");
  REQUIRE(serverView0.routes.at(0).autoindex == true);
  REQUIRE(serverView0.routes.at(1).locationPrefix == "locPref01");
  REQUIRE(serverView0.routes.at(1).autoindex == false);
  REQUIRE(serverView1.routes.at(0).locationPrefix == "locPref10");
  REQUIRE(serverView1.routes.at(0).autoindex == false);
  REQUIRE(serverView1.routes.at(1).locationPrefix == "locPref11");
  REQUIRE(serverView1.routes.at(1).autoindex == true);
}

TEST_CASE("autoindex http on", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "  autoindex on;\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name name0;\n"
                    "      location locPref00 {\n"
                    "      }\n"
                    "      location locPref01 {\n"
                    "        autoindex off;\n"
                    "      }\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name name1;\n"
                    "    autoindex off;\n"
                    "    location locPref10 {\n"
                    "    }\n"
                    "    location locPref11 {\n"
                    "      autoindex on;\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(serverRegistry.GetServerViewCount() == 2);

  const ServerView& serverView0(serverRegistry.GetServerView(0));
  const ServerView& serverView1(serverRegistry.GetServerView(1));

  REQUIRE(serverView0.routes.at(0).locationPrefix == "locPref00");
  REQUIRE(serverView0.routes.at(0).autoindex == true);
  REQUIRE(serverView0.routes.at(1).locationPrefix == "locPref01");
  REQUIRE(serverView0.routes.at(1).autoindex == false);
  REQUIRE(serverView1.routes.at(0).locationPrefix == "locPref10");
  REQUIRE(serverView1.routes.at(0).autoindex == false);
  REQUIRE(serverView1.routes.at(1).locationPrefix == "locPref11");
  REQUIRE(serverView1.routes.at(1).autoindex == true);
}

TEST_CASE("cgi", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name name0;\n"
                    "      location locPref00 {\n"
                    "      }\n"
                    "      location locPref01 {\n"
                    "        cgi on;\n"
                    "      }\n"
                    "  }\n"
                    "\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(serverRegistry.GetServerViewCount() == 1);

  const ServerView& serverView0(serverRegistry.GetServerView(0));

  REQUIRE(serverView0.routes.at(0).locationPrefix == "locPref00");
  REQUIRE(serverView0.routes.at(0).cgi == false);
  REQUIRE(serverView0.routes.at(1).locationPrefix == "locPref01");
  REQUIRE(serverView0.routes.at(1).cgi == true);
}

TEST_CASE("cgi_extension", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name name0;\n"
                    "      location locPref00 {\n"
                    "      }\n"
                    "      location locPref01 {\n"
                    "        cgi_extension php /usr/bin/location-php-cgi;\n"
                    "      }\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name name1;\n"
                    "    cgi_extension php /usr/bin/server-php-cgi;\n"
                    "    cgi_extension perl /usr/bin/server-perl;\n"
                    "    location locPref10 {\n"
                    "    }\n"
                    "    location locPref11 {\n"
                    "      cgi_extension py /usr/bin/location-python3;\n"
                    "      cgi_extension php /usr/bin/location-php-cgi;\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(serverRegistry.GetServerViewCount() == 2);

  const ServerView& serverView0(serverRegistry.GetServerView(0));
  const ServerView& serverView1(serverRegistry.GetServerView(1));

  REQUIRE(serverView0.routes.at(0).locationPrefix == "locPref00");
  REQUIRE(serverView0.routes.at(0).cgiExePaths.has_value() == false);
  REQUIRE(serverView0.routes.at(1).locationPrefix == "locPref01");
  REQUIRE(serverView0.routes.at(1).cgiExePaths->size() == 1);
  REQUIRE(serverView0.routes.at(1).cgiExePaths->at("php") == "/usr/bin/location-php-cgi");
  REQUIRE(serverView1.routes.at(0).locationPrefix == "locPref10");
  REQUIRE(serverView1.routes.at(0).cgiExePaths->size() == 2);
  REQUIRE(serverView1.routes.at(0).cgiExePaths->at("php") == "/usr/bin/server-php-cgi");
  REQUIRE(serverView1.routes.at(0).cgiExePaths->at("perl") == "/usr/bin/server-perl");
  REQUIRE(serverView1.routes.at(1).locationPrefix == "locPref11");
  REQUIRE(serverView1.routes.at(1).cgiExePaths->size() == 3);
  REQUIRE(serverView1.routes.at(1).cgiExePaths->at("perl") == "/usr/bin/server-perl");
  REQUIRE(serverView1.routes.at(1).cgiExePaths->at("py") == "/usr/bin/location-python3");
  REQUIRE(serverView1.routes.at(1).cgiExePaths->at("php") == "/usr/bin/location-php-cgi");
}

// SOMEWHAT REALISTIC TEST CASE

TEST_CASE("one server, four locations, lots of inheritance", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "  index indexHttp.html;\n"
                    "  autoindex on;\n"
                    "  client_max_body_size 10m;\n"
                    "  error_page 400 401 402 /http;\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name example.com;\n"
                    "    client_max_body_size 2m;\n"
                    "    error_page 500 /server;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index indexLocation.html;\n"
                    "      autoindex off;\n"
                    "      error_page 504 /location;\n"
                    "    }\n"
                    "\n"
                    "    location /uploads {\n"
                    "      root ./www;\n"
                    "      client_max_body_size 10m;\n"
                    "      allowed_methods POST DELETE;\n"
                    "      error_page 400 401 402 /location;\n"
                    "    }\n"
                    "\n"
                    "    location /old-page {\n"
                    "      return 301 /new-page;\n"
                    "    }\n"
                    "\n"
                    "    location /cgi-bin {\n"
                    "      root ./www;\n"
                    "      allowed_methods GET POST;\n"
                    "      cgi_extension php /usr/bin/php-cgi;\n"
                    "      cgi_extension py /usr/bin/python3;\n"
                    "    }\n"
                    "  }\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(serverRegistry.GetServerViewCount() == 1);

  const ServerView& serverView = serverRegistry.GetServerView(0);

  REQUIRE(serverView.hostNames[0] == "example.com");
  REQUIRE(serverView.ipPort.ip == "::");
  REQUIRE(serverView.ipPort.port == "8080");
  REQUIRE(serverView.routes.size() == 4);

  const RouteView& routeView0 = serverView.routes[0];
  REQUIRE(routeView0.locationPrefix == "/");
  REQUIRE(routeView0.root == std::filesystem::path("./www"));
  REQUIRE(routeView0.alias.has_value() == false);
  REQUIRE(routeView0.index == "indexLocation.html");
  REQUIRE(routeView0.autoindex == false);
  REQUIRE(routeView0.clientMaxBody == std::size_t(2 * 1024 * 1024));
  REQUIRE(routeView0.allowedMask == RouteView::MethodMask::kGet);
  REQUIRE(routeView0.returnRule.has_value() == false);
  REQUIRE(routeView0.errorPages.size() == 5);
  REQUIRE(routeView0.errorPages.at(400) == "/http");
  REQUIRE(routeView0.errorPages.at(401) == "/http");
  REQUIRE(routeView0.errorPages.at(402) == "/http");
  REQUIRE(routeView0.errorPages.at(500) == "/server");
  REQUIRE(routeView0.errorPages.at(504) == "/location");
  REQUIRE(routeView0.cgi == false);
  REQUIRE(routeView0.cgiExePaths.has_value() == false);

  const RouteView& routeView1 = serverView.routes[1];
  REQUIRE(routeView1.locationPrefix == "/uploads");
  REQUIRE(routeView1.root == std::filesystem::path("./www"));
  REQUIRE(routeView1.alias.has_value() == false);
  REQUIRE(routeView1.index == "indexHttp.html");
  REQUIRE(routeView1.autoindex == true);
  REQUIRE(routeView1.clientMaxBody == std::size_t(10 * 1024 * 1024));
  REQUIRE(routeView1.allowedMask == (RouteView::MethodMask::kPost | RouteView::MethodMask::kDelete));
  REQUIRE(routeView1.returnRule.has_value() == false);
  REQUIRE(routeView1.errorPages.size() == 4);
  REQUIRE(routeView1.errorPages.at(400) == "/location");
  REQUIRE(routeView1.errorPages.at(401) == "/location");
  REQUIRE(routeView1.errorPages.at(402) == "/location");
  REQUIRE(routeView1.errorPages.at(500) == "/server");
  REQUIRE(routeView1.cgi == false);
  REQUIRE(routeView1.cgiExePaths.has_value() == false);

  const RouteView& routeView2 = serverView.routes[2];
  REQUIRE(routeView2.locationPrefix == "/old-page");
  REQUIRE(routeView2.root == std::filesystem::path("./www"));
  REQUIRE(routeView2.alias.has_value() == false);
  REQUIRE(routeView2.index == "indexHttp.html");
  REQUIRE(routeView2.autoindex == true);
  REQUIRE(routeView2.clientMaxBody == std::size_t(2 * 1024 * 1024));
  REQUIRE(routeView2.allowedMask == (RouteView::MethodMask::kGet));
  REQUIRE(routeView2.returnRule.has_value() == true);
  REQUIRE(routeView2.returnRule->code == std::uint16_t(301));
  REQUIRE(routeView2.returnRule->target == "/new-page");
  REQUIRE(routeView2.errorPages.size() == 4);
  REQUIRE(routeView2.errorPages.at(400) == "/http");
  REQUIRE(routeView2.errorPages.at(401) == "/http");
  REQUIRE(routeView2.errorPages.at(402) == "/http");
  REQUIRE(routeView2.errorPages.at(500) == "/server");
  REQUIRE(routeView2.cgi == false);
  REQUIRE(routeView2.cgiExePaths.has_value() == false);

  const RouteView& routeView3 = serverView.routes[3];
  REQUIRE(routeView3.locationPrefix == "/cgi-bin");
  REQUIRE(routeView3.root == std::filesystem::path("./www"));
  REQUIRE(routeView3.alias.has_value() == false);
  REQUIRE(routeView3.index == "indexHttp.html");
  REQUIRE(routeView3.autoindex == true);
  REQUIRE(routeView3.clientMaxBody == std::size_t(2 * 1024 * 1024));
  REQUIRE(routeView3.allowedMask == (RouteView::MethodMask::kGet | RouteView::MethodMask::kPost));
  REQUIRE(routeView3.returnRule.has_value() == false);
  REQUIRE(routeView3.errorPages.size() == 4);
  REQUIRE(routeView3.errorPages.at(400) == "/http");
  REQUIRE(routeView3.errorPages.at(401) == "/http");
  REQUIRE(routeView3.errorPages.at(402) == "/http");
  REQUIRE(routeView3.errorPages.at(500) == "/server");
  REQUIRE(routeView3.cgi == false);
  REQUIRE(routeView3.cgiExePaths.has_value() == true);
  REQUIRE(routeView3.cgiExePaths->size() == 2);
  REQUIRE(routeView3.cgiExePaths->at("php") == std::filesystem::path("/usr/bin/php-cgi"));
  REQUIRE(routeView3.cgiExePaths->at("py") == std::filesystem::path("/usr/bin/python3"));
}

TEST_CASE("minimal valid config file", "[ServerRegistry]")
{
  std::string raw = "http{server{}}";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(serverRegistry.GetServerViewCount() == 1);

  const ServerView& serverView = serverRegistry.GetServerView(0);

  REQUIRE(serverView.hostNames[0] == "");
  REQUIRE(serverView.ipPort.ip == "::");
  REQUIRE(serverView.ipPort.port == "8080");
  REQUIRE(serverView.routes.size() == 1);

  const RouteView& routeView0 = serverView.routes[0];
  REQUIRE(routeView0.locationPrefix == "/");
  REQUIRE(routeView0.root == std::filesystem::path("./www"));
  REQUIRE(routeView0.alias.has_value() == false);
  REQUIRE(routeView0.index == "index.html");
  REQUIRE(routeView0.autoindex == false);
  REQUIRE(routeView0.clientMaxBody == std::size_t(1 * 1024 * 1024));
  REQUIRE(routeView0.allowedMask == RouteView::MethodMask::kGet);
  REQUIRE(routeView0.returnRule.has_value() == false);
  REQUIRE(routeView0.errorPages.size() == 0);
  REQUIRE(routeView0.cgi == false);
  REQUIRE(routeView0.cgiExePaths.has_value() == false);

  const RouteView* routeView = serverRegistry.GetRouteView("::", "8080", "", "/");
  REQUIRE(routeView != nullptr);
  REQUIRE(routeView->locationPrefix == "/");
  REQUIRE(routeView->root == std::filesystem::path("./www"));
  REQUIRE(routeView->alias.has_value() == false);
  REQUIRE(routeView->index == "index.html");
  REQUIRE(routeView->autoindex == false);
  REQUIRE(routeView->clientMaxBody == std::size_t(1 * 1024 * 1024));
  REQUIRE(routeView->allowedMask == RouteView::MethodMask::kGet);
  REQUIRE(routeView->returnRule.has_value() == false);
  REQUIRE(routeView->errorPages.size() == 0);
  REQUIRE(routeView->cgi == false);
  REQUIRE(routeView->cgiExePaths.has_value() == false);
}

TEST_CASE("Bell's default.conf", "[ServerRegistry]")
{
  std::string raw = " http {"
                    "   index index.html;"
                    "   client_max_body_size 1m;"
                    "   autoindex off;"
                    "   server {"
                    "     listen 8080;"
                    "     server_name localhost;"
                    "     root ./www;"
                    "     index index.html;"
                    "     allowed_methods GET POST DELETE;"
                    "     error_page 404 /404.html;"
                    "     error_page 500 /500.html;"
                    "     # default location\n"
                    "     location / {"
                    "         root ./www;"
                    "         autoindex on;"
                    "     }"
                    "     # upload / POST testing\n"
                    "     location /upload {"
                    "         root ./www;"
                    "         allowed_methods POST;"
                    "     }"
                    "     # delete testing\n"
                    "     location /delete {"
                    "        root ./www;"
                    "        allowed_methods DELETE;"
                    "     }"
                    "     # CGI (for later)\n"
                    "     location /cgi-bin {"
                    "         root ./www/cgi-bin;"
                    "         cgi on;"
                    "         cgi_extension .py /usr/bin/python3;"
                    "         cgi_extension .sh /bin/bash;"
                    "         allowed_methods GET POST;"
                    "     }"
                    "   }"
                    " }";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  lexer.PrintErrorMessages();

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(serverRegistry.GetServerViewCount() == 1);

  const ServerView& serverView = serverRegistry.GetServerView(0);
  REQUIRE(serverView.hostNames.size() == 1);
  REQUIRE(serverView.hostNames[0] == "localhost");
  REQUIRE(serverView.ipPort.ip == "::");
  REQUIRE(serverView.ipPort.port == "8080");
  REQUIRE(serverView.routes.size() == 4);

  const RouteView& routeView0 = serverView.routes[0];
  REQUIRE(routeView0.locationPrefix == "/");
  REQUIRE(routeView0.root == std::filesystem::path("./www"));
  REQUIRE(routeView0.alias.has_value() == false);
  REQUIRE(routeView0.index == "index.html");
  REQUIRE(routeView0.autoindex == true);
  REQUIRE(routeView0.clientMaxBody == std::size_t(1 * 1024 * 1024));
  REQUIRE(routeView0.allowedMask == (RouteView::MethodMask::kGet | RouteView::MethodMask::kPost | RouteView::MethodMask::kDelete));
  REQUIRE(routeView0.returnRule.has_value() == false);
  REQUIRE(routeView0.errorPages.size() == 2);
  REQUIRE(routeView0.errorPages.at(404) == "/404.html");
  REQUIRE(routeView0.errorPages.at(500) == "/500.html");
  REQUIRE(routeView0.cgi == false);
  REQUIRE(routeView0.cgiExePaths.has_value() == false);

  const RouteView& routeView1 = serverView.routes[1];
  REQUIRE(routeView1.locationPrefix == "/upload");
  REQUIRE(routeView1.root == std::filesystem::path("./www"));
  REQUIRE(routeView1.alias.has_value() == false);
  REQUIRE(routeView1.index == "index.html");
  REQUIRE(routeView1.autoindex == false);
  REQUIRE(routeView1.clientMaxBody == std::size_t(1 * 1024 * 1024));
  REQUIRE(routeView1.allowedMask == RouteView::MethodMask::kPost);
  REQUIRE(routeView1.returnRule.has_value() == false);
  REQUIRE(routeView1.errorPages.size() == 2);
  REQUIRE(routeView1.errorPages.at(404) == "/404.html");
  REQUIRE(routeView1.errorPages.at(500) == "/500.html");
  REQUIRE(routeView1.cgi == false);
  REQUIRE(routeView1.cgiExePaths.has_value() == false);

  const RouteView& routeView2 = serverView.routes[2];
  REQUIRE(routeView2.locationPrefix == "/delete");
  REQUIRE(routeView2.root == std::filesystem::path("./www"));
  REQUIRE(routeView2.alias.has_value() == false);
  REQUIRE(routeView2.index == "index.html");
  REQUIRE(routeView2.autoindex == false);
  REQUIRE(routeView2.clientMaxBody == std::size_t(1 * 1024 * 1024));
  REQUIRE(routeView2.allowedMask == RouteView::MethodMask::kDelete);
  REQUIRE(routeView2.returnRule.has_value() == false);
  REQUIRE(routeView2.errorPages.size() == 2);
  REQUIRE(routeView2.errorPages.at(404) == "/404.html");
  REQUIRE(routeView2.errorPages.at(500) == "/500.html");
  REQUIRE(routeView2.cgi == false);
  REQUIRE(routeView2.cgiExePaths.has_value() == false);

  const RouteView& routeView3 = serverView.routes[3];
  REQUIRE(routeView3.locationPrefix == "/cgi-bin");
  REQUIRE(routeView3.root == std::filesystem::path("./www/cgi-bin"));
  REQUIRE(routeView3.alias.has_value() == false);
  REQUIRE(routeView3.index == "index.html");
  REQUIRE(routeView3.autoindex == false);
  REQUIRE(routeView3.clientMaxBody == std::size_t(1 * 1024 * 1024));
  REQUIRE(routeView3.allowedMask == (RouteView::MethodMask::kGet | RouteView::MethodMask::kPost));
  REQUIRE(routeView3.returnRule.has_value() == false);
  REQUIRE(routeView3.errorPages.size() == 2);
  REQUIRE(routeView3.errorPages.at(404) == "/404.html");
  REQUIRE(routeView3.errorPages.at(500) == "/500.html");
  REQUIRE(routeView3.cgi == true);
  REQUIRE(routeView3.cgiExePaths.has_value() == true);
  REQUIRE(routeView3.cgiExePaths->size() == 2);
  REQUIRE(routeView3.cgiExePaths->at(".sh") == std::filesystem::path("/bin/bash"));
  REQUIRE(routeView3.cgiExePaths->at(".py") == std::filesystem::path("/usr/bin/python3"));
}