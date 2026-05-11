#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

#include "catch_amalgamated.hpp"
#include "config/Lexer.hpp"
#include "config/Parser.hpp"
#include "config/Validator.hpp"
#include "config/ValidatorIpPort.hpp"

// Test: empty input

TEST_CASE("empty input", "[Validator]")
{
  std::string raw = "";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetTokenError(0) == true);
  REQUIRE(lexer.GetTokenErrorMessage(0).empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == true);
}

// Test: whitespace

TEST_CASE("whitespace", "[Validator]")
{
  std::string raw = "  \t\t\n\n\v\v\f\f\r\r";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetTokenError(0) == true);
  REQUIRE(lexer.GetTokenErrorMessage(0).empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == true);
}

// Test: comments

TEST_CASE("comments", "[Validator]")
{
  std::string raw = "#this\n#is\n#a\n#config file";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetTokenError(0) == true);
  REQUIRE(lexer.GetTokenErrorMessage(0).empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == true);
}

TEST_CASE("http", "[validator]")
{
  std::string raw = "http";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == true);
  REQUIRE(validator.GetError() == true);
  REQUIRE(lexer.GetTokenError(0) == false);
  REQUIRE(lexer.GetTokenErrorMessage(0).empty() == true);
  REQUIRE(lexer.GetTokenError(1) == true);
  REQUIRE(lexer.GetTokenErrorMessage(1).empty() == false);
}

TEST_CASE("location", "[validator]")
{
  std::string raw = "location";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == true);
  REQUIRE(validator.GetError() == true);
  REQUIRE(lexer.GetTokenError(0) == true);
  REQUIRE(lexer.GetTokenErrorMessage(0).empty() == false);
  REQUIRE(lexer.GetTokenError(1) == true);
  REQUIRE(lexer.GetTokenErrorMessage(1).empty() == false);
}

TEST_CASE("server", "[validator]")
{
  std::string raw = "server";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == true);
  REQUIRE(validator.GetError() == true);
  REQUIRE(lexer.GetTokenError(0) == true);
  REQUIRE(lexer.GetTokenErrorMessage(0).empty() == false);
  REQUIRE(lexer.GetTokenError(1) == true);
  REQUIRE(lexer.GetTokenErrorMessage(1).empty() == false);
}

TEST_CASE("LBrace", "[validator]")
{
  std::string raw = "{";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == true);
  REQUIRE(validator.GetError() == true);
  REQUIRE(lexer.GetTokenError(0) == true);
  REQUIRE(lexer.GetTokenErrorMessage(0).empty() == false);
  REQUIRE(lexer.GetTokenError(1) == true);
  REQUIRE(lexer.GetTokenErrorMessage(1).empty() == false);
}

TEST_CASE("RBrace", "[validator]")
{
  std::string raw = "}";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == true);
  REQUIRE(validator.GetError() == true);
  REQUIRE(lexer.GetTokenError(0) == true);
  REQUIRE(lexer.GetTokenErrorMessage(0).empty() == false);
  REQUIRE(lexer.GetTokenError(1) == true);
  REQUIRE(lexer.GetTokenErrorMessage(1).empty() == false);
}

TEST_CASE("semicolon", "[validator]")
{
  std::string raw = ";";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == true);
  REQUIRE(validator.GetError() == true);
  REQUIRE(lexer.GetTokenError(0) == true);
  REQUIRE(lexer.GetTokenErrorMessage(0).empty() == false);
  REQUIRE(lexer.GetTokenError(1) == true);
  REQUIRE(lexer.GetTokenErrorMessage(1).empty() == false);
}

TEST_CASE("non-printable character", "[validator]")
{
  std::string raw = "";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);
  
  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == true);
  REQUIRE(parser.GetError() == true);
  REQUIRE(validator.GetError() == true);
  REQUIRE(lexer.GetTokenError(0) == true);
  REQUIRE(lexer.GetTokenErrorMessage(0).empty() == false);
  REQUIRE(lexer.GetTokenError(1) == true);
  REQUIRE(lexer.GetTokenErrorMessage(1).empty() == false);
}

TEST_CASE("string", "[validator]")
{
  std::string raw = "string";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == true);
  REQUIRE(validator.GetError() == true);
  REQUIRE(lexer.GetTokenError(0) == true);
  REQUIRE(lexer.GetTokenErrorMessage(0).empty() == false);
  REQUIRE(lexer.GetTokenError(1) == true);
  REQUIRE(lexer.GetTokenErrorMessage(1).empty() == false);
}

TEST_CASE("missing LBrace", "[validator]")
{
  std::string raw = "http index Wait, where's the left brace?};";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  lexer.PrintErrorMessages();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == true);
  REQUIRE(validator.GetError() == true);

  std::set<std::size_t> errorsIdx{1,2,3,4,5,6,7,8,9};
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

TEST_CASE("missing RBrace", "[validator]")
{
  std::string raw = "http { index Wait, where's the right brace?;";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == true);
  REQUIRE(validator.GetError() == true);

  std::set<std::size_t> errorsIdx{9};
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

TEST_CASE("missing semicolon", "[validator]")
{
  std::string raw = "http { client_max_body_size Wait, where's the semicolon? }";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == true);
  REQUIRE(validator.GetError() == true);
  
  std::set<std::size_t> errorsIdx{3,4,5,6,7,8};
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

