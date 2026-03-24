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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  validator.ValidateAst();
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  validator.ValidateAst();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetTokenError(0) == true);
  REQUIRE(lexer.GetTokenErrorMessage(0).empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == true);
}

TEST_CASE("events", "[validator]")
{
  std::string raw = "events";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
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

TEST_CASE("http", "[validator]")
{
  std::string raw = "http";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  lexer.PrintErrorMessages();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
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
  std::string raw = "string 1234;{}#this is not a location token\n location http events server listen server_name  root   index    return #this is  not   an   alias    token      \n     alias client_max_body_size\nerror_page\nallowed_methods client_body_temp_path autoindex\n";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  REQUIRE(lexer.GetError() == false);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  lexer.PrintErrorMessages();
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();

  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == false);
  REQUIRE(parser.GetError() == true);
  REQUIRE(validator.GetError() == true);
  
  std::set<std::size_t> errorsIdx{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
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
  std::string raw = "http {    client_max_body_size 10m;    error_page 404 /errors/404.html;    error_page 500 502 503 504 /errors/50x.html;        server {        listen 8080;        listen 8443;        server_name blog.example.com;        root /var/www/blog;        error_page 404 /404.html;        error_page 500 /500.html;        index index.html index.htm;        allowed_methods GET POST;        location /admin {            root /var/www/blog;        }    }    server {        listen 3000;        server_name static.example.com;        root /var/www/static;        client_max_body_size 50m;        location / {            allowed_methods GET;        }    }    }";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);


  REQUIRE(output.empty());
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
  std::string raw = "# incorrectly nested events\n"
                    "\n"
                    "events {events{}}\n"
                    "\n"
                    "http{events{}}\n"
                    "\n"
                    "http{server{events{}}}\n"
                    "\n"
                    "http{server{location /documents/{events{}}}}\n"
                    "\n"
                    "\n"
                    "# incorrectly nested http\n"
                    "\n"
                    "events{http{}}\n"
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
                    "events{server{}}\n"
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
                    "events{location /documents/{}}\n"
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
                    "                events {\n"
                    "                  http {\n"
                    "                    events {\n"
                    "                      http {\n"
                    "                        events {\n"
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
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);

  REQUIRE(output.empty() == false);
  REQUIRE(validator.GetError() == true);
  std::set<std::size_t> errorsIdx{2, 8, 11, 16, 28, 36, 38, 42, 44, 45, 50, 52, 62, 64, 68, 73, 81, 88, 91, 95, 103, 109, 113, 116, 119, 127, 139, 148, 151, 156, 164, 171, 174, 176, 178, 180, 182, 184, 186, 188, 190, 192, 197, 205, 207, 217};
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
                    "return 100;\n"
                    "client_body_temp_path /path;\n"
                    "\n"
                    "events {\n"
                    "  listen 8080;\n"
                    "  server_name name;\n"
                    "  root /path;\n"
                    "  index file;\n"
                    "  alias /path;\n"
                    "  client_max_body_size 1G;\n"
                    "  error_page 401 http://path;\n"
                    "  allowed_methods DELETE;\n"
                    "  return 100;\n"
                    "  client_body_temp_path /path;\n"
                    "}\n"
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
                    "  return 100;\n"
                    "  client_body_temp_path /path;\n"
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
                    "    return 100;\n"
                    "    client_body_temp_path /path;\n"
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
                    "      return 100;\n"
                    "      client_body_temp_path /path;\n"
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
                    "    return 100;\n"
                    "    client_body_temp_path /path;\n"
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
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);


  REQUIRE(output.empty() == false);
  REQUIRE(validator.GetError() == true);
  std::set<std::size_t> errorsIdx{0, 3, 6, 9, 12, 15, 18, 22, 25, 28, 33, 36, 39, 42, 45, 48, 51, 55, 58, 61, 67, 70, 73, 79, 89, 92, 112, 134, 137, 146, 167, 170, 173, 182};

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
  std::string raw = "http {server {location /path { alias /path;alias path;alias /path;alias path /path;alias /path path;}}}";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  REQUIRE(lexer.GetError() == true);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);


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
  std::string raw = "http {  server {    location path {       allowed_methods GET POST DELETE;      allowed_methods GET;      allowed_methods POST;      allowed_methods DELETE;      allowed_methods DELETE POST GET;            allowed_methods;      allowed_methods GET POST delete DELETE;      allowed_methods method;      allowed_methods METHOD;      allowed_methods POSt;      allowed_methods GET POST DELETE GET POST DELETE;      allowed_methods get post delete GET POST delete GET POST DELETE get post DELETE GET POST DELETE;    }  }}";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  REQUIRE(lexer.GetError() == false);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);


  REQUIRE(output.empty() == false);
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
  std::string raw = "http {      autoindex on;  autoindex off;  server {        autoindex on;    autoindex off;    location /path{      autoindex on;      autoindex off;      autoindex ;      autoindex on off;      autoindex off on on on ;      autoindex huh?;      autoindex on;    }  }}";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  REQUIRE(lexer.GetError() == true);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);


  REQUIRE(output.empty() == false);
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
  Lexer lexer;
  lexer.Lex(raw);
  REQUIRE(lexer.GetError() == false);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);


  REQUIRE(output.empty() == false);
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

