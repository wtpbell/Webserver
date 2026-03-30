#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "catch_amalgamated.hpp"
#include "config/Lexer.hpp"



TEST_CASE("lexer handles empty input correctly", "[lexer]")
{
  std::string raw = "";

  Lexer lexer(raw);

  REQUIRE(lexer.GetError() == false);

  REQUIRE(lexer.GetTokenKind(0) == TokenKind::kEof);
  REQUIRE(lexer.GetLexeme(0) == "");
  REQUIRE(lexer.GetTrivia(0) == "");
  REQUIRE(lexer.GetLine(0) == 1);
  REQUIRE(lexer.GetCol(0) == 1);
  REQUIRE(lexer.GetTokenError(0) == false);
  REQUIRE(lexer.GetTokenErrorMessage(0).empty());
}

TEST_CASE("lexer handles whitespace correctly", "[lexer]")
{
  std::string raw = "                    \f\n\r\t\v";

  Lexer lexer(raw);

  REQUIRE(lexer.GetError() == false);

  REQUIRE(lexer.GetTokenKind(0) == TokenKind::kEof);
  REQUIRE(lexer.GetLexeme(0) == "");
  REQUIRE(lexer.GetTrivia(0) == "                    \f\n\r\t\v");
  REQUIRE(lexer.GetLine(0) == 3);
  REQUIRE(lexer.GetCol(0) == 1);
  REQUIRE(lexer.GetTokenError(0) == false);
  REQUIRE(lexer.GetTokenErrorMessage(0).empty());
}

TEST_CASE("lexer handles comments correctly", "[lexer]")
{
  std::string raw = "\n\n\n #comment  #comment\n#comment\v                     #this is the last comment      \n";

  Lexer lexer(raw);

  REQUIRE(lexer.GetError() == false);

  REQUIRE(lexer.GetTokenKind(0) == TokenKind::kEof);
  REQUIRE(lexer.GetLexeme(0) == "");
  REQUIRE(lexer.GetTrivia(0) == "\n\n\n #comment  #comment\n#comment\v                     #this is the last comment      \n");
  REQUIRE(lexer.GetLine(0) == 7);
  REQUIRE(lexer.GetCol(0) == 1);
  REQUIRE(lexer.GetTokenError(0) == false);
  REQUIRE(lexer.GetTokenErrorMessage(0).empty());
}