TEST_CASE("all token kinds once", "[validator]")
{
  std::string raw = "string 1234;{}#this is not a location token\n"
                    "location http server listen server_name  root   index    return #this is  not   an   alias    token      \n"
                    "     alias client_max_body_size\n"
                    "error_page\n"
                    "allowed_methods client_max_body_size  autoindex\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  REQUIRE(lexer.GetError() == false);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == true);
  REQUIRE(validator.GetError() == true);
  
  std::set<std::size_t> errorsIdx{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
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

// Test: correct config tree

TEST_CASE("validator.Validate prints no errors for correct config tree", "[Validator]")
{
  std::string raw = "http {"
                    "    client_max_body_size 10m;"
                    ""
                    "    error_page 404 /errors/404.html;"
                    "    error_page 500 502 503 504 /errors/50x.html;"
                    ""
                    "    server {"
                    "        listen 80;"
                    "        listen 443;"
                    "        server_name example.com www.example.com;"
                    ""
                    "        root /var/www/example;"
                    ""
                    "        error_page 404 /errors/404.html;"
                    "        error_page 500 502 503 504 /errors/50x.html;"
                    "        "
                    "        location /errors/404.html {"
                    ""
                    "        }"
                    "        "
                    "        location /errors/50x.html {"
                    "            "
                    "        }"
                    ""
                    "        location /api {"
                    "            allowed_methods GET POST DELETE;"
                    "            root /var/www/example;"
                    "        }"
                    ""
                    "        location /old-page {"
                    "            return 301 /new-page;"
                    "        }"
                    ""
                    "        location /downloads {"
                    "            root /var/www/example;"
                    "        }"
                    ""
                    "        location /private {"
                    "            root /var/www/example;"
                    "        }"
                    ""
                    "        location /docs {"
                    "            root /var/www/example;"
                    "            index index.html index.htm default.html;"
                    "        }"
                    ""
                    "        location /upload {"
                    "            allowed_methods POST;"
                    "            "
                    "            root /var/www/uploads;"
                    "        }"
                    "    }"
                    ""
                    "    server {"
                    "        listen 8080;"
                    "        listen 8443;"
                    "        server_name blog.example.com;"
                    ""
                    "        root /var/www/blog;"
                    ""
                    "        error_page 404 /404.html;"
                    "        error_page 500 /500.html;"
                    ""
                    "        index index.html index.htm;"
                    ""
                    "        allowed_methods GET POST;"
                    ""
                    "        location /admin {"
                    "            root /var/www/blog;"
                    "        }"
                    "    }"
                    ""
                    "    server {"
                    "        listen 3000;"
                    "        server_name static.example.com;"
                    ""
                    "        root /var/www/static;"
                    ""
                    "        client_max_body_size 50m;"
                    ""
                    "        location / {"
                    "            allowed_methods GET;"
                    "        }"
                    "    }"
                    ""
                    "    server {"
                    "        listen 5000;"
                    "        server_name api.example.com;"
                    ""
                    "        error_page 404 /api/errors/not_found.json;"
                    "        error_page 500 /api/errors/server_error.json;"
                    ""
                    "        location /api/v1 {"
                    "            root /var/www/api;"
                    "            allowed_methods DELETE GET POST;"
                    "        }"
                    ""
                    "        location /api/v0 {"
                    "            return 302;"
                    "        }"
                    "    }"
                    "}";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty());
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == false);

  for (std::size_t i = 0; i < lexer.GetSizeTokenList(); ++i)
  {
    CAPTURE(i);
    REQUIRE(lexer.GetTokenError(i) == false);
    REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  }
}

// Test: nesting

