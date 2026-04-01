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
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == true);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == false);
}


TEST_CASE("GetError() -> true", "[ServerRegistry]")
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
                    "    listen 8081;\n"
                    "    listen 8081;\n"
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
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);
  REQUIRE(builder.GetError() == true);
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