TEST_CASE("lexer.Lex(raw) correctly constructs token structs for every kind of token", "[lexer]")
{
  std::string raw = "string 1234;{}#this is not a location token\n location http events server listen server_name  root   index    return #this is  not   an   alias    token      \n     alias client_max_body_size\nerror_page\nallowed_methods client_max_body_size  autoindex\ncgi cgi_extension\n";

  Lexer lexer(raw);

  REQUIRE(lexer.GetError() == false);

  std::size_t i = 0;

  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kString);
  REQUIRE(lexer.GetLexeme(i) == "string");
  REQUIRE(lexer.GetTrivia(i).empty() == true);
  REQUIRE(lexer.GetLine(i) == 1);
  REQUIRE(lexer.GetCol(i) == 1);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kString);
  REQUIRE(lexer.GetLexeme(i) == "1234");
  REQUIRE(lexer.GetTrivia(i) == " ");
  REQUIRE(lexer.GetLine(i) == 1);
  REQUIRE(lexer.GetCol(i) == 8);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kSemicolon);
  REQUIRE(lexer.GetLexeme(i) == ";");
  REQUIRE(lexer.GetTrivia(i).empty() == true);
  REQUIRE(lexer.GetLine(i) == 1);
  REQUIRE(lexer.GetCol(i) == 12);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kLBrace);
  REQUIRE(lexer.GetLexeme(i) == "{");
  REQUIRE(lexer.GetTrivia(i).empty() == true);
  REQUIRE(lexer.GetLine(i) == 1);
  REQUIRE(lexer.GetCol(i) == 13);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kRBrace);
  REQUIRE(lexer.GetLexeme(i) == "}");
  REQUIRE(lexer.GetTrivia(i).empty() == true);
  REQUIRE(lexer.GetLine(i) == 1);
  REQUIRE(lexer.GetCol(i) == 14);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kLocation);
  REQUIRE(lexer.GetLexeme(i) == "location");
  REQUIRE(lexer.GetTrivia(i) == "#this is not a location token\n ");
  REQUIRE(lexer.GetLine(i) == 2);
  REQUIRE(lexer.GetCol(i) == 2);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kHttp);
  REQUIRE(lexer.GetLexeme(i) == "http");
  REQUIRE(lexer.GetTrivia(i) == " ");
  REQUIRE(lexer.GetLine(i) == 2);
  REQUIRE(lexer.GetCol(i) == 11);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kString);
  REQUIRE(lexer.GetLexeme(i) == "events");
  REQUIRE(lexer.GetTrivia(i) == " ");
  REQUIRE(lexer.GetLine(i) == 2);
  REQUIRE(lexer.GetCol(i) == 16);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kServer);
  REQUIRE(lexer.GetLexeme(i) == "server");
  REQUIRE(lexer.GetTrivia(i) == " ");
  REQUIRE(lexer.GetLine(i) == 2);
  REQUIRE(lexer.GetCol(i) == 23);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kListen);
  REQUIRE(lexer.GetLexeme(i) == "listen");
  REQUIRE(lexer.GetTrivia(i) == " ");
  REQUIRE(lexer.GetLine(i) == 2);
  REQUIRE(lexer.GetCol(i) == 30);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kServerName);
  REQUIRE(lexer.GetLexeme(i) == "server_name");
  REQUIRE(lexer.GetTrivia(i) == " ");
  REQUIRE(lexer.GetLine(i) == 2);
  REQUIRE(lexer.GetCol(i) == 37);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kRoot);
  REQUIRE(lexer.GetLexeme(i) == "root");
  REQUIRE(lexer.GetTrivia(i) == "  ");
  REQUIRE(lexer.GetLine(i) == 2);
  REQUIRE(lexer.GetCol(i) == 50);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kIndex);
  REQUIRE(lexer.GetLexeme(i) == "index");
  REQUIRE(lexer.GetTrivia(i) == "   ");
  REQUIRE(lexer.GetLine(i) == 2);
  REQUIRE(lexer.GetCol(i) == 57);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kReturn);
  REQUIRE(lexer.GetLexeme(i) == "return");
  REQUIRE(lexer.GetTrivia(i) == "    ");
  REQUIRE(lexer.GetLine(i) == 2);
  REQUIRE(lexer.GetCol(i) == 66);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kAlias);
  REQUIRE(lexer.GetLexeme(i) == "alias");
  REQUIRE(lexer.GetTrivia(i) == " #this is  not   an   alias    token      \n     ");
  REQUIRE(lexer.GetLine(i) == 3);
  REQUIRE(lexer.GetCol(i) == 6);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kClientMaxBodySize);
  REQUIRE(lexer.GetLexeme(i) == "client_max_body_size");
  REQUIRE(lexer.GetTrivia(i) == " ");
  REQUIRE(lexer.GetLine(i) == 3);
  REQUIRE(lexer.GetCol(i) == 12);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kErrorPage);
  REQUIRE(lexer.GetLexeme(i) == "error_page");
  REQUIRE(lexer.GetTrivia(i) == "\n");
  REQUIRE(lexer.GetLine(i) == 4);
  REQUIRE(lexer.GetCol(i) == 1);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kAllowedMethods);
  REQUIRE(lexer.GetLexeme(i) == "allowed_methods");
  REQUIRE(lexer.GetTrivia(i) == "\n");
  REQUIRE(lexer.GetLine(i) == 5);
  REQUIRE(lexer.GetCol(i) == 1);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kClientMaxBodySize);
  REQUIRE(lexer.GetLexeme(i) == "client_max_body_size");
  REQUIRE(lexer.GetTrivia(i) == " ");
  REQUIRE(lexer.GetLine(i) == 5);
  REQUIRE(lexer.GetCol(i) == 17);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kAutoindex);
  REQUIRE(lexer.GetLexeme(i) == "autoindex");
  REQUIRE(lexer.GetTrivia(i) == "  ");
  REQUIRE(lexer.GetLine(i) == 5);
  REQUIRE(lexer.GetCol(i) == 39);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kCgi);
  REQUIRE(lexer.GetLexeme(i) == "cgi");
  REQUIRE(lexer.GetTrivia(i) == "\n");
  REQUIRE(lexer.GetLine(i) == 6);
  REQUIRE(lexer.GetCol(i) == 1);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kCgiExtension);
  REQUIRE(lexer.GetLexeme(i) == "cgi_extension");
  REQUIRE(lexer.GetTrivia(i) == " ");
  REQUIRE(lexer.GetLine(i) == 6);
  REQUIRE(lexer.GetCol(i) == 5);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kEof);
  REQUIRE(lexer.GetLexeme(i) == "");
  REQUIRE(lexer.GetTrivia(i) == "\n");
  REQUIRE(lexer.GetLine(i) == 7);
  REQUIRE(lexer.GetCol(i) == 1);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
}