TEST_CASE("errors incorrectly nested blocks", "[Validator]")
{
  std::string raw = "# incorrectly nested http\n"
                    "\n"
                    "\n"
                    "http{http{}}\n"
                    "\n"
                    "http{server{http{}}}\n"
                    "\n"
                    "http{server{location /documents/{http{}}}}\n"
                    "\n"
                    "# incorrectly nested server\n"
                    "\n"
                    "server{}\n"
                    "\n"
                    "\n"
                    "http{server{server{}}}\n"
                    "\n"
                    "http{location /documents/{server{}}}\n"
                    "\n"
                    "http{server{location /documents/{server{}}}}\n"
                    "\n"
                    "# incorrectly nested location /documents/\n"
                    "\n"
                    "location /documents/{}\n"
                    "\n"
                    "location /documents/{location /documents/{location /documents/{}}}\n"
                    "\n"
                    "\n"
                    "http{server{location /documents/{location /documents/{}}}}\n"
                    "\n"
                    "http{location /documents/{location /documents/{}}}\n"
                    "\n"
                    "# multiple recursively incorrectly nested blocks\n"
                    "\n"
                    "http {server {location /documents/ {http{\n"
                    "        \n"
                    "\n"
                    "\n"
                    "\n"
                    "\n"
                    "\n"
                    "\n"
                    "\n"
                    "\n"
                    "\n"
                    "\n"
                    "\n"
                    "\n"
                    "\n"
                    "\n"
                    "\n"
                    "        server {\n"
                    "          location path {\n"
                    "            location /documents/ {\n"
                    "              server {\n"
                    "                  http {\n"
                    "                      http {\n"
                    "                          location /documents/ /path {\n"
                    "                            server {\n"
                    "                              http {\n"
                    "                                server {\n"
                    "                                  location /documents/ {\n"
                    "\n"
                    "                                  }\n"
                    "                                }\n"
                    "                              }\n"
                    "                            }\n"
                    "                          }\n"
                    "                        }\n"
                    "                      }\n"
                    "                    }\n"
                    "                  }\n"
                    "                }\n"
                    "              }\n"
                    "            }\n"
                    "          }\n"
                    "        }\n"
                    "      }\n"
                    "    }\n"
                    "  }\n"
                    "}\n"
                    "\n"
                    "\n"
                    "\n"
                    "\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(validator.GetError() == true);
  std::set<std::size_t> errorsIdx{2, 4, 5, 6, 10, 12, 15, 22, 24, 28, 31, 35, 40, 42, 45, 49, 50, 57, 63, 67, 70, 73, 79, 86, 93, 95, 98, 103, 104, 111, 118, 121, 123, 125, 127, 129, 131, 133, 138, 145, 146, 155, 156, 157};
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

TEST_CASE("errors incorrectly nested dirs", "[Validator]")
{
  std::string raw = "listen 8080;\n"
                    "server_name name;\n"
                    "root /path;\n"
                    "index file;\n"
                    "alias /path;\n"
                    "client_max_body_size 1G;\n"
                    "error_page 401 http://path;\n"
                    "allowed_methods DELETE;\n"
                    "return 302;\n"
                    "\n"
                    "http {\n"
                    "\n"
                    "  listen 8080;\n"
                    "  server_name name;\n"
                    "  root /path;\n"
                    "  index file;\n"
                    "  alias /path;\n"
                    "  client_max_body_size 1G;\n"
                    "  error_page 401 http://path;\n"
                    "  allowed_methods DELETE;\n"
                    "  return 302;\n"
                    "\n"
                    "  server {\n"
                    "    listen 8080;\n"
                    "    server_name name;\n"
                    "    root /path;\n"
                    "    index file;\n"
                    "    alias /path;\n"
                    "    client_max_body_size 1G;\n"
                    "    error_page 401 http://path;\n"
                    "    allowed_methods DELETE;\n"
                    "    return 302;\n"
                    "    \n"
                    "    location /path/ {\n"
                    "      listen 8080;\n"
                    "      server_name name;\n"
                    "      root /path;\n"
                    "      index file;\n"
                    "      alias /path;\n"
                    "      client_max_body_size 1G;\n"
                    "      error_page 401 http://path;\n"
                    "      allowed_methods DELETE;\n"
                    "      return 302;\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  location /path/ {\n"
                    "    listen 8080;\n"
                    "    server_name name;\n"
                    "    root /path;\n"
                    "    index file;\n"
                    "    alias /path;\n"
                    "    client_max_body_size 1G;\n"
                    "    error_page 401 http://path;\n"
                    "    allowed_methods DELETE;\n"
                    "    return 302;\n"
                    "  }\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(validator.GetError() == true);
  std::set<std::size_t> errorsIdx{0, 3, 6, 9, 12, 15, 18, 22, 25, 30, 33, 36, 42, 52, 55, 72, 91, 94, 103, 121, 124, 127, 136};

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

// Test: individual directives

TEST_CASE("errors alias", "[Validator]")
{
  std::string raw = "http {"
                    "  server {"
                    "    location /path {"
                    ""
                    "      # valid\n"
                    ""
                    "      alias /path;"
                    ""
                    "      # invalid\n"
                    ""
                    "      alias path;"
                    "      alias /path;"
                    "      alias path /path;"
                    "      alias /path path;"
                    "    }"
                    "  }"
                    "}";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(lexer.GetError() == true);
  REQUIRE(output.empty() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == true);
  std::set<std::size_t> errorsIdx{10, 13, 14, 16, 18, 20, 22};
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

TEST_CASE("errors allowed_methods", "[Validator]")
{
  std::string raw = "http {"
                    "  server {"
                    "    location path {"
                    "      # valid\n"
                    "      allowed_methods GET POST DELETE;"
                    "      allowed_methods GET;"
                    "      allowed_methods POST;"
                    "      allowed_methods DELETE;"
                    "      allowed_methods DELETE POST GET;"
                    "      "
                    "      # invalid\n"
                    "      allowed_methods;"
                    "      allowed_methods GET POST delete DELETE;"
                    "      allowed_methods method;"
                    "      allowed_methods METHOD;"
                    "      allowed_methods POSt;"
                    "      allowed_methods GET POST DELETE GET POST DELETE;"
                    "      allowed_methods get post delete GET POST delete GET POST DELETE get post DELETE GET POST DELETE;"
                    "    }"
                    "  }"
                    "}";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);


  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == true);
  std::set<std::size_t> errorsIdx{12, 15, 18, 21, 26, 27, 28, 31, 34, 35, 37, 38, 40, 41, 43, 47, 48, 49, 51, 52, 53, 54, 57, 58, 59, 61, 62, 63, 64, 65, 66};
  for (size_t i = 0; i < lexer.GetSizeTokenList(); ++i)
  {
    if (errorsIdx.count(i) > 0)
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

TEST_CASE("errors autoindex", "[Validator]")
{
  std::string raw = "http {"
                    "  "
                    "  #valid\n"
                    ""
                    "  autoindex on;"
                    "  autoindex off;"
                    ""
                    "  server {"
                    ""
                    "    #valid\n"
                    ""
                    "    autoindex on;"
                    "    autoindex off;"
                    ""
                    "    location /path{"
                    ""
                    "      #valid\n"
                    ""
                    "      autoindex on;"
                    "      autoindex off;"
                    ""
                    "      # invalid\n"
                    ""
                    "      autoindex ;"
                    "      autoindex on off;"
                    "      autoindex off on on on ;"
                    "      autoindex huh?;"
                    "      autoindex on;"
                    "    }"
                    "  }"
                    "}";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);


  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == true);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == true);
  std::set<std::size_t> errorsIdx{5, 13, 22, 25, 26, 27, 29, 31, 33, 34, 35, 37, 38, 40, 41};

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

TEST_CASE("errors cgi", "[Validator]")
{
  std::string raw = "# invalid\n"
                    "\n"
                    "cgi ;\n"
                    "cgi off;\n"
                    "cgi on;\n"
                    "cgi error;\n"
                    "cgi on off;\n"
                    "cgi off on;\n"
                    "\n"
                    "http {"
                    "# valid \n"
                    "cgi on;\n"
                    "\n"
                    "  # invalid \n"
                    "\n"
                    "  cgi ; \n"
                    "  cgi error; \n"
                    "  cgi on off; \n"
                    "  cgi off on; \n"
                    "\n"
                    "  server { \n"
                    "\n"
                    "    # valid \n"
                    "\n"
                    "    cgi off; \n"
                    "\n"
                    "    # invalid \n"
                    "\n"
                    "    cgi ; \n"
                    "   cgi error; \n"
                    "    cgi on off; \n"
                    "    cgi off on; \n"
                    "\n"
                    "    location /path { \n"
                    "\n"
                    "     # valid\n"
                    "\n"
                    "     cgi on;\n"
                    "     cgi off;\n"
                    "\n"
                    "     # invalid \n \n"
                    "\n"
                    "      cgi ; \n"
                    "      cgi error; \n"
                    "      cgi on off; \n"
                    "      cgi off on; \n"
                    "   } \n"
                    "  } \n"
                    "}";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);


  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == true);
  std::set<std::size_t> errorsIdx{0, 1, 2, 5, 8, 9, 11, 13, 15, 17, 21, 24, 25, 26, 27, 29, 31, 33, 35, 39, 42, 43, 44, 45, 47, 49, 51, 53, 61, 64, 65, 66, 67, 69, 71, 73, 75};

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