TEST_CASE("errors client_body_temp_path", "[Validator]")
{
  std::string raw = "http {  client_body_temp_path /path;  client_body_temp_path ;  client_body_temp_path path;  client_body_temp_path /path /path;  client_body_temp_path /path path;  client_body_temp_path path /path;}";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  REQUIRE(lexer.GetError() == false);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);


  REQUIRE(output.empty() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == true);
  std::set<std::size_t> errorsIdx{5, 6, 7, 10, 12, 14, 16, 18, 20, 22};
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
  Lexer lexer;
  lexer.Lex(raw);
  REQUIRE(lexer.GetError() == false);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);


  REQUIRE(output.empty() == false);
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

  Lexer lexer;
  lexer.Lex(raw);
  REQUIRE(lexer.GetError() == true);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);


  REQUIRE(output.empty() == false);
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
  std::string raw = "http { index file file file;  index ;  index file file file;}";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  REQUIRE(lexer.GetError() == true);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);


  REQUIRE(output.empty() == false);
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
  std::string raw = "http {  server {     listen 0;    listen 8000;    listen 65535;   listen -1;    listen 65536;    listen 018446744073709551616;    listen portNumber;    listen 65535a;    listen +65535;    listen 01;    listen 0.0.0.0;    listen 255.255.255.255;     listen ;    listen 256.0.0.0;    listen 0.256.0.0;    listen 0.0.256.0;    listen 0.0.0.256;    listen 018446744073709551616.0.0.0;    listen 00.18446744073709551616.0.0;    listen 0.0.18446744073709551616.00;    listen 0.0.0.018446744073709551616;    listen 01.1.1.1;    listen 1.01.1.1;    listen 1.1.01.1;    listen 1.1.1.00000000000000000001;    listen 1a.1.1.1;    listen 1.a1.1.1;    listen 1.1.1a.1;    listen 1.1.1.a1;    listen .1.1.1;    listen 1..1.1;    listen 1.1..1;    listen 1.1.1.;    listen -1.1.1.1;    listen 1.-1.1.1;    listen 1.1.-1.1;    listen 1.1.1.-1;    listen 1.;    listen 1.1;    listen 1.1.;    listen 1.1.1;    listen 1.1.1.;    listen 1.1.1.1.;    listen 1.1.1.1.1;    listen 1.1.1.1.1.;    listen .;    listen ..;    listen ...;    listen 255.255.255.255 1.1.1.1 1.2.3.4;    listen 255255255255;    listen ipv4address;    listen [1:1:1:1:1:1:1:1];    listen [0001:0001:0001:0001:0001:0001:0001:0001];    listen [0123:4567:89ab:cdef:fedc:ba98:7654:3210];    listen [ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff];    listen [::];    listen [::ffff];    listen [::ffff:ffff];    listen [::ffff:ffff:ffff];    listen [::ffff:ffff:ffff:ffff];    listen [::ffff:ffff:ffff:ffff:ffff];    listen [::ffff:ffff:ffff:ffff:ffff:ffff];    listen [::ffff:ffff:ffff:ffff:ffff:ffff:ffff];    listen [ffff::];    listen [ffff:ffff::];    listen [ffff:ffff:ffff::];    listen [ffff:ffff:ffff:ffff::];    listen [ffff:ffff:ffff:ffff:ffff::];    listen [ffff:ffff:ffff:ffff:ffff:ffff::];    listen [ffff:ffff:ffff:ffff:ffff:ffff:ffff::];    listen [abcd::6789];    listen [0001::0001];    listen [0001:0001::0001];    listen [0001:0001:0001::0001];    listen [0001:0001:0001:0001::0001];    listen [0001:0001:0001:0001:0001::0001];    listen [0001:0001:0001:0001:0001:0001::0001];    listen [a:0b:00d:000d:0000:001:02:3];  listen [1:1:1:1:1:1:1:1] [1:1:1:1:1:1:1:1] [1:1:1:1:1:1:1:1] ;    listen ;    listen [];    listen [;    listen ];    listen [:];    listen [:::];    listen [1];    listen [1:1];    listen [1:1:1];    listen [1:1:1:1];    listen [1:1:1:1:1];    listen [1:1:1:1:1:1];    listen [1:1:1:1:1:1:1];    listen [1:1:1:1:1:1:1:1;    listen 1:1:1:1:1:1:1:1];    listen [[1:1:1:1:1:1:1:1];    listen [[1:1:1:1:1:1:1:1]];    listen [1:1:1:1:1:1:1:1]];    listen [11111:1:1:1:1:1:1:1];    listen [0001:0001:0001:0001:0001:0001:0001:00001];    listen [0123:4567:89ab:cdef:ffedc:ba98:7654:3210];    listen [ffff:ffff:fffff:ffff:ffff:ffff:ffff:ffff];    listen [1:1:1:1:1:1:1,1];    listen [0001:00k1:0001:0001:0001:0001:0001:0001];    listen [0123:4567:89ab:cdef:fedc:ba98:7654:32?0];    listen [fff-:ffff:ffff:ffff:ffff:ffff:ffff:ffff];    listen [1:1:1:1:1:1:1:1:];    listen [0001:0001:0001:0001:0001:0001:0001:0001:];    listen [0123:4567:89ab:cdef:fedc:ba98:7654:3210:];    listen [ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:];    listen [::ffff:];    listen [::ffff:ffff:];    listen [::ffff:ffff:ffff:];    listen [::ffff:ffff:ffff:ffff:];    listen [::ffff:ffff:ffff:ffff:ffff:];    listen [::ffff:ffff:ffff:ffff:ffff:ffff:];    listen [::ffff:ffff:ffff:ffff:ffff:ffff:ffff:];    listen [ffff:::];    listen [ffff:ffff:::];    listen [ffff:ffff:ffff:::];    listen [ffff:ffff:ffff:ffff:::];    listen [ffff:ffff:ffff:ffff:ffff:::];    listen [ffff:ffff:ffff:ffff:ffff:ffff:::];    listen [ffff:ffff:ffff:ffff:ffff:ffff:ffff:::];    listen [abcd::6789:];    listen [0001::0001:];    listen [0001:0001::0001:];    listen [0001:0001:0001::0001:];    listen [0001:0001:0001:0001::0001:];    listen [0001:0001:0001:0001:0001::0001:];    listen [0001:0001:0001:0001:0001:0001::0001:];    listen [a:0b:00d:000d:0000:001:02:3:];    listen [1:1:1:1:1:1:1:1::];    listen [0001:0001:0001:0001:0001:0001:0001:0001::];    listen [0123:4567:89ab:cdef:fedc:ba98:7654:3210::];    listen [ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff::];    listen [::ffff::];    listen [::ffff:ffff::];    listen [::ffff:ffff:ffff::];    listen [::ffff:ffff:ffff:ffff::];    listen [::ffff:ffff:ffff:ffff:ffff::];    listen [::ffff:ffff:ffff:ffff:ffff:ffff::];    listen [::ffff:ffff:ffff:ffff:ffff:ffff:ffff::];    listen [abcd::6789::];    listen [0001::0001::];    listen [0001:0001::0001::];    listen [0001:0001:0001::0001::];    listen [0001:0001:0001:0001::0001::];    listen [0001:0001:0001:0001:0001::0001::];    listen [0001:0001:0001:0001:0001:0001::0001::];    listen [a:0b:00d:000d:0000:001:02:3::];    listen [:1:1:1:1:1:1:1:1];    listen [:0001:0001:0001:0001:0001:0001:0001:0001];    listen [:0123:4567:89ab:cdef:fedc:ba98:7654:3210];    listen [:ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff];    listen [:::ffff];    listen [:::ffff:ffff];    listen [:::ffff:ffff:ffff];    listen [:::ffff:ffff:ffff:ffff];    listen [:::ffff:ffff:ffff:ffff:ffff];    listen [:::ffff:ffff:ffff:ffff:ffff:ffff];    listen [:::ffff:ffff:ffff:ffff:ffff:ffff:ffff];    listen [:ffff::];    listen [:ffff:ffff::];    listen [:ffff:ffff:ffff::];    listen [:ffff:ffff:ffff:ffff::];    listen [:ffff:ffff:ffff:ffff:ffff::];    listen [:ffff:ffff:ffff:ffff:ffff:ffff::];    listen [:ffff:ffff:ffff:ffff:ffff:ffff:ffff::];    listen [:abcd::6789];    listen [:0001::0001];    listen [:0001:0001::0001];    listen [:0001:0001:0001::0001];    listen [:0001:0001:0001:0001::0001];    listen [:0001:0001:0001:0001:0001::0001];    listen [:0001:0001:0001:0001:0001:0001::0001];    listen [:a:0b:00d:000d:0000:001:02:3];    listen [1:1:1:1:1:1:1:1:1];    listen [::1:1:1:1:1:1:1:1];    listen [::1:1:1:1:1:1:1::];    listen [1:1:1:1:1:1:1:1:1];    listen 1.1.1.1:1;    listen 1.1.1.1:65536;    listen 1.1.1.1,1;   listen [::]:1;    listen [::]:65536;    listen [::],65535;    listen 1:1:1:1:1:1:1:1:8080;    listen [::]:8080;  }}";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  REQUIRE(lexer.GetError() == false);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);


  REQUIRE(output.empty() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == true);
  std::set<std::size_t> errorsIdx{14,17,20,23,26,29,32,41,43,46,49,52,55,58,61,64,67,70,73,76,79,82,85,88,91,94,97,100,103,106,109,112,115,118,121,124,127,130,133,136,139,142,145,149,150,153,156,241,242,245,247,250,253,256,259,262,265,268,271,274,277,280,283,286,289,292,295,298,301,304,307,310,313,316,319,322,325,328,331,334,337,340,343,346,349,352,355,358,361,364,367,370,373,376,379,382,385,388,391,394,397,400,403,406,409,412,415,418,421,424,427,430,433,436,439,442,445,448,451,454,457,460,463,466,469,472,475,478,481,484,487,490,493,496,499,502,505,508,511,514,517,520,523,526,529,532,535,538,541,544,550,553,559,562,565};
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
  std::string raw = "http {  server {    location  /path/path {     return 100 /path;      return 100 http://someplacesomewhere;      return 100;      return /path;      return http://someplace;     return ;      return code;      return path;      return 100 path;      return code /path;      return 100 /path more more more;      return 100 /path;    }  }}";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  REQUIRE(lexer.GetError() == true);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);


  REQUIRE(output.empty() == false);
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
  std::string raw = "http {  server {root /path;   root ;    root path;    root /path /path;    root /path;  }}";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  REQUIRE(lexer.GetError() == true);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);


  REQUIRE(output.empty() == false);
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

