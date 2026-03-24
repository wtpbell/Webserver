#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

#include "catch_amalgamated.hpp"
#include "config/Lexer.hpp"
#include "config/Parser.hpp"

TEST_CASE("syntactically correct tree", "[parser]")
{
  std::string raw = "http {    client_max_body_size 10m;    error_page 404 /errors/404.html;    error_page 500 502 503 504 /errors/50x.html;    server {        listen 80;        listen 443;        server_name example.com www.example.com;        root /var/www/example;        error_page 404 /errors/404.html;        error_page 500 502 503 504 /errors/50x.html;                location /errors/404.html {        }                location /errors/50x.html {                    }        location /api {            allowed_methods GET POST DELETE;            root /var/www/example;        }        location /old-page {            return 100 /new-page;        }        location /downloads {            root /var/www/example;        }        location /private {            root /var/www/example;        }        location /docs {            root /var/www/example;            index index.html index.htm default.html;        }        location /upload {            allowed_methods POST;                        client_body_temp_path /var/www/uploads/temp;                        root /var/www/uploads;        }    }    server {        listen 8080;        listen 8443;        server_name blog.example.com;        root /var/www/blog;        error_page 404 /404.html;        error_page 500 /500.html;        index index.html index.htm;        allowed_methods GET POST;        location /admin {            root /var/www/blog;        }    }    server {        listen 3000;        server_name static.example.com;        root /var/www/static;        client_max_body_size 50m;        location / {            allowed_methods GET;        }    }    server {        listen 5000;        server_name api.example.com;        error_page 404 /api/errors/not_found.json;        error_page 500 /api/errors/server_error.json;        location /api/v1 {            root /var/www/api;            allowed_methods DELETE GET POST;        }        location /api/v0 {            return 100;        }    }}";

  Lexer lexer;
  Parser parser(lexer);
  std::stringstream buffer;

  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  lexer.Lex(raw);
  parser.Parse();
  lexer.PrintErrorMessages();
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();

  REQUIRE(output.empty() == true);
  REQUIRE(parser.GetError() == false);
  for (std::size_t i = 0; i < lexer.GetSizeTokenList(); ++i)
  {
    CAPTURE(i);
    REQUIRE(lexer.GetTokenError(i) == false);
    REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  }
}

TEST_CASE("empty input", "[parser]")
{
  std::string raw = "";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  lexer.PrintErrorMessages();
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();

  REQUIRE(output.empty() == true);
  REQUIRE(parser.GetError() == false);
  for (std::size_t i = 0; i < lexer.GetSizeTokenList(); ++i)
  {
    CAPTURE(i);
    REQUIRE(lexer.GetTokenError(i) == false);
    REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  }
}

TEST_CASE("whitespace", "[parser]")
{
  std::string raw = "                    \f\n\r\t\v";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  lexer.PrintErrorMessages();
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();

  REQUIRE(output.empty() == true);
  REQUIRE(parser.GetError() == false);
  for (std::size_t i = 0; i < lexer.GetSizeTokenList(); ++i)
  {
    CAPTURE(i);
    REQUIRE(lexer.GetTokenError(i) == false);
    REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  }
}

TEST_CASE("events", "[parser]")
{
  std::string raw = "events";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  lexer.PrintErrorMessages();
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();

  REQUIRE(output.empty() == false);
  REQUIRE(parser.GetError() == true);
  REQUIRE(lexer.GetTokenError(0) == false);
  REQUIRE(lexer.GetTokenErrorMessage(0).empty() == true);
  REQUIRE(lexer.GetTokenError(1) == true);
  REQUIRE(lexer.GetTokenErrorMessage(1) ==  "1:7: [31merror:[0m unexpected token: [31mEOF[0m expected: `{`\n"
                                            "1:7: [31merror:[0m unexpected token: [31mEOF[0m expected: `}`\n");
}



TEST_CASE("http", "[parser]")
{
  std::string raw = "http";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  lexer.PrintErrorMessages();
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();

  REQUIRE(output.empty() == false);
  REQUIRE(parser.GetError() == true);
  REQUIRE(lexer.GetTokenError(0) == false);
  REQUIRE(lexer.GetTokenErrorMessage(0).empty() == true);
  REQUIRE(lexer.GetTokenError(1) == true);
  REQUIRE(lexer.GetTokenErrorMessage(1) ==  "1:5: [31merror:[0m unexpected token: [31mEOF[0m expected: `{`\n"
                                            "1:5: [31merror:[0m unexpected token: [31mEOF[0m expected: `}`\n");
}