TEST_CASE("errors client_max_body_size", "[Validator]")
{
  std::string raw = "http {\n"
                    "  server {\n"
                    "    # valid\n"
                    "    client_max_body_size 1;\n"
                    "    client_max_body_size        1k;\n"
                    "    client_max_body_size                    1m;\n"
                    "    client_max_body_size 1g;\n"
                    "    client_max_body_size        1K;\n"
                    "    client_max_body_size                    1M;\n"
                    "    client_max_body_size 1G;\n"
                    "    client_max_body_size 18446744073709551615;\n"
                    "    client_max_body_size 18014398509481983k;\n"
                    "    client_max_body_size 17592186044415m;\n"
                    "    client_max_body_size 17179869183g;\n"
                    "    client_max_body_size 18014398509481983K;\n"
                    "    client_max_body_size 17592186044415M;\n"
                    "    client_max_body_size 17179869183G;\n"
                    "\n"
                    "\n"
                    "    # invalid\n"
                    "    client_max_body_size ;\n"
                    "    client_max_body_size 0;\n"
                    "    client_max_body_size -1;\n"
                    "    client_max_body_size k;\n"
                    "    client_max_body_size m;\n"
                    "    client_max_body_size g;\n"
                    "    client_max_body_size error error;\n"
                    "    client_max_body_size 1ka;\n"
                    "    client_max_body_size 1ma;\n"
                    "    client_max_body_size 1g9;\n"
                    "    client_max_body_size 01;\n"
                    "    client_max_body_size 1 k;\n"
                    "    client_max_body_size 1 m;\n"
                    "    client_max_body_size 1 g;\n"
                    "    client_max_body_size 1 1;\n"
                    "    client_max_body_size 1k 1m 1g;\n"
                    "    client_max_body_size 000000000000000000000000000000000000000000001;\n"
                    "    client_max_body_size 18446744073709551616;\n"
                    "    client_max_body_size 18014398509481984k;\n"
                    "    client_max_body_size 17592186044416m;\n"
                    "    client_max_body_size 17179869184g;\n"
                    "    client_max_body_size 18014398509481984K;\n"
                    "    client_max_body_size 17592186044416M;\n"
                    "    client_max_body_size 17179869184G;\n"
                    "    \n"
                    "  }\n"
                    "}\n";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);


  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == true);
  std::set<std::size_t> errorsIdx{7, 10, 13, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46, 47, 48, 49, 51, 52, 54, 55, 57, 58, 60, 61, 63, 64, 65, 67, 68, 70, 71, 73, 74, 76, 77, 79, 81, 83, 85, 87, 89, 91, 93, 95, 97, 98, 100, 101, 103, 104, 106, 107, 109, 110, 112, 113, 115, 116, 118, 119, 121, 122};
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

TEST_CASE("errors error_page", "[Validator]")
{
  std::string raw = "http {\n"
                    "  server {\n"
                    "    location path {\n"
                    "\n"
                    "      # valid\n"
                    "\n"
                    "      error_page 504 301 503 400 502 401 500 404 /path;\n"
                    "      error_page 504 301 503 400 502 401 500 404 http://path;\n"
                    "      \n"
                    "      # invalid\n"
                    "\n"
                    "      error_page ;\n"
                    "      error_page 400 100 401 402 404 path;\n"
                    "      error_page 400 500 600 401 402 999 path;\n"
                    "      error_page /path 401;\n"
                    "      error_page 504 301 503 400 502 401 500 404 /path /path /path;\n"
                    "      error_page 504 301 503 400 502 401 500 404 http://path http://path;\n"
                    "      error_page 504 301 503 400 502 401 500 404 /path;\n"
                    "    }\n"
                    "  }\n"
                    "}";

  std::stringstream buffer;

  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());

  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == true);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == true);
  std::set<std::size_t> errorsIdx{30, 33, 37, 42, 45, 46, 49, 50, 61, 62, 74, 86};
                                    
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

TEST_CASE("errors index", "[Validator]")
{
  std::string raw = "http {"
                    "  "
                    "  # valid\n"
                    ""
                    "  index file file file;"
                    ""
                    "  # invalid\n"
                    ""
                    "  index ;"
                    "  index file file file;"
                    "}";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == true);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == true);
  std::set<std::size_t> errorsIdx{7, 8, 9, 12, 14};
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

