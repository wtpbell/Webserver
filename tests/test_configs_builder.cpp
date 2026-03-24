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

// PUBLIC FUNCTIONS

TEST_CASE("GetServerCount()", "[Builder]")
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  Builder builder(lexer, parser, validatorIpPort);
  builder.Build();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(builder.GetServerCount() == 3);
}

TEST_CASE("GetServerView()", "[Builder]")
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
                    "    listen 8083;\n"
                    "    server_name example3.com;\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8084;\n"
                    "    server_name example4.com;\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8085;\n"
                    "    server_name example5.com;\n"
                    "\n"
                    "  }\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  Builder builder(lexer, parser, validatorIpPort);
  builder.Build();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(builder.GetServerCount() == 6);

  const ServerView& serverView0(builder.GetServerView(0));
  const ServerView& serverView1(builder.GetServerView(1));
  const ServerView& serverView2(builder.GetServerView(2));
  const ServerView& serverView3(builder.GetServerView(3));
  const ServerView& serverView4(builder.GetServerView(4));
  const ServerView& serverView5(builder.GetServerView(5));

  REQUIRE(serverView0.hostNames.size() == 1);
  REQUIRE(serverView0.hostNames.at(0) == "example0.com");
  REQUIRE(serverView0.ipPortList.at(0).first == "::");
  REQUIRE(serverView0.ipPortList.at(0).second == 8080);

  REQUIRE(serverView1.hostNames.size() == 1);
  REQUIRE(serverView1.hostNames.at(0) == "example1.com");
  REQUIRE(serverView1.ipPortList.at(0).first == "::");
  REQUIRE(serverView1.ipPortList.at(0).second == 8081);
  
  REQUIRE(serverView2.hostNames.size() == 1);
  REQUIRE(serverView2.hostNames.at(0) == "example2.com");
  REQUIRE(serverView2.ipPortList.at(0).first == "::");
  REQUIRE(serverView2.ipPortList.at(0).second == 8082);
  
  REQUIRE(serverView3.hostNames.size() == 1);
  REQUIRE(serverView3.hostNames.at(0) == "example3.com");
  REQUIRE(serverView3.ipPortList.at(0).first == "::");
  REQUIRE(serverView3.ipPortList.at(0).second == 8083);
  
  REQUIRE(serverView4.hostNames.size() == 1);
  REQUIRE(serverView4.hostNames.at(0) == "example4.com");
  REQUIRE(serverView4.ipPortList.at(0).first == "::");
  REQUIRE(serverView4.ipPortList.at(0).second == 8084);
  
  REQUIRE(serverView5.hostNames.size() == 1);
  REQUIRE(serverView5.hostNames.at(0) == "example5.com");
  REQUIRE(serverView5.ipPortList.at(0).first == "::");
  REQUIRE(serverView5.ipPortList.at(0).second == 8085);
}

TEST_CASE("GetRouteView(hostname, targetPath)", "[Builder]")
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
                    "\n"
                    "    }\n"
                    "    location /test/ {\n"
                    "\n"
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
                    "    listen 8081;\n"
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
                    "  \n"
                    "    location /test2{\n"
                    "    }\n"
                    "  }\n"
                    "  server {\n"
                    "    server_name example2.com;\n"
                    "  }\n"
                    "  server {\n"
                    "  \n"
                    "    location /test3{\n"
                    "    }\n"
                    "  }\n"
                    "  server {\n"
                    "    server_name example2.com;\n"
                    "    location /not-default {\n"
                    "    }\n"
                    "  }\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  Builder builder(lexer, parser, validatorIpPort);
  builder.Build();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