TEST_CASE("no false positives when directive names are embedded in longer lexemes", "[lexer]")
{
  std::string raw = "string 1234;{}#this is not a location token\n llocation httpp eevents serverserver llisten server_namee  rootroot   indexx    rreturn #this is  not   an   alias    token      \n     aliasaliasaliasalias cclient_max_body_size\nerror_pagee\nallowed_methodsallowed_methods client_body_temp_pathroot aautoindex\nccgi cgi_extensionn\n";

  Lexer lexer(raw);

  REQUIRE(lexer.GetError() == false);

  std::size_t i = 0;

  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kString);
  REQUIRE(lexer.GetLexeme(i) == "string");
  REQUIRE(lexer.GetTrivia(i).empty() == true);
  REQUIRE(lexer.GetLine(i) == 1);
  REQUIRE(lexer.GetCol(i) == 1);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kString);
  REQUIRE(lexer.GetLexeme(i) == "1234");
  REQUIRE(lexer.GetTrivia(i) == " ");
  REQUIRE(lexer.GetLine(i) == 1);
  REQUIRE(lexer.GetCol(i) == 8);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kSemicolon);
  REQUIRE(lexer.GetLexeme(i) == ";");
  REQUIRE(lexer.GetTrivia(i).empty() == true);
  REQUIRE(lexer.GetLine(i) == 1);
  REQUIRE(lexer.GetCol(i) == 12);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kLBrace);
  REQUIRE(lexer.GetLexeme(i) == "{");
  REQUIRE(lexer.GetTrivia(i).empty() == true);
  REQUIRE(lexer.GetLine(i) == 1);
  REQUIRE(lexer.GetCol(i) == 13);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kRBrace);
  REQUIRE(lexer.GetLexeme(i) == "}");
  REQUIRE(lexer.GetTrivia(i).empty() == true);
  REQUIRE(lexer.GetLine(i) == 1);
  REQUIRE(lexer.GetCol(i) == 14);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kString);
  REQUIRE(lexer.GetLexeme(i) == "llocation");
  REQUIRE(lexer.GetTrivia(i) == "#this is not a location token\n ");
  REQUIRE(lexer.GetLine(i) == 2);
  REQUIRE(lexer.GetCol(i) == 2);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kString);
  REQUIRE(lexer.GetLexeme(i) == "httpp");
  REQUIRE(lexer.GetTrivia(i) == " ");
  REQUIRE(lexer.GetLine(i) == 2);
  REQUIRE(lexer.GetCol(i) == 12);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kString);
  REQUIRE(lexer.GetLexeme(i) == "eevents");
  REQUIRE(lexer.GetTrivia(i) == " ");
  REQUIRE(lexer.GetLine(i) == 2);
  REQUIRE(lexer.GetCol(i) == 18);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kString);
  REQUIRE(lexer.GetLexeme(i) == "serverserver");
  REQUIRE(lexer.GetTrivia(i) == " ");
  REQUIRE(lexer.GetLine(i) == 2);
  REQUIRE(lexer.GetCol(i) == 26);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kString);
  REQUIRE(lexer.GetLexeme(i) == "llisten");
  REQUIRE(lexer.GetTrivia(i) == " ");
  REQUIRE(lexer.GetLine(i) == 2);
  REQUIRE(lexer.GetCol(i) == 39);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kString);
  REQUIRE(lexer.GetLexeme(i) == "server_namee");
  REQUIRE(lexer.GetTrivia(i) == " ");
  REQUIRE(lexer.GetLine(i) == 2);
  REQUIRE(lexer.GetCol(i) == 47);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kString);
  REQUIRE(lexer.GetLexeme(i) == "rootroot");
  REQUIRE(lexer.GetTrivia(i) == "  ");
  REQUIRE(lexer.GetLine(i) == 2);
  REQUIRE(lexer.GetCol(i) == 61);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kString);
  REQUIRE(lexer.GetLexeme(i) == "indexx");
  REQUIRE(lexer.GetTrivia(i) == "   ");
  REQUIRE(lexer.GetLine(i) == 2);
  REQUIRE(lexer.GetCol(i) == 72);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kString);
  REQUIRE(lexer.GetLexeme(i) == "rreturn");
  REQUIRE(lexer.GetTrivia(i) == "    ");
  REQUIRE(lexer.GetLine(i) == 2);
  REQUIRE(lexer.GetCol(i) == 82);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kString);
  REQUIRE(lexer.GetLexeme(i) == "aliasaliasaliasalias");
  REQUIRE(lexer.GetTrivia(i) == " #this is  not   an   alias    token      \n     ");
  REQUIRE(lexer.GetLine(i) == 3);
  REQUIRE(lexer.GetCol(i) == 6);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kString);
  REQUIRE(lexer.GetLexeme(i) == "cclient_max_body_size");
  REQUIRE(lexer.GetTrivia(i) == " ");
  REQUIRE(lexer.GetLine(i) == 3);
  REQUIRE(lexer.GetCol(i) == 27);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kString);
  REQUIRE(lexer.GetLexeme(i) == "error_pagee");
  REQUIRE(lexer.GetTrivia(i) == "\n");
  REQUIRE(lexer.GetLine(i) == 4);
  REQUIRE(lexer.GetCol(i) == 1);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kString);
  REQUIRE(lexer.GetLexeme(i) == "allowed_methodsallowed_methods");
  REQUIRE(lexer.GetTrivia(i) == "\n");
  REQUIRE(lexer.GetLine(i) == 5);
  REQUIRE(lexer.GetCol(i) == 1);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kString);
  REQUIRE(lexer.GetLexeme(i) == "client_body_temp_pathroot");
  REQUIRE(lexer.GetTrivia(i) == " ");
  REQUIRE(lexer.GetLine(i) == 5);
  REQUIRE(lexer.GetCol(i) == 32);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kString);
  REQUIRE(lexer.GetLexeme(i) == "aautoindex");
  REQUIRE(lexer.GetTrivia(i) == " ");
  REQUIRE(lexer.GetLine(i) == 5);
  REQUIRE(lexer.GetCol(i) == 58);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kString);
  REQUIRE(lexer.GetLexeme(i) == "ccgi");
  REQUIRE(lexer.GetTrivia(i) == "\n");
  REQUIRE(lexer.GetLine(i) == 6);
  REQUIRE(lexer.GetCol(i) == 1);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kString);
  REQUIRE(lexer.GetLexeme(i) == "cgi_extensionn");
  REQUIRE(lexer.GetTrivia(i) == " ");
  REQUIRE(lexer.GetLine(i) == 6);
  REQUIRE(lexer.GetCol(i) == 6);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
  REQUIRE(lexer.GetTokenKind(i) == TokenKind::kEof);
  REQUIRE(lexer.GetLexeme(i) == "");
  REQUIRE(lexer.GetTrivia(i) == "\n");
  REQUIRE(lexer.GetLine(i) == 7);
  REQUIRE(lexer.GetCol(i) == 1);
  REQUIRE(lexer.GetTokenError(i) == false);
  REQUIRE(lexer.GetTokenErrorMessage(i).empty() == true);
  ++i;
}