TEST_CASE("errors listen", "[Validator]")
{
  std::string raw = "http {\n"
                    "  server {\n"
                    "\n"
                    "    # PORT NUMBER\n"
                    "    # valid\n"
                    "\n"
                    "    listen 0;\n"
                    "    listen 8000;\n"
                    "    listen 65535;\n"
                    "\n"
                    "    # invalid\n"
                    "    listen -1;\n"
                    "    listen 65536;\n"
                    "    listen 018446744073709551616;\n"
                    "    listen portNumber;\n"
                    "    listen 65535a;\n"
                    "    listen +65535;\n"
                    "    listen 01;\n"
                    "\n"
                    "    # IPV4\n"
                    "    # valid\n"
                    "    listen 0.0.0.0;\n"
                    "    listen 255.255.255.255;\n"
                    "\n"
                    "    # invalid\n"
                    "    listen ;\n"
                    "\n"
                    "    listen 256.0.0.0;\n"
                    "    listen 0.256.0.0;\n"
                    "    listen 0.0.256.0;\n"
                    "    listen 0.0.0.256;\n"
                    "\n"
                    "    listen 018446744073709551616.0.0.0;\n"
                    "    listen 00.18446744073709551616.0.0;\n"
                    "    listen 0.0.18446744073709551616.00;\n"
                    "    listen 0.0.0.018446744073709551616;\n"
                    "\n"
                    "    listen 01.1.1.1;\n"
                    "    listen 1.01.1.1;\n"
                    "    listen 1.1.01.1;\n"
                    "    listen 1.1.1.00000000000000000001;\n"
                    "\n"
                    "    listen 1a.1.1.1;\n"
                    "    listen 1.a1.1.1;\n"
                    "    listen 1.1.1a.1;\n"
                    "    listen 1.1.1.a1;\n"
                    "\n"
                    "    listen .1.1.1;\n"
                    "    listen 1..1.1;\n"
                    "    listen 1.1..1;\n"
                    "    listen 1.1.1.;\n"
                    "\n"
                    "    listen -1.1.1.1;\n"
                    "    listen 1.-1.1.1;\n"
                    "    listen 1.1.-1.1;\n"
                    "    listen 1.1.1.-1;\n"
                    "\n"
                    "    listen 1.;\n"
                    "    listen 1.1;\n"
                    "    listen 1.1.;\n"
                    "    listen 1.1.1;\n"
                    "    listen 1.1.1.;\n"
                    "    listen 1.1.1.1.;\n"
                    "    listen 1.1.1.1.1;\n"
                    "    listen 1.1.1.1.1.;\n"
                    "\n"
                    "    listen .;\n"
                    "    listen ..;\n"
                    "    listen ...;\n"
                    "\n"
                    "    listen 255.255.255.255 1.1.1.1 1.2.3.4;\n"
                    "\n"
                    "    listen 255255255255;\n"
                    "    listen ipv4address;\n"
                    "\n"
                    "    # IPV6\n"
                    "    # valid\n"
                    "    listen [1:1:1:1:1:1:1:1];\n"
                    "    listen [0001:0001:0001:0001:0001:0001:0001:0001];\n"
                    "    listen [0000:0000:0000:0000:0000:0000:0000:0000];\n"
                    "    listen [0:0:0:0:0:0:0:0];\n"
                    "    listen [0123:4567:89ab:cdef:fedc:ba98:7654:3210];\n"
                    "    listen [ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff];\n"
                    "\n"
                    "    listen [2001:db8:0:0:0::1];\n"
                    "    listen   [2001:db8:0:0::1];\n"
                    "    listen   [2001:db8:0::1];\n"
                    "    listen   [2001:db8::1];\n"
                    "\n"
                    "    listen [::];\n"
                    "\n"
                    "    listen [::0];\n"
                    "    listen [0::];\n"
                    "\n"
                    "    listen [::0000:0000];\n"
                    "    listen [0000::0000];\n"
                    "    listen [0000:0000::];\n"
                    "\n"
                    "    listen [::ABCD:ABCD:ABCD];\n"
                    "    listen [ABCD::ABCD:ABCD];\n"
                    "    listen [ABCD:ABCD::ABCD];\n"
                    "    listen [ABCD:ABCD:ABCD::];\n"
                    "\n"
                    "    listen [::f:f:f:f];\n"
                    "    listen [f::f:f:f];\n"
                    "    listen [f:f::f:f];\n"
                    "    listen [f:f:f::f];\n"
                    "    listen [f:f:f:f::];\n"
                    "\n"
                    "    listen [::000a:000a:000a:000a:000a];\n"
                    "    listen [000a::000a:000a:000a:000a];\n"
                    "    listen [000a:000a::000a:000a:000a];\n"
                    "    listen [000a:000a:000a::000a:000a];\n"
                    "    listen [000a:000a:000a:000a::000a];\n"
                    "    listen [000a:000a:000a:000a:000a::];\n"
                    "\n"
                    "    listen [::89ab:89ab:89ab:89ab:89ab:89ab];\n"
                    "    listen [89ab::89ab:89ab:89ab:89ab:89ab];\n"
                    "    listen [89ab:89ab::89ab:89ab:89ab:89ab];\n"
                    "    listen [89ab:89ab:89ab::89ab:89ab:89ab];\n"
                    "    listen [89ab:89ab:89ab:89ab::89ab:89ab];\n"
                    "    listen [89ab:89ab:89ab:89ab:89ab::89ab];\n"
                    "    listen [89ab:89ab:89ab:89ab:89ab:89ab::];\n"
                    "\n"
                    "    listen [::89ab:89ab:89ab:89ab:89ab:89ab:89ab];\n"
                    "    listen [89ab::89ab:89ab:89ab:89ab:89ab:89ab];\n"
                    "    listen [89ab:89ab::89ab:89ab:89ab:89ab:89ab];\n"
                    "    listen [89ab:89ab:89ab::89ab:89ab:89ab:89ab];\n"
                    "    listen [89ab:89ab:89ab:89ab::89ab:89ab:89ab];\n"
                    "    listen [89ab:89ab:89ab:89ab:89ab::89ab:89ab];\n"
                    "    listen [89ab:89ab:89ab:89ab:89ab:89ab::89ab];\n"
                    "    listen [89ab:89ab:89ab:89ab:89ab:89ab:89ab::];\n"
                    "    \n"
                    "    listen [0123::4567];\n"
                    "    listen [89ab::cdef];\n"
                    "    listen [0123:4567::89AB];\n"
                    "    listen [CDEF:0123:4567::89ab];\n"
                    "    listen [cdef:0123:4567:89AB::CDEF];\n"
                    "    listen [0123:4567:89ab:cdef:0123::4567];\n"
                    "    listen [89AB:CDEF:0123:4567:89ab:cdef::0123];\n"
                    "    \n"
                    "    listen [a:0b:00d:000d:0000:001:02:3];\n"
                    "\n"
                    "    # invalid\n"
                    "\n"
                    "    listen [1:1:1:1:1:1:1:-1];\n"
                    "    listen [1:1:+1:1:1:1:1:1];\n"
                    "\n"
                    "    listen [1:1:1:1:1:1:1:1] [1:1:1:1:1:1:1:1] [1:1:1:1:1:1:1:1] ;\n"
                    "\n"
                    "    listen [ipv6address];\n"
                    "    listen [0001:ipv6address:0001:0001:0001:0001::ipv6address];\n"
                    "\n"
                    "    listen ;\n"
                    "    listen [];\n"
                    "    listen [;\n"
                    "    listen [[]];\n"
                    "    listen [[;\n"
                    "    listen ]];\n"
                    "    listen ];\n"
                    "    listen [:];\n"
                    "    listen [:::];\n"
                    "\n"
                    "    listen [1];\n"
                    "    listen [1:1];\n"
                    "    listen [1:1:1];\n"
                    "    listen [1:1:1:1];\n"
                    "    listen [1:1:1:1:1];\n"
                    "    listen [1:1:1:1:1:1];\n"
                    "    listen [1:1:1:1:1:1:1];\n"
                    "\n"
                    "    listen [1:1:1:1:1:1:1:1;\n"
                    "    listen 1:1:1:1:1:1:1:1];\n"
                    "    listen [[1:1:1:1:1:1:1:1];\n"
                    "    listen [[1:1:1:1:1:1:1:1]];\n"
                    "    listen [1:1:1:1:1:1:1:1]];\n"
                    "\n"
                    "    listen [11111:1:1:1:1:1:1:1];\n"
                    "    listen [0001:0001:0001:0001:0001:0001:0001:00001];\n"
                    "    listen [0123:4567:89ab:cdef:ffedc:ba98:7654:3210];\n"
                    "    listen [ffff:ffff:fffff:ffff:ffff:ffff:ffff:ffff];\n"
                    "\n"
                    "    listen [1:1:1:1:1:1:1,1];\n"
                    "    listen [0001:00k1:0001:0001:0001:0001:0001:0001];\n"
                    "    listen [0123:4567:89ab:cdef:fedc:ba98:7654:32?0];\n"
                    "    listen [fff-:ffff:ffff:ffff:ffff:ffff:ffff:ffff];\n"
                    "\n"
                    "    listen [1:1:1:1:1:1:1:1:];\n"
                    "    listen [0001:0001:0001:0001:0001:0001:0001:0001:];\n"
                    "    listen [0123:4567:89ab:cdef:fedc:ba98:7654:3210:];\n"
                    "    listen [ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:];\n"
                    "\n"
                    "    listen [::ffff:];\n"
                    "    listen [::ffff:ffff:];\n"
                    "    listen [::ffff:ffff:ffff:];\n"
                    "    listen [::ffff:ffff:ffff:ffff:];\n"
                    "    listen [::ffff:ffff:ffff:ffff:ffff:];\n"
                    "    listen [::ffff:ffff:ffff:ffff:ffff:ffff:];\n"
                    "    listen [::ffff:ffff:ffff:ffff:ffff:ffff:ffff:];\n"
                    "    listen [ffff:::];\n"
                    "    listen [ffff:ffff:::];\n"
                    "    listen [ffff:ffff:ffff:::];\n"
                    "    listen [ffff:ffff:ffff:ffff:::];\n"
                    "    listen [ffff:ffff:ffff:ffff:ffff:::];\n"
                    "    listen [ffff:ffff:ffff:ffff:ffff:ffff:::];\n"
                    "    listen [ffff:ffff:ffff:ffff:ffff:ffff:ffff:::];\n"
                    "    listen [abcd::6789:];\n"
                    "    listen [0001::0001:];\n"
                    "    listen [0001:0001::0001:];\n"
                    "    listen [0001:0001:0001::0001:];\n"
                    "    listen [0001:0001:0001:0001::0001:];\n"
                    "    listen [0001:0001:0001:0001:0001::0001:];\n"
                    "    listen [0001:0001:0001:0001:0001:0001::0001:];\n"
                    "\n"
                    "    listen [a:0b:00d:000d:0000:001:02:3:];\n"
                    "    \n"
                    "    listen [1:1:1:1:1:1:1:1::];\n"
                    "    listen [0001:0001:0001:0001:0001:0001:0001:0001::];\n"
                    "    listen [0123:4567:89ab:cdef:fedc:ba98:7654:3210::];\n"
                    "    listen [ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff::];\n"
                    "\n"
                    "    listen [::ffff::];\n"
                    "    listen [::ffff:ffff::];\n"
                    "    listen [::ffff:ffff:ffff::];\n"
                    "    listen [::ffff:ffff:ffff:ffff::];\n"
                    "    listen [::ffff:ffff:ffff:ffff:ffff::];\n"
                    "    listen [::ffff:ffff:ffff:ffff:ffff:ffff::];\n"
                    "    listen [::ffff:ffff:ffff:ffff:ffff:ffff:ffff::];\n"
                    "    listen [abcd::6789::];\n"
                    "    listen [0001::0001::];\n"
                    "    listen [0001:0001::0001::];\n"
                    "    listen [0001:0001:0001::0001::];\n"
                    "    listen [0001:0001:0001:0001::0001::];\n"
                    "    listen [0001:0001:0001:0001:0001::0001::];\n"
                    "    listen [0001:0001:0001:0001:0001:0001::0001::];\n"
                    "\n"
                    "    listen [a:0b:00d:000d:0000:001:02:3::];\n"
                    "\n"
                    "    listen [:1:1:1:1:1:1:1:1];\n"
                    "    listen [:0001:0001:0001:0001:0001:0001:0001:0001];\n"
                    "    listen [:0123:4567:89ab:cdef:fedc:ba98:7654:3210];\n"
                    "    listen [:ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff];\n"
                    "\n"
                    "    listen [:::ffff];\n"
                    "    listen [:::ffff:ffff];\n"
                    "    listen [:::ffff:ffff:ffff];\n"
                    "    listen [:::ffff:ffff:ffff:ffff];\n"
                    "    listen [:::ffff:ffff:ffff:ffff:ffff];\n"
                    "    listen [:::ffff:ffff:ffff:ffff:ffff:ffff];\n"
                    "    listen [:::ffff:ffff:ffff:ffff:ffff:ffff:ffff];\n"
                    "    listen [:ffff::];\n"
                    "    listen [:ffff:ffff::];\n"
                    "    listen [:ffff:ffff:ffff::];\n"
                    "    listen [:ffff:ffff:ffff:ffff::];\n"
                    "    listen [:ffff:ffff:ffff:ffff:ffff::];\n"
                    "    listen [:ffff:ffff:ffff:ffff:ffff:ffff::];\n"
                    "    listen [:ffff:ffff:ffff:ffff:ffff:ffff:ffff::];\n"
                    "    listen [:abcd::6789];\n"
                    "    listen [:0001::0001];\n"
                    "    listen [:0001:0001::0001];\n"
                    "    listen [:0001:0001:0001::0001];\n"
                    "    listen [:0001:0001:0001:0001::0001];\n"
                    "    listen [:0001:0001:0001:0001:0001::0001];\n"
                    "    listen [:0001:0001:0001:0001:0001:0001::0001];\n"
                    "\n"
                    "    listen [:a:0b:00d:000d:0000:001:02:3];\n"
                    "\n"
                    "    listen [1:1:1:1:1:1:1:1:1];\n"
                    "    listen [::1:1:1:1:1:1:1:1];\n"
                    "    listen [::1:1:1:1:1:1:1::];\n"
                    "    listen [1:1:1:1:1:1:1:1:];\n"
                    "\n"
                    "    # ipv4 and port number\n"
                    "    # valid\n"
                    "\n"
                    "    listen 1.1.1.1:1;\n"
                    "\n"
                    "    # invalid\n"
                    "\n"
                    "    listen 1.1.1.1:65536;\n"
                    "    listen 1.1.1.1,1;\n"
                    "\n"
                    "    # ipv6 and port number\n"
                    "    # valid\n"
                    "\n"
                    "    listen [::]:1;\n"
                    "\n"
                    "    # invalid\n"
                    "\n"
                    "    listen [::]:65536;\n"
                    "    listen [::],65535;\n"
                    "    listen 1:1:1:1:1:1:1:1:8080;\n"
                    "\n"
                    "    # default_server\n"
                    "    # valid\n"
                    "\n"
                    "    listen 1.1.1.1:1 default_server;\n"
                    "    listen [::1]:65535 default_server;\n"
                    "\n"
                    "    # invalid\n"
                    "\n"
                    "    listen 1.1.1.1:1 default_server default_server;\n"
                    "    listen default_server [::1]:65535;\n"
                    "    listen default_server;\n"
                    "    listen 1.1.1.1:1 default_server error;\n"
                    "    listen [::1]:65535 error default_server;\n"
                    "  }\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == true);
  std::set<std::size_t> errorsIdx{14, 17, 20, 23, 26, 29, 32, 41, 43, 46, 49, 52, 55, 58, 61, 64, 67, 70, 73, 76, 79, 82, 85, 88, 91, 94, 97, 100, 103, 106, 109, 112, 115, 118, 121, 124, 127, 130, 133, 136, 139, 142, 145, 149, 150, 153, 156, 321, 324, 328, 329, 332, 335, 338, 340, 343, 346, 349, 352, 355, 358, 361, 364, 367, 370, 373, 376, 379, 382, 385, 388, 391, 394, 397, 400, 403, 406, 409, 412, 415, 418, 421, 424, 427, 430, 433, 436, 439, 442, 445, 448, 451, 454, 457, 460, 463, 466, 469, 472, 475, 478, 481, 484, 487, 490, 493, 496, 499, 502, 505, 508, 511, 514, 517, 520, 523, 526, 529, 532, 535, 538, 541, 544, 547, 550, 553, 556, 559, 562, 565, 568, 571, 574, 577, 580, 583, 586, 589, 592, 595, 598, 601, 604, 607, 610, 613, 616, 619, 622, 625, 628, 631, 634, 637, 640, 643, 646, 652, 655, 661, 664, 667, 680, 683, 684, 687, 692, 696, 697};
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

TEST_CASE("errors return", "[Validator]")
{
  std::string raw = "http {"
                    "  server {"
                    "    location  /path/path {"
                    ""
                    "      # valid\n"
                    ""
                    "      return 301 /path;"
                    "      return 302 http://someplacesomewhere;"
                    "      return 302;"
                    "      return /path;"
                    "      return http://someplace;"
                    ""
                    "      # invalid\n"
                    ""
                    "      return ;"
                    "      return code;"
                    "      return path;"
                    "      return 301 path;"
                    "      return code /path;"
                    "      return 302 /path more more more;"
                    ""
                    "      return 302 /path;"
                    "    }"
                    "  }"
                    "}";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == true);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == true);
  std::set<std::size_t> errorsIdx{11, 15, 18, 21, 24, 25, 26, 27, 29, 30, 32, 34, 36, 37, 40, 43, 44, 45, 47, 49};
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