std::cout << output << std::endl;

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(builder.GetServerCount() == 6);

  // unknown host name: invalid
  const RouteView* routeView = builder.GetRouteView("ex.com", "/test");
  REQUIRE(routeView == nullptr);

  // no location prefix match: invalid
  routeView = builder.GetRouteView("ex0.com", "nope, not today");
  REQUIRE(routeView == nullptr);

  // location parameter /test is prefix of request URI /testtest, but location param is not path segment of URI: invalid
  routeView = builder.GetRouteView("ex0.com", "/testtest");
  REQUIRE(routeView == nullptr);

  // request URI is prefix of location parameter, but not vice versa: invalid
  routeView = builder.GetRouteView("example.com", "/tes");
  REQUIRE(routeView == nullptr);

  // request URI empty string: invalid
  routeView = builder.GetRouteView("example.com", "");
  REQUIRE(routeView == nullptr);
  
  // location parameter and request URI identical: valid
  routeView = builder.GetRouteView("ex0.com", "/tessst/test/test");
  REQUIRE(routeView->locationPrefix == "/tessst/test/test");

  // location parameter and request URI identical, both ending in '/': valid
  routeView = builder.GetRouteView("ex0.com", "/test/");
  REQUIRE(routeView->locationPrefix == "/test/");

  // location parameter and request URI identical, neither ending in '/': valid
  routeView = builder.GetRouteView("ex0.com", "/test");
  REQUIRE(routeView->locationPrefix == "/test");

  // location parameter is prefix of request URI, location parameter ending in '/': valid
  routeView = builder.GetRouteView("ex0.com", "/test/test");
  REQUIRE(routeView->locationPrefix == "/test/");

  // location parameter is prefix and path segment of request URI: valid
  routeView = builder.GetRouteView("example1.com", "/tessst1/test/test/test/test");
  REQUIRE(routeView->locationPrefix == "/tessst1/test/test");

  // minimal match location parameter and request URI (location param is one char): '/': valid
  routeView = builder.GetRouteView("ex1.com", "/abc");
  REQUIRE(routeView->locationPrefix == "/");

  // minimal match location parameter and request URI (both one char): '/': valid
  routeView = builder.GetRouteView("ex1.com", "/");
  REQUIRE(routeView->locationPrefix == "/");

  // minimal match location parameter and request URI (one char): 't': valid
  routeView = builder.GetRouteView("ex1.com", "t/test");
  REQUIRE(routeView->locationPrefix == "t");

  // minimal match location parameter and request URI (both one char): 't': valid
  routeView = builder.GetRouteView("ex1.com", "t");
  REQUIRE(routeView->locationPrefix == "t");

  // anonymous server, match in first anonymous server: valid
  routeView = builder.GetRouteView("", "/test2");
  REQUIRE(routeView->locationPrefix == "/test2");

  // anonymous server, match in second anonymous server: valid
  routeView = builder.GetRouteView("", "/test3");
  REQUIRE(routeView->locationPrefix == "/test3");

  // anonymous server, invalid target
  routeView = builder.GetRouteView("", "/invalid");
  REQUIRE(routeView == nullptr);

  // no location block in server, target == default location prefix
  routeView = builder.GetRouteView("example2.com", "/");
  REQUIRE(routeView->locationPrefix == "/");

  // two homonymous servers, match in second server: valid
  routeView = builder.GetRouteView("example2.com", "/not-default");
  REQUIRE(routeView->locationPrefix == "/not-default");

  // no location block in server, target != default location prefix: invalid
  routeView = builder.GetRouteView("example2.com", "error");
  REQUIRE(routeView == nullptr);
}