TEST_CASE("GetTokenError() and SetTokenErrorTrue()", "[lexer]")
{
  std::string raw = "string";

  Lexer lexer(raw);

  REQUIRE(lexer.GetError() == false);

  REQUIRE(lexer.GetTokenError(0) == false);
  lexer.SetTokenErrorTrue(0);
  REQUIRE(lexer.GetTokenError(0) == true);

  REQUIRE(lexer.GetTokenError(1) == false);
  lexer.SetTokenErrorTrue(1);
  REQUIRE(lexer.GetTokenError(1) == true);
}

TEST_CASE("SetTokenErrorMessage() and GetTokenErrorMessage()", "[lexer]")
{
  std::string raw = "string";

  Lexer lexer(raw);

  REQUIRE(lexer.GetTokenErrorMessage(0).empty() == true);
  lexer.SetTokenErrorMessage(0, "this is a test");
  REQUIRE(lexer.GetTokenErrorMessage(0).empty() == false);
  REQUIRE(lexer.GetTokenErrorMessage(0) == "this is a test");
}

TEST_CASE("GetSizeTokenList", "[lexer]")
{
  std::string raw0 = "";
  Lexer lexer0(raw0);
  REQUIRE(lexer0.GetSizeTokenList() == 1);

  std::string raw1 = "1";
  Lexer lexer1(raw1);
  REQUIRE(lexer1.GetSizeTokenList() == 2);

  std::string raw2 = " 9 9 9 9 9 9 9 9 9 ";
  Lexer lexer2(raw2);
  REQUIRE(lexer2.GetSizeTokenList() == 10);
}