TEST_CASE("errors root", "[Validator]")
{
  std::string raw = "http {"
                    "  server {"
                    ""
                    "    # valid\n"
                    ""
                    "    root /path;"
                    ""
                    "    # invalid\n"
                    ""
                    "    root ;"
                    "    root path;"
                    "    root /path /path;"
                    "    root /path;"
                    "  }"
                    "}";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == true);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == true);

  std::set<std::size_t> errorsIdx{7, 8, 9, 12, 14, 16, 17};
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

TEST_CASE("errors cgi_extension", "[Validator]")
{
  std::string raw = "http {"
										"	server {\n"
										"		location path {\n"
										"\n"
										"			 # valid\n"
										"\n"
										"			 cgi_extension py /path/path/path;\n"
										"			 cgi_extension php path/path/path;\n"
										"\n"
										"			 # invalid\n"
										"\n"
										"			 cgi_extension ;\n"
										"			 cgi_extension py;\n"
										"			 cgi_extension php;\n"
										"			 cgi_extension py py py;\n"
										"			 cgi_extension php /path 1 2 3 4;\n"
										"			 cgi_extension unknown /path/path;\n"
										"\n"
										"		}\n"
										"	}\n"
										"}";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == true);

  std::set<std::size_t> errorsIdx{16, 19, 22, 26, 31, 32, 33, 34};
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