TEST_CASE("GetRouteView(ip, port, hostName, targetPath)", "[Builder]")
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
                    "\n"
                    "    }\n"
                    "    location /test/ {\n"
                    "\n"
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
                    "  \n"
                    "    location /test2{\n"
                    "    }\n"
                    "  }\n"
                    "  server {\n"
                    "    server_name example2.com;\n"
                    "  }\n"
                    "  server {\n"
                    "  \n"
                    "    location /test3{\n"
                    "    }\n"
                    "  }\n"
                    "  server {\n"
                    "    server_name example2.com;\n"
                    "    location /not-default {\n"
                    "    }\n"
                    "  }\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  Builder builder(lexer, parser, validatorIpPort);
  builder.Build();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(builder.GetServerCount() == 6);

  // unknown host name: invalid
  const RouteView* routeView = builder.GetRouteView("::", 8080, "ex.com", "/test");
  REQUIRE(routeView == nullptr);

  // no location prefix match: invalid
  routeView = builder.GetRouteView("::", 8080, "ex0.com", "nope, not today");
  REQUIRE(routeView == nullptr);

  // location parameter /test is prefix of request URI /testtest, but location param is not path segment of URI: invalid
  routeView = builder.GetRouteView("::", 8080, "ex0.com", "/testtest");
  REQUIRE(routeView == nullptr);

  // request URI is prefix of location parameter, but not vice versa: invalid
  routeView = builder.GetRouteView("::", 8080, "example.com", "/tes");
  REQUIRE(routeView == nullptr);

  // request URI empty string: invalid
  routeView = builder.GetRouteView("::", 8080, "example.com", "");
  REQUIRE(routeView == nullptr);

  // unknown ip: invalid
  routeView = builder.GetRouteView("128.0.0.64", 8080, "example.com", "");
  REQUIRE(routeView == nullptr);

  // ip address invalid: invalid
  routeView = builder.GetRouteView("", 8081, "example.com", "");
  REQUIRE(routeView == nullptr);

  // ip address invalid: invalid
  routeView = builder.GetRouteView("lkajsdfopiuaewrl;h", 8081, "example.com", "");
  REQUIRE(routeView == nullptr);

  // unknown port num: invalid
  routeView = builder.GetRouteView("::", 8081, "example.com", "");
  REQUIRE(routeView == nullptr);

  // invalid port num: invalid
  routeView = builder.GetRouteView("::", -8081, "example.com", "");
  REQUIRE(routeView == nullptr);
  
  // location parameter and request URI identical: valid
  routeView = builder.GetRouteView("::", 8080, "ex0.com", "/tessst/test/test");
  REQUIRE(routeView->locationPrefix == "/tessst/test/test");

  // location parameter and request URI identical, both ending in '/': valid
  routeView = builder.GetRouteView("::", 8080, "ex0.com", "/test/");
  REQUIRE(routeView->locationPrefix == "/test/");

  // location parameter and request URI identical, neither ending in '/': valid
  routeView = builder.GetRouteView("::", 8080,"ex0.com", "/test");
  REQUIRE(routeView->locationPrefix == "/test");

  // location parameter is prefix of request URI, location parameter ending in '/': valid
  routeView = builder.GetRouteView("::", 8080, "ex0.com", "/test/test");
  REQUIRE(routeView->locationPrefix == "/test/");

  // location parameter is prefix and path segment of request URI: valid
  routeView = builder.GetRouteView("127.0.0.35", 8084, "example1.com", "/tessst1/test/test/test/test");
  REQUIRE(routeView->locationPrefix == "/tessst1/test/test");

  // minimal match location parameter and request URI (location param is one char): '/': valid
  routeView = builder.GetRouteView("127.0.0.35", 8084, "ex1.com", "/abc");
  REQUIRE(routeView->locationPrefix == "/");

  // minimal match location parameter and request URI (both one char): '/': valid
  routeView = builder.GetRouteView("127.0.0.35", 8084, "ex1.com", "/");
  REQUIRE(routeView->locationPrefix == "/");

  // minimal match location parameter and request URI (one char): 't': valid
  routeView = builder.GetRouteView("127.0.0.35", 8084, "ex1.com", "t/test");
  REQUIRE(routeView->locationPrefix == "t");

  // minimal match location parameter and request URI (both one char): 't': valid
  routeView = builder.GetRouteView("127.0.0.35", 8084, "ex1.com", "t");
  REQUIRE(routeView->locationPrefix == "t");

  // anonymous server, match in first anonymous server: valid
  routeView = builder.GetRouteView("::", 80, "", "/test2");
  REQUIRE(routeView->locationPrefix == "/test2");

  // anonymous server, match in second anonymous server: valid
  routeView = builder.GetRouteView("::", 80, "", "/test3");
  REQUIRE(routeView->locationPrefix == "/test3");

  // anonymous server, invalid target
  routeView = builder.GetRouteView("::", 80, "", "/invalid");
  REQUIRE(routeView == nullptr);

  // no location block in server, target == default location prefix
  routeView = builder.GetRouteView("::", 80, "example2.com", "/");
  REQUIRE(routeView->locationPrefix == "/");

  // two homonymous servers, match in second server: valid
  routeView = builder.GetRouteView("::", 80, "example2.com", "/not-default");
  REQUIRE(routeView->locationPrefix == "/not-default");

  // no location block in server, target != default location prefix: invalid
  routeView = builder.GetRouteView("::", 80, "example2.com", "error");
  REQUIRE(routeView == nullptr);
}