TEST_CASE("non-printable character", "[lexer]")
{
  std::string raw = " http autoindex \nlisten";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer(raw);
  lexer.PrintErrorMessages();

  std::string output = buffer.str();
  std::cerr.rdbuf(oldBuf);
  
  REQUIRE(output.empty() == false);
  REQUIRE(lexer.GetError() == true);
  REQUIRE(lexer.GetTokenError(0) == true);
  REQUIRE(lexer.GetTokenErrorMessage(0) ==  "1:1: [31merror:[0m non-printable character: `[31m[0m`\n");
  REQUIRE(lexer.GetTokenError(1) == true);
  REQUIRE(lexer.GetTokenErrorMessage(1) ==  "1:3: [31merror:[0m non-printable character: `[31mhttp[0m`\n");
  REQUIRE(lexer.GetTokenError(2) == true);
  REQUIRE(lexer.GetTokenErrorMessage(2) ==  "1:9: [31merror:[0m non-printable character: `[31mautoindex[0m`\n");
  REQUIRE(lexer.GetTokenError(3) == true);
  REQUIRE(lexer.GetTokenErrorMessage(3) ==  "2:1: [31merror:[0m non-printable character: `[31mlisten[0m`\n");
  REQUIRE(lexer.GetTokenError(4) == false);
  REQUIRE(lexer.GetTokenErrorMessage(4).empty() == true);
}