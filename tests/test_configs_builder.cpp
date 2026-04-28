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

TEST_CASE("GetError() -> false", "[ServerRegistry]")
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
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == true);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);
}

TEST_CASE("GetError() -> true: duplicate IP:port hostname combinations", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name example0.com;\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name example0.com;\n"
                    "  }\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == true);
}

TEST_CASE("GetError() -> true: duplicate hostnames", "[ServerRegistry]")
{
  std::string raw = "\n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name example0.com example0.com;\n"
                    "  }\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == true);
}

TEST_CASE("GetError() -> true - duplicate 'default_server'", "[Builder]")
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

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  Builder builder(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == true);
}

TEST_CASE("GetError() -> true - duplicate location Prefix in server block", "[Builder]")
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
                    "    location /////////////////////////server0path0 {\n"
                    "      root server0root1;\n"
                    "    }\n"
                    "    location //./////////./////././././/////server0path0 {\n"
                    "      root server0root1;\n"
                    "    }\n"
                    "\n"
                    "  }\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name server1;\n"
                    "    location /server1path0/ {\n"
                    "      root server1root0;\n"
                    "    }\n"
                    "    location /server1path0/server1path0/server1path0/../.. {\n"
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
                    "      location /server2path0{\n"
                    "        root server2root3;\n"
                    "      }\n"
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
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == true);

  std::set<std::size_t> errorsIdx{18, 25, 48, 85};
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

TEST_CASE("builder.BuildServerRegistry() moves data out of builder (no deep copy)", "[ServerRegistry]")
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

  const void* addressServersBeforeMove = static_cast<const void*>(builder.GetServersData());
  const void* addressMapValueBeforeMove = static_cast<const void*>(builder.GetAddressValue({"::", "8080"}));
  ServerRegistry serverRegistry(builder.BuildServerRegistry());
  const void* addressServersAfterMove = static_cast<const void*>(serverRegistry.GetServersData());
  const void* addressMapValueAfterMove = static_cast<const void*>(serverRegistry.GetAddressValue({"::", "8080"}));

  REQUIRE(addressServersBeforeMove == addressServersAfterMove);
  REQUIRE(addressMapValueBeforeMove == addressMapValueAfterMove);
  REQUIRE(serverRegistry.GetServerViewCount() == 3);
}