// INDIVIDUAL DIRECTIVES

TEST_CASE("listen", "[Builder]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080;\n"
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  Builder builder(lexer, parser, validatorIpPort);
  builder.Build();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(builder.GetServerCount() == 2);

  const ServerView& serverView0(builder.GetServerView(0));
  const ServerView& serverView1(builder.GetServerView(1));

  REQUIRE(serverView0.ipPortList.at(0).first == "::");
  REQUIRE(serverView0.ipPortList.at(0).second == 8080);
  REQUIRE(serverView0.ipPortList.at(1).first == "::");
  REQUIRE(serverView0.ipPortList.at(1).second == 9090);
  REQUIRE(serverView0.ipPortList.at(2).first == "120.0.0.64");
  REQUIRE(serverView0.ipPortList.at(2).second == 8081);
  REQUIRE(serverView0.ipPortList.at(3).first == "120.0.0.64");
  REQUIRE(serverView0.ipPortList.at(3).second == 80);
  REQUIRE(serverView0.ipPortList.at(4).first == "120::120");
  REQUIRE(serverView0.ipPortList.at(4).second == 8082);
  REQUIRE(serverView0.ipPortList.at(5).first == "120::120");
  REQUIRE(serverView0.ipPortList.at(5).second == 80);
  REQUIRE(serverView1.ipPortList.at(0).first == "::");
  REQUIRE(serverView1.ipPortList.at(0).second == 80);
}

TEST_CASE("listen invalid 1 - duplicates within one server and different servers", "[Builder]")
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
                    "    listen [0120::ABC]:8082;\n"
                    "    listen [120::0001];\n"
                    "    server_name example0.com;\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 120.0.0.64:8081;\n"
                    "    server_name example1.com;\n"
                    "  }\n"
                    "\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  Builder builder(lexer, parser, validatorIpPort);
  builder.Build();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == true);

  REQUIRE(builder.GetServerCount() == 2);

  const ServerView& serverView0(builder.GetServerView(0));
  const ServerView& serverView1(builder.GetServerView(1));

  REQUIRE(serverView0.ipPortList.at(0).first == "120::abc");
  REQUIRE(serverView0.ipPortList.at(0).second == 8082);
  REQUIRE(serverView0.ipPortList.at(1).first == "::");
  REQUIRE(serverView0.ipPortList.at(1).second == 8080);
  REQUIRE(serverView0.ipPortList.at(2).first == "::");
  REQUIRE(serverView0.ipPortList.at(2).second == 9090);
  REQUIRE(serverView0.ipPortList.at(3).first == "120.0.0.64");
  REQUIRE(serverView0.ipPortList.at(3).second == 8081);
  REQUIRE(serverView0.ipPortList.at(4).first == "120.0.0.64");
  REQUIRE(serverView0.ipPortList.at(4).second == 80);
  REQUIRE(serverView0.ipPortList.at(5).first == "120::abc");
  REQUIRE(serverView0.ipPortList.at(5).second == 8082);
  REQUIRE(serverView0.ipPortList.at(6).first == "120::1");
  REQUIRE(serverView0.ipPortList.at(6).second == 80);
  REQUIRE(serverView1.ipPortList.at(0).first == "120.0.0.64");
  REQUIRE(serverView1.ipPortList.at(0).second == 8081);

  std::set<std::size_t> errorsIdx{20,32};
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