TEST_CASE("errors server_name", "[Validator]")
{
  std::string raw = "http {  server {    server_name www.example1.com www.example2.com www.example3.com;    server_name hostname;    server_name our-server.com;    server_name our-machine.our-host.our-domain;    server_name 63chaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaars.63chaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaars.63chaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaars.61chaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaars;    server_name ;    server_name &/();    server_name test.test.test.test!!.test;    server_name 63chaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaars.63chaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaars.63chaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaars.62chaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaars;    server_name 64chaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaars.test.test;    server_name test.test.64chaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaars.test.test;    server_name test.test.64chaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaars;    server_name -test;    server_name test-;    server_name -test.test;    server_name test-.test;    server_name test.-test;    server_name test.test-;    server_name .test;    server_name test.;    server_name .test.test;    server_name test..test;    server_name test.test.;    server_name test test. test..test;    server_name test- test-.test test;  }}";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  REQUIRE(lexer.GetError() == false);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);


  REQUIRE(output.empty() == false);
  REQUIRE(parser.GetError() == false);
  REQUIRE(validator.GetError() == true);
  std::set<std::size_t> errorsIdx{22,24,27,30,33,36,39,42,45,48,51,54,57,60,63,66,69,72,76,77,80,81};
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
  Lexer lexer;
  lexer.Lex(raw);
  REQUIRE(lexer.GetError() == false);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);


  REQUIRE(output.empty() == false);
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