// stack overflow protection

TEST_CASE("stack overflow protection", "[validator]")
{
  std::string raw = "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name example0.com ex0.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test/test {\n"
                    "\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    listen 8082;\n"
                    "    listen 124.0.0.34:8081;\n"
                    "    server_name example1.com ex1.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test1 {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test/test {\n"
                    "      \n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name example0.com ex0.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test/test {\n"
                    "\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    listen 8082;\n"
                    "    listen 124.0.0.34:8081;\n"
                    "    server_name example1.com ex1.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test1 {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test/test {\n"
                    "      \n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name example0.com ex0.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test/test {\n"
                    "\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    listen 8082;\n"
                    "    listen 124.0.0.34:8081;\n"
                    "    server_name example1.com ex1.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test1 {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test/test {\n"
                    "      \n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name example0.com ex0.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test/test {\n"
                    "\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    listen 8082;\n"
                    "    listen 124.0.0.34:8081;\n"
                    "    server_name example1.com ex1.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test1 {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test/test {\n"
                    "      \n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name example0.com ex0.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test/test {\n"
                    "\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    listen 8082;\n"
                    "    listen 124.0.0.34:8081;\n"
                    "    server_name example1.com ex1.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test1 {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test/test {\n"
                    "      \n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name example0.com ex0.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test/test {\n"
                    "\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    listen 8082;\n"
                    "    listen 124.0.0.34:8081;\n"
                    "    server_name example1.com ex1.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test1 {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test/test {\n"
                    "      \n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name example0.com ex0.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test/test {\n"
                    "\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    listen 8082;\n"
                    "    listen 124.0.0.34:8081;\n"
                    "    server_name example1.com ex1.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test1 {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test/test {\n"
                    "      \n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name example0.com ex0.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test/test {\n"
                    "\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    listen 8082;\n"
                    "    listen 124.0.0.34:8081;\n"
                    "    server_name example1.com ex1.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test1 {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test/test {\n"
                    "      \n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name example0.com ex0.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test/test {\n"
                    "\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    listen 8082;\n"
                    "    listen 124.0.0.34:8081;\n"
                    "    server_name example1.com ex1.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test1 {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test/test {\n"
                    "      \n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name example0.com ex0.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test/test {\n"
                    "\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    listen 8082;\n"
                    "    listen 124.0.0.34:8081;\n"
                    "    server_name example1.com ex1.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test1 {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test/test {\n"
                    "      \n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name example0.com ex0.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test/test {\n"
                    "\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    listen 8082;\n"
                    "    listen 124.0.0.34:8081;\n"
                    "    server_name example1.com ex1.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test1 {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test/test {\n"
                    "      \n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name example0.com ex0.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test/test {\n"
                    "\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    listen 8082;\n"
                    "    listen 124.0.0.34:8081;\n"
                    "    server_name example1.com ex1.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test1 {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test/test {\n"
                    "      \n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name example0.com ex0.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test/test {\n"
                    "\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    listen 8082;\n"
                    "    listen 124.0.0.34:8081;\n"
                    "    server_name example1.com ex1.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test1 {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test/test {\n"
                    "      \n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name example0.com ex0.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test/test {\n"
                    "\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    listen 8082;\n"
                    "    listen 124.0.0.34:8081;\n"
                    "    server_name example1.com ex1.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test1 {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test/test {\n"
                    "      \n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name example0.com ex0.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test/test {\n"
                    "\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    listen 8082;\n"
                    "    listen 124.0.0.34:8081;\n"
                    "    server_name example1.com ex1.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test1 {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test/test {\n"
                    "      \n"
                    "http {\n"
                    "\n"
                    "  server {\n"
                    "    listen 8081;\n"
                    "    server_name example0.com ex0.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test/test/test {\n"
                    "\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    listen 8082;\n"
                    "    listen 124.0.0.34:8081;\n"
                    "    server_name example1.com ex1.com;\n"
                    "\n"
                    "    client_max_body_size 2m;\n"
                    "\n"
                    "    location / {\n"
                    "      root ./www;\n"
                    "      index index.html;\n"
                    "      autoindex off;\n"
                    "    }\n"
                    "\n"
                    "    location /test1 {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test {\n"
                    "\n"
                    "    }\n"
                    "\n"
                    "    location /test1/test/test {\n"
                    "      \n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    server_name ex2.com;\n"
                    "  }\n"
                    "}\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    server_name ex2.com;\n"
                    "  }\n"
                    "}\n"
                    "\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    server_name ex2.com;\n"
                    "  }\n"
                    "}\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    server_name ex2.com;\n"
                    "  }\n"
                    "}\n"
                    "\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    server_name ex2.com;\n"
                    "  }\n"
                    "}\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    server_name ex2.com;\n"
                    "  }\n"
                    "}\n"
                    "\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    server_name ex2.com;\n"
                    "  }\n"
                    "}\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    server_name ex2.com;\n"
                    "  }\n"
                    "}\n"
                    "\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    server_name ex2.com;\n"
                    "  }\n"
                    "}\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    server_name ex2.com;\n"
                    "  }\n"
                    "}\n"
                    "\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    server_name ex2.com;\n"
                    "  }\n"
                    "}\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    server_name ex2.com;\n"
                    "  }\n"
                    "}\n"
                    "\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    server_name ex2.com;\n"
                    "  }\n"
                    "}\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    server_name ex2.com;\n"
                    "  }\n"
                    "}\n"
                    "\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    server_name ex2.com;\n"
                    "  }\n"
                    "}\n"
                    "    }\n"
                    "  }\n"
                    "\n"
                    "  server {\n"
                    "    server_name ex2.com;\n"
                    "  }\n"
                    "}\n";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  Parser parser(lexer);
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == true);
  REQUIRE(validator.GetError() == true);
  std::set<std::size_t> errorsIdx{79, 158, 237, 316, 395, 474, 553, 632, 711, 790, 869, 948, 1027, 1106};
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