TEST_CASE("location", "[parser]")
{
  std::string raw = "location";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  lexer.PrintErrorMessages();
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();

  REQUIRE(output.empty() == false);
  REQUIRE(parser.GetError() == true);
  REQUIRE(lexer.GetTokenError(0) == false);
  REQUIRE(lexer.GetTokenErrorMessage(0).empty() == true);
  REQUIRE(lexer.GetTokenError(1) == true);
  REQUIRE(lexer.GetTokenErrorMessage(1) ==  "1:9: [31merror:[0m unexpected token: [31mEOF[0m expected: PARAMETER\n"
                                            "1:9: [31merror:[0m unexpected token: [31mEOF[0m expected: `{`\n"
                                            "1:9: [31merror:[0m unexpected token: [31mEOF[0m expected: `}`\n");
}

TEST_CASE("server", "[parser]")
{
  std::string raw = "server";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  lexer.PrintErrorMessages();
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();

  REQUIRE(output.empty() == false);
  REQUIRE(parser.GetError() == true);
  REQUIRE(lexer.GetTokenError(0) == false);
  REQUIRE(lexer.GetTokenErrorMessage(0).empty() == true);
  REQUIRE(lexer.GetTokenError(1) == true);
  REQUIRE(lexer.GetTokenErrorMessage(1) ==  "1:7: [31merror:[0m unexpected token: [31mEOF[0m expected: `{`\n"
                                            "1:7: [31merror:[0m unexpected token: [31mEOF[0m expected: `}`\n");

}

TEST_CASE("LBrace", "[parser]")
{
  std::string raw = "{";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  lexer.PrintErrorMessages();
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();

  REQUIRE(output.empty() == false);
  REQUIRE(parser.GetError() == true);
  REQUIRE(lexer.GetTokenError(0) == true);
  REQUIRE(lexer.GetTokenErrorMessage(0) ==  "1:1: [31merror:[0m unexpected token: `[31m{[0m` expected: DIRECTIVE or BLOCKDIRECTIVE\n");
  REQUIRE(lexer.GetTokenError(1) == false);
  REQUIRE(lexer.GetTokenErrorMessage(1).empty() == true);
}

TEST_CASE("RBrace", "[parser]")
{
  std::string raw = "}";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  lexer.PrintErrorMessages();
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();

  REQUIRE(output.empty() == false);
  REQUIRE(parser.GetError() == true);
  REQUIRE(lexer.GetTokenError(0) == true);
  REQUIRE(lexer.GetTokenErrorMessage(0) ==  "1:1: [31merror:[0m unexpected token: `[31m}[0m` expected: DIRECTIVE or BLOCKDIRECTIVE\n");
  REQUIRE(lexer.GetTokenError(1) == false);
  REQUIRE(lexer.GetTokenErrorMessage(1).empty() == true);
}

TEST_CASE("semicolon", "[parser]")
{
  std::string raw = ";";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  lexer.PrintErrorMessages();
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();

  REQUIRE(output.empty() == false);
  REQUIRE(parser.GetError() == true);
  REQUIRE(lexer.GetTokenError(0) == true);
  REQUIRE(lexer.GetTokenErrorMessage(0) ==  "1:1: [31merror:[0m unexpected token: `[31m;[0m` expected: DIRECTIVE or BLOCKDIRECTIVE\n");
  REQUIRE(lexer.GetTokenError(1) == false);
  REQUIRE(lexer.GetTokenErrorMessage(1).empty() == true);
}

TEST_CASE("non-printable character", "[parser]")
{
  std::string raw = "";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  lexer.PrintErrorMessages();
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();
  

  REQUIRE(output.empty() == false);
  REQUIRE(parser.GetError() == true);
  REQUIRE(lexer.GetTokenError(0) == true);
  REQUIRE(lexer.GetTokenErrorMessage(0) ==  "1:1: [31merror:[0m non-printable character: `[31m[0m`\n"
                                            "1:1: [31merror:[0m unexpected token: `[31m[0m` expected: DIRECTIVE or BLOCKDIRECTIVE\n");
  REQUIRE(lexer.GetTokenError(1) == false);
  REQUIRE(lexer.GetTokenErrorMessage(1).empty() == true);
}

TEST_CASE("string", "[parser]")
{
  std::string raw = "string";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  lexer.PrintErrorMessages();
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();

  REQUIRE(output.empty() == false);
  REQUIRE(parser.GetError() == true);
  REQUIRE(lexer.GetTokenError(0) == true);
  REQUIRE(lexer.GetTokenErrorMessage(0) ==  "1:1: [31merror:[0m unexpected token: `[31mstring[0m` expected: DIRECTIVE or BLOCKDIRECTIVE\n");
  REQUIRE(lexer.GetTokenError(1) == false);
  REQUIRE(lexer.GetTokenErrorMessage(1).empty() == true);
}