// mixed errors

TEST_CASE("multi_error05.conf", "[validator]")
{
  std::string raw = ";;;http {    client_max_body_size 10000000000M;    error_page 404 /errors/404.html;    error_page 500 502 503 504 /errors/50x.html;    server {        listen 80;        listen 443 ssl;        server_name example.com www.example.com;        root /var/www/example;        error_page 404 /errors/404.html;        error_page 500 502 503 504 /errors/50x.html;                location /errors/404.html {                    }                location /errors/50x.html {            ;        }                location /api {            allowed_methods POST DELETE GET;            root /path /var/www/example;;;;        }                location /old-page {            return 100 /new-page;        }                location /kapouet {            root /tmp/www;        }        g enabled        location /downloads {            root /var/www/example;            autoindex on;        }                location /private {            root /var/www/example;            autoindex off;        }                location /docs {            root /var/www/example;            index index.html index.htm default.html;        }                location /upload {            allowed_methods POST POST;            client_body_temp_path /var/www/uploads/temp /path;            root /var/www/uploads;        }    }        server /invalid_param {        listen 8080;        listen 8443 ssl;        server_name blog.example.com;        root /var/www/blog;        error_page 404 /404.html;        error_page 500 /500.html;        index index.html index.htm;        allowed_methods GEt POST;        location /admin {            root /var/www/blog;            autoindex off;        }    }     server    server {        listen 3000;        server_name static.example.com;        root /var/www/static;        autoindex on;        client_max_body_size 50M;        location / {            allowed_methods GET;        }    }{}        server {        listen 5000;        server_name api.example.commmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm;        error_page 404 /api/errors/not_found.json;        error_page 500 /api/errors/server_error.json;        location /api/v1 {            root /var/www/api;            allowed_methods GET POST DELETE;        }                location /api/v0 /path {            return 999 /api/v1;            }}invalid events {";
  
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  REQUIRE(lexer.GetError() == false);
  Parser parser(lexer);
  parser.Parse();
  ValidatorIpPort validatorIpPort;
  Validator validator(lexer, parser, validatorIpPort);
  validator.ValidateAst();
  lexer.PrintErrorMessages();
  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);


  REQUIRE(output.empty() == false);
  REQUIRE(parser.GetError() == true);
  REQUIRE(validator.GetError() == true);
  std::set<std::size_t> errorsIdx{0,1,2,26,53,65,67,68,69,86,87,125,129,137,144,165,180,205,207,213,237,240,245,248};
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