TEST_CASE("listen invalid 2 - ipv6 normalization", "[Builder]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen [::]:8082;\n"
                    "    server_name example0.com;\n"
                    "  }\n"
                    "  server {\n"
                    "    listen [::]:8082;\n"
                    "    server_name example1.com;\n"
                    "  }\n"
                    "  server {\n"
                    "    listen [120:120:120:120::120]:8082;\n"
                    "    server_name example2.com;\n"
                    "  }\n"
                    "  server {\n"
                    "    listen [120:120:120:120:0:0:0:120]:8082;\n"
                    "    server_name example3.com;\n"
                    "  }\n"
                    "  server {\n"
                    "    listen [120:120:120:120:120:120::120]:8082;\n"
                    "    server_name example4.com;\n"
                    "  }\n"
                    "  server {\n"
                    "    listen [120:120:120:120:120:120:0:120]:8082;\n"
                    "    server_name example5.com;\n"
                    "  }\n"
                    "  server {\n"
                    "    listen [120:120:120:120:120:120::]:8082;\n"
                    "    server_name example6.com;\n"
                    "  }\n"
                    "  server {\n"
                    "    listen [120:120:120:120:120:120:0:0]:8082;\n"
                    "    server_name example7.com;\n"
                    "  }\n"
                    "  server {\n"
                    "    listen [120:120:120:120:120:120:120:0]:8082;\n"
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  Builder builder(lexer, parser, validatorIpPort);
  builder.Build();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == true);

  REQUIRE(builder.GetServerCount() == 10);

  const ServerView& serverView0(builder.GetServerView(0));
  const ServerView& serverView1(builder.GetServerView(1));
  const ServerView& serverView2(builder.GetServerView(2));
  const ServerView& serverView3(builder.GetServerView(3));
  const ServerView& serverView4(builder.GetServerView(4));
  const ServerView& serverView5(builder.GetServerView(5));
  const ServerView& serverView6(builder.GetServerView(6));
  const ServerView& serverView7(builder.GetServerView(7));
  const ServerView& serverView8(builder.GetServerView(8));
  const ServerView& serverView9(builder.GetServerView(9));

  REQUIRE(serverView0.ipPortList.at(0).first == "::");
  REQUIRE(serverView0.ipPortList.at(0).second == 8082);
  REQUIRE(serverView1.ipPortList.at(0).first == "::");
  REQUIRE(serverView1.ipPortList.at(0).second == 8082);
  REQUIRE(serverView2.ipPortList.at(0).first == "120:120:120:120::120");
  REQUIRE(serverView2.ipPortList.at(0).second == 8082);
  REQUIRE(serverView3.ipPortList.at(0).first == "120:120:120:120::120");
  REQUIRE(serverView3.ipPortList.at(0).second == 8082);
  REQUIRE(serverView4.ipPortList.at(0).first == "120:120:120:120:120:120:0:120");
  REQUIRE(serverView4.ipPortList.at(0).second == 8082);
  REQUIRE(serverView5.ipPortList.at(0).first == "120:120:120:120:120:120:0:120");
  REQUIRE(serverView5.ipPortList.at(0).second == 8082);
  REQUIRE(serverView6.ipPortList.at(0).first == "120:120:120:120:120:120::");
  REQUIRE(serverView6.ipPortList.at(0).second == 8082);
  REQUIRE(serverView7.ipPortList.at(0).first == "120:120:120:120:120:120::");
  REQUIRE(serverView7.ipPortList.at(0).second == 8082);
  REQUIRE(serverView8.ipPortList.at(0).first == "120:120:120:120:120:120:120:0");
  REQUIRE(serverView8.ipPortList.at(0).second == 8082);
  REQUIRE(serverView9.ipPortList.at(0).first == "0:120:120:120:120:120:120:120");
  REQUIRE(serverView9.ipPortList.at(0).second == 8082);

  std::set<std::size_t> errorsIdx{14,32,50, 68};
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