TEST_CASE("missing LBrace", "[parser]")
{
  std::string raw = "http index Wait, where's the left brace?};";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  lexer.PrintErrorMessages();
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();

  REQUIRE(output.empty() == false);
  REQUIRE(parser.GetError() == true);

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

TEST_CASE("missing RBrace", "[parser]")
{
  std::string raw = "http { index Wait, where's the right brace?;";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  lexer.PrintErrorMessages();
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();

  REQUIRE(output.empty() == false);
  REQUIRE(parser.GetError() == true);

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

TEST_CASE("missing semicolon", "[parser]")
{
  std::string raw = "http { index Wait, where's the semicolon?}";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  Parser parser(lexer);
  parser.Parse();
  lexer.PrintErrorMessages();
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();

  REQUIRE(output.empty() == false);
  REQUIRE(parser.GetError() == true);
  
  std::set<std::size_t> errorsIdx{7,8};
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

TEST_CASE("all token kinds once", "[parser]")
{
  std::string raw = "string 1234;{}#this is not a location token\n location http events server listen server_name  root   index    return #this is  not   an   alias    token      \n     alias client_max_body_size\nerror_page\nallowed_methods client_body_temp_path autoindex\n";
  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
  REQUIRE(lexer.GetError() == false);
  Parser parser(lexer);
  parser.Parse();
  lexer.PrintErrorMessages();
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();

  REQUIRE(output.empty() == false);
  REQUIRE(parser.GetError() == true);
  
  std::set<std::size_t> errorsIdx{0, 1, 2, 3, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
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

TEST_CASE("misplaced semicolons", "[parser]")
{
  std::string raw = ";listen 1234param;events{listen param;} http{root param;\n;                    server {alias param;location param{index param 1234param;}}} alias param;;";

  Lexer lexer;
  Parser parser(lexer);
  std::stringstream buffer;

  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  lexer.Lex(raw);
  parser.Parse();
  lexer.PrintErrorMessages();
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();

  REQUIRE(output.empty() == false);
  REQUIRE(parser.GetError() == true);
  std::set<std::size_t> errorsIdx{0,15,34};
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

TEST_CASE("multi_error02.conf", "[parser]")
{
  std::string raw = "42;42;42;42;42;listen ;{}listen param param param;;listen param param param;http { server { location /path/ { listen 42;}http}}";

  Lexer lexer;
  Parser parser(lexer);
  std::stringstream buffer;

  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  lexer.Lex(raw);
  parser.Parse();
  lexer.PrintErrorMessages();
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();

  REQUIRE(output.empty() == false);
  REQUIRE(parser.GetError() == true);
  std::set<std::size_t> errorsIdx{0,1,2,3,4,5,6,7,8,9,12,13,19,37,38,39};

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

TEST_CASE("multi_error05.conf", "[parser]")
{
  std::string raw = ";;;http {    client_max_body_size 10000000000M;    error_page 404 /errors/404.html;    error_page 500 502 503 504 /errors/50x.html;    server {        listen 80;        listen 443 ssl;        server_name example.com www.example.com;        root /var/www/example;        error_page 404 /errors/404.html;        error_page 500 502 503 504 /errors/50x.html;                location /errors/404.html {                    }                location /errors/50x.html {            ;        }                location /api {            allowed_methods POST DELETE GET;            root /path /var/www/example;;;;        }                location /old-page {            return 100 /new-page;        }                location /kapouet {            root /tmp/www;        }        g enabled        location /downloads {            root /var/www/example;            autoindex on;        }                location /private {            root /var/www/example;            autoindex off;        }                location /docs {            root /var/www/example;            index index.html index.htm default.html;        }                location /upload {            allowed_methods POST POST;            client_body_temp_path /var/www/uploads/temp /path;            root /var/www/uploads;        }    }        server /invalid_param {        listen 8080;        listen 8443 ssl;        server_name blog.example.com;        root /var/www/blog;        error_page 404 /404.html;        error_page 500 /500.html;        index index.html index.htm;        allowed_methods GEt POST;        location /admin {            root /var/www/blog;            autoindex off;        }    }     server    server {        listen 3000;        server_name static.example.com;        root /var/www/static;        autoindex on;        client_max_body_size 50M;        location / {            allowed_methods GET;        }    }{}        server {        listen 5000;        server_name api.example.commmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm;        error_page 404 /api/errors/not_found.json;        error_page 500 /api/errors/server_error.json;        location /api/v1 {            root /var/www/api;            allowed_methods GET POST DELETE;        }                location /api/v0 /path {            return 999 /api/v1;            }}invalid events {";
  Lexer lexer;
  Parser parser(lexer);
  std::stringstream buffer;

  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  lexer.Lex(raw);
  parser.Parse();
  lexer.PrintErrorMessages();
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();

  REQUIRE(output.empty() == false);
  REQUIRE(parser.GetError() == true);
  std::set<std::size_t> errorsIdx{0,1,2,53,67,68,69,86,87,137,180,205,237,245,248};
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