TEST_CASE("listen invalid 3", "[Builder]")
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  Builder builder(lexer, parser, validatorIpPort);
  builder.Build();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == true);

  REQUIRE(builder.GetServerCount() == 2);

  const ServerView& serverView0(builder.GetServerView(0));
  const ServerView& serverView1(builder.GetServerView(1));

  REQUIRE(serverView0.ipPortList.at(0).first == "120.0.0.120");
  REQUIRE(serverView0.ipPortList.at(0).second == 8081);
  REQUIRE(serverView0.ipPortList.at(1).first == "120::120");
  REQUIRE(serverView0.ipPortList.at(1).second == 8082);
  REQUIRE(serverView0.ipPortList.at(2).first == "120:120::120");
  REQUIRE(serverView0.ipPortList.at(2).second == 8082);
  REQUIRE(serverView0.ipPortList.at(3).first == "::");
  REQUIRE(serverView0.ipPortList.at(3).second == 8082);
  REQUIRE(serverView0.ipPortList.at(4).first == "0:0:0:120::");
  REQUIRE(serverView0.ipPortList.at(4).second == 8082);
  REQUIRE(serverView0.ipPortList.at(5).first == "::120:0:0:0");
  REQUIRE(serverView0.ipPortList.at(5).second == 8082);
  REQUIRE(serverView0.ipPortList.at(6).first == "::120");
  REQUIRE(serverView0.ipPortList.at(6).second == 8082);
  REQUIRE(serverView0.ipPortList.at(7).first == "::");
  REQUIRE(serverView0.ipPortList.at(7).second == 80);
  REQUIRE(serverView0.ipPortList.at(8).first == "abc::");
  REQUIRE(serverView0.ipPortList.at(8).second == 8082);
  REQUIRE(serverView0.ipPortList.at(9).first == "abc:deff:a:a000::");
  REQUIRE(serverView0.ipPortList.at(9).second == 8082);
  REQUIRE(serverView0.ipPortList.at(10).first == "abc:0:0:a000::f");
  REQUIRE(serverView0.ipPortList.at(10).second == 8082);
  REQUIRE(serverView0.ipPortList.at(11).first == "abc::a000:0:0:f");
  REQUIRE(serverView0.ipPortList.at(11).second == 8082);

  REQUIRE(serverView1.ipPortList.at(0).first == "120.0.0.120");
  REQUIRE(serverView1.ipPortList.at(0).second == 8081);
  REQUIRE(serverView1.ipPortList.at(1).first == "120::120");
  REQUIRE(serverView1.ipPortList.at(1).second == 8082);
  REQUIRE(serverView1.ipPortList.at(2).first == "120:120::120");
  REQUIRE(serverView1.ipPortList.at(2).second == 8082);
  REQUIRE(serverView1.ipPortList.at(3).first == "::");
  REQUIRE(serverView1.ipPortList.at(3).second == 8082);
  REQUIRE(serverView1.ipPortList.at(4).first == "0:0:0:120::");
  REQUIRE(serverView1.ipPortList.at(4).second == 8082);
  REQUIRE(serverView1.ipPortList.at(5).first == "::120:0:0:0");
  REQUIRE(serverView1.ipPortList.at(5).second == 8082);
  REQUIRE(serverView1.ipPortList.at(6).first == "::120");
  REQUIRE(serverView1.ipPortList.at(6).second == 8082);
  REQUIRE(serverView1.ipPortList.at(7).first == "::");
  REQUIRE(serverView1.ipPortList.at(7).second == 80);
  REQUIRE(serverView1.ipPortList.at(8).first == "abc::");
  REQUIRE(serverView1.ipPortList.at(8).second == 8082);
  REQUIRE(serverView1.ipPortList.at(9).first == "abc:deff:a:a000::");
  REQUIRE(serverView1.ipPortList.at(9).second == 8082);
  REQUIRE(serverView1.ipPortList.at(10).first == "abc:0:0:a000::f");
  REQUIRE(serverView1.ipPortList.at(10).second == 8082);
  REQUIRE(serverView1.ipPortList.at(11).first == "abc::a000:0:0:f");
  REQUIRE(serverView1.ipPortList.at(11).second == 8082);

  std::set<std::size_t> errorsIdx{47, 50, 53, 56, 59, 62, 65, 68, 71, 74, 77, 80};
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

TEST_CASE("server_name", "[Builder]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    server_name name0 name1 name2;\n"
                    "  }\n"
                    "  server {\n"
                    "    server_name name3 name4 name5;\n"
                    "  }\n"
                    "  server {\n"
                    "  }\n"
                    "\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  Builder builder(lexer, parser, validatorIpPort);
  builder.Build();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);


  REQUIRE(builder.GetServerCount() == 3);

  const ServerView& serverView0(builder.GetServerView(0));
  const ServerView& serverView1(builder.GetServerView(1));
  const ServerView& serverView2(builder.GetServerView(2));

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

TEST_CASE("root", "[Builder]")
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  Builder builder(lexer, parser, validatorIpPort);
  builder.Build();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(builder.GetServerCount() == 2);

  const ServerView& serverView0(builder.GetServerView(0));
  const ServerView& serverView1(builder.GetServerView(1));

  REQUIRE(serverView0.routes.at(0).locationPrefix == "locPref00");
  REQUIRE(serverView0.routes.at(0).root == "serverRoot0");
  REQUIRE(serverView0.routes.at(1).locationPrefix == "locPref01");
  REQUIRE(serverView0.routes.at(1).root == "locationRoot01");
  REQUIRE(serverView1.routes.at(0).locationPrefix == "locPref1");
  REQUIRE(serverView1.routes.at(0).root == "locationRoot1");
}

TEST_CASE("index", "[Builder]")
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  Builder builder(lexer, parser, validatorIpPort);
  builder.Build();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(builder.GetServerCount() == 2);

  const ServerView& serverView0(builder.GetServerView(0));
  const ServerView& serverView1(builder.GetServerView(1));

  REQUIRE(serverView0.routes.at(0).locationPrefix == "locPref00");
  REQUIRE(serverView0.routes.at(0).index == "serverIndex0");
  REQUIRE(serverView0.routes.at(1).locationPrefix == "locPref01");
  REQUIRE(serverView0.routes.at(1).index == "locationIndex01");
  REQUIRE(serverView1.routes.at(0).locationPrefix == "locPref1");
  REQUIRE(serverView1.routes.at(0).index == "httpIndex");
}

TEST_CASE("alias", "[Builder]")
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  Builder builder(lexer, parser, validatorIpPort);
  builder.Build();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(builder.GetServerCount() == 1);

  const ServerView& serverView0(builder.GetServerView(0));

  REQUIRE(serverView0.routes.at(0).locationPrefix == "locPref00");
  REQUIRE(serverView0.routes.at(0).alias.has_value() == false);
  REQUIRE(serverView0.routes.at(1).locationPrefix == "locPref01");
  REQUIRE(serverView0.routes.at(1).alias == "locationAlias01");
}

TEST_CASE("client_max_body_size", "[Builder]")
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  Builder builder(lexer, parser, validatorIpPort);
  builder.Build();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(builder.GetServerCount() == 2);

  const ServerView& serverView0(builder.GetServerView(0));
  const ServerView& serverView1(builder.GetServerView(1));

  REQUIRE(serverView0.routes.at(0).locationPrefix == "locPref00");
  REQUIRE(serverView0.routes.at(0).clientMaxBody == 32 * 1024);
  REQUIRE(serverView0.routes.at(1).locationPrefix == "locPref01");
  REQUIRE(serverView0.routes.at(1).clientMaxBody == 1 * 1024 * 1024 * 1024);
  REQUIRE(serverView1.routes.at(0).locationPrefix == "locPref1");
  REQUIRE(serverView1.routes.at(0).clientMaxBody == 8 * 1024 * 1024);
}

TEST_CASE("error_page", "[Builder]")
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  Builder builder(lexer, parser, validatorIpPort);
  builder.Build();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(builder.GetServerCount() == 2);

  const ServerView& serverView0(builder.GetServerView(0));
  const ServerView& serverView1(builder.GetServerView(1));

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

TEST_CASE("return", "[Builder]")
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  Builder builder(lexer, parser, validatorIpPort);
  builder.Build();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(builder.GetServerCount() == 2);

  const ServerView& serverView0(builder.GetServerView(0));
  const ServerView& serverView1(builder.GetServerView(1));

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

TEST_CASE("allowed_methods", "[Builder]")
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  Builder builder(lexer, parser, validatorIpPort);
  builder.Build();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(builder.GetServerCount() == 2);

  const ServerView& serverView0(builder.GetServerView(0));
  const ServerView& serverView1(builder.GetServerView(1));

  REQUIRE(serverView0.routes.at(0).locationPrefix == "locPref00");
  REQUIRE(serverView0.routes.at(0).allowedMask == RouteView::MethodMask::Post);
  REQUIRE(serverView0.routes.at(1).locationPrefix == "locPref01");
  REQUIRE(serverView0.routes.at(1).allowedMask == RouteView::MethodMask::Delete);
  REQUIRE(serverView1.routes.at(0).locationPrefix == "locPref10");
  REQUIRE(serverView1.routes.at(0).allowedMask == (RouteView::MethodMask::Get | RouteView::MethodMask::Post | RouteView::MethodMask::Delete));
  REQUIRE(serverView1.routes.at(1).locationPrefix == "locPref11");
  REQUIRE(serverView1.routes.at(1).allowedMask == RouteView::MethodMask::Get);
}

TEST_CASE("autoindex http default (off)", "[Builder]")
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  Builder builder(lexer, parser, validatorIpPort);
  builder.Build();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(builder.GetServerCount() == 2);

  const ServerView& serverView0(builder.GetServerView(0));
  const ServerView& serverView1(builder.GetServerView(1));

  REQUIRE(serverView0.routes.at(0).locationPrefix == "locPref00");
  REQUIRE(serverView0.routes.at(0).autoindex == true);
  REQUIRE(serverView0.routes.at(1).locationPrefix == "locPref01");
  REQUIRE(serverView0.routes.at(1).autoindex == false);
  REQUIRE(serverView1.routes.at(0).locationPrefix == "locPref10");
  REQUIRE(serverView1.routes.at(0).autoindex == false);
  REQUIRE(serverView1.routes.at(1).locationPrefix == "locPref11");
  REQUIRE(serverView1.routes.at(1).autoindex == true);
}

TEST_CASE("autoindex http on", "[Builder]")
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  Builder builder(lexer, parser, validatorIpPort);
  builder.Build();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(builder.GetServerCount() == 2);

  const ServerView& serverView0(builder.GetServerView(0));
  const ServerView& serverView1(builder.GetServerView(1));

  REQUIRE(serverView0.routes.at(0).locationPrefix == "locPref00");
  REQUIRE(serverView0.routes.at(0).autoindex == true);
  REQUIRE(serverView0.routes.at(1).locationPrefix == "locPref01");
  REQUIRE(serverView0.routes.at(1).autoindex == false);
  REQUIRE(serverView1.routes.at(0).locationPrefix == "locPref10");
  REQUIRE(serverView1.routes.at(0).autoindex == false);
  REQUIRE(serverView1.routes.at(1).locationPrefix == "locPref11");
  REQUIRE(serverView1.routes.at(1).autoindex == true);
}

TEST_CASE("cgi", "[Builder]")
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  Builder builder(lexer, parser, validatorIpPort);
  builder.Build();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(builder.GetServerCount() == 1);

  const ServerView& serverView0(builder.GetServerView(0));

  REQUIRE(serverView0.routes.at(0).locationPrefix == "locPref00");
  REQUIRE(serverView0.routes.at(0).cgi == false);
  REQUIRE(serverView0.routes.at(1).locationPrefix == "locPref01");
  REQUIRE(serverView0.routes.at(1).cgi == true);
}

TEST_CASE("cgi_extension", "[Builder]")
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  Builder builder(lexer, parser, validatorIpPort);
  builder.Build();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(builder.GetServerCount() == 2);

  const ServerView& serverView0(builder.GetServerView(0));
  const ServerView& serverView1(builder.GetServerView(1));

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

TEST_CASE("one server, four locations", "[Builder]")
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  Builder builder(lexer, parser, validatorIpPort);
  builder.Build();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);

  REQUIRE(builder.GetServerCount() == 1);

  const ServerView& serverView = builder.GetServerView(0);

  REQUIRE(serverView.hostNames[0] == "example.com");
  REQUIRE(serverView.ipPortList.size() == 1);
  REQUIRE(serverView.ipPortList.at(0).first == "::");
  REQUIRE(serverView.ipPortList.at(0).second == static_cast<std::uint16_t>(8080));
  REQUIRE(serverView.routes.size() == 4);

  const RouteView& routeView0 = serverView.routes[0];
  REQUIRE(routeView0.locationPrefix == "/");
  REQUIRE(routeView0.root == std::filesystem::path("./www"));
  REQUIRE(routeView0.alias.has_value() == false);
  REQUIRE(routeView0.index == "indexLocation.html");
  REQUIRE(routeView0.autoindex == false);
  REQUIRE(routeView0.clientMaxBody == std::size_t(2 * 1024 * 1024));
  REQUIRE(routeView0.allowedMask == RouteView::MethodMask::Get);
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
  REQUIRE(routeView1.allowedMask == (RouteView::MethodMask::Post | RouteView::MethodMask::Delete));
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
  REQUIRE(routeView2.allowedMask == (RouteView::MethodMask::Get));
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
  REQUIRE(routeView3.allowedMask == (RouteView::MethodMask::Get | RouteView::MethodMask::Post));
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