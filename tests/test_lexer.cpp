#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "catch_amalgamated.hpp"
#include "config/Lexer.hpp"

TEST_CASE("lexer.Current() returns current token", "[lexer]")
{
  std::string raw = "string";

  Lexer lexer;
  lexer.Lex(raw);

  REQUIRE(lexer.GetError() == false);

  Token current = lexer.Current();

  REQUIRE(current.kind == TokenKind::String);
  REQUIRE(current.lexeme == "string");
  REQUIRE(current.leadingTrivia == "");
  REQUIRE(current.line == 1);
  REQUIRE(current.col == 1);
  REQUIRE(current.idxTokenList == 0);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
}

TEST_CASE("lexer.Next() returns next token", "[lexer]")
{
  std::string raw = "string1                     string2";

  Lexer lexer;
  lexer.Lex(raw);

  REQUIRE(lexer.GetError() == false);

  Token current = lexer.Current();

  REQUIRE(current.kind == TokenKind::String);
  REQUIRE(current.lexeme == "string1");
  REQUIRE(current.leadingTrivia == "");
  REQUIRE(current.line == 1);
  REQUIRE(current.col == 1);
  REQUIRE(current.idxTokenList == 0);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);

  current = lexer.Next();

  REQUIRE(current.kind == TokenKind::String);
  REQUIRE(current.lexeme == "string2");
  REQUIRE(current.leadingTrivia == "                     ");
  REQUIRE(current.line == 1);
  REQUIRE(current.col == 29);
  REQUIRE(current.idxTokenList == 1);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
}

TEST_CASE("lexer handles empty input correctly", "[lexer]")
{
  std::string raw = "";

  Lexer lexer;
  lexer.Lex(raw);

  REQUIRE(lexer.GetError() == false);

  Token current = lexer.Current();

  REQUIRE(current.kind == TokenKind::Eof);
  REQUIRE(current.lexeme == "");
  REQUIRE(current.leadingTrivia == "");
  REQUIRE(current.line == 1);
  REQUIRE(current.col == 1);
  REQUIRE(current.idxTokenList == 0);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
}

TEST_CASE("lexer handles whitespace correctly", "[lexer]")
{
  std::string raw = "                    \f\n\r\t\v";

  Lexer lexer;
  lexer.Lex(raw);

  REQUIRE(lexer.GetError() == false);

  Token current = lexer.Current();

  REQUIRE(current.kind == TokenKind::Eof);
  REQUIRE(current.lexeme == "");
  REQUIRE(current.leadingTrivia == "                    \f\n\r\t\v");
  REQUIRE(current.line == 3);
  REQUIRE(current.col == 1);
  REQUIRE(current.idxTokenList == 0);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
}

TEST_CASE("lexer handles comments correctly", "[lexer]")
{
  std::string raw = "\n\n\n #comment  #comment\n#comment\n                     #this is the last comment      \n";

  Lexer lexer;
  lexer.Lex(raw);

  REQUIRE(lexer.GetError() == false);

  Token current = lexer.Current();

  REQUIRE(current.kind == TokenKind::Eof);
  REQUIRE(current.lexeme == "");
  REQUIRE(current.leadingTrivia == "\n\n\n #comment  #comment\n#comment\n                     #this is the last comment      \n");
  REQUIRE(current.line == 7);
  REQUIRE(current.col == 1);
  REQUIRE(current.idxTokenList == 0);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
}

TEST_CASE("lexer.Lex(raw) correctly constructs token structs for every kind of token", "[lexer]")
{
  std::string raw = "string 1234;{}#this is not a location token\n location http events server listen server_name  root   index    return #this is  not   an   alias    token      \n     alias client_max_body_size\nerror_page\nallowed_methods client_body_temp_path autoindex\n";

  Lexer lexer;
  lexer.Lex(raw);

  REQUIRE(lexer.GetError() == false);

  Token current = lexer.Current();

  REQUIRE(current.kind == TokenKind::String);
  REQUIRE(current.lexeme == "string");
  REQUIRE(current.leadingTrivia == "");
  REQUIRE(current.line == 1);
  REQUIRE(current.col == 1);
  REQUIRE(current.idxTokenList == 0);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::String);
  REQUIRE(current.lexeme == "1234");
  REQUIRE(current.leadingTrivia == " ");
  REQUIRE(current.line == 1);
  REQUIRE(current.col == 8);
  REQUIRE(current.idxTokenList == 1);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::Semicolon);
  REQUIRE(current.lexeme == ";");
  REQUIRE(current.leadingTrivia == "");
  REQUIRE(current.line == 1);
  REQUIRE(current.col == 12);
  REQUIRE(current.idxTokenList == 2);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::LBrace);
  REQUIRE(current.lexeme == "{");
  REQUIRE(current.leadingTrivia == "");
  REQUIRE(current.line == 1);
  REQUIRE(current.col == 13);
  REQUIRE(current.idxTokenList == 3);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::RBrace);
  REQUIRE(current.lexeme == "}");
  REQUIRE(current.leadingTrivia == "");
  REQUIRE(current.line == 1);
  REQUIRE(current.col == 14);
  REQUIRE(current.idxTokenList == 4);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::Location);
  REQUIRE(current.lexeme == "location");
  REQUIRE(current.leadingTrivia == "#this is not a location token\n ");
  REQUIRE(current.line == 2);
  REQUIRE(current.col == 2);
  REQUIRE(current.idxTokenList == 5);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::Http);
  REQUIRE(current.lexeme == "http");
  REQUIRE(current.leadingTrivia == " ");
  REQUIRE(current.line == 2);
  REQUIRE(current.col == 11);
  REQUIRE(current.idxTokenList == 6);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::Events);
  REQUIRE(current.lexeme == "events");
  REQUIRE(current.leadingTrivia == " ");
  REQUIRE(current.line == 2);
  REQUIRE(current.col == 16);
  REQUIRE(current.idxTokenList == 7);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::Server);
  REQUIRE(current.lexeme == "server");
  REQUIRE(current.leadingTrivia == " ");
  REQUIRE(current.line == 2);
  REQUIRE(current.col == 23);
  REQUIRE(current.idxTokenList == 8);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::Listen);
  REQUIRE(current.lexeme == "listen");
  REQUIRE(current.leadingTrivia == " ");
  REQUIRE(current.line == 2);
  REQUIRE(current.col == 30);
  REQUIRE(current.idxTokenList == 9);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::Server_name);
  REQUIRE(current.lexeme == "server_name");
  REQUIRE(current.leadingTrivia == " ");
  REQUIRE(current.line == 2);
  REQUIRE(current.col == 37);
  REQUIRE(current.idxTokenList == 10);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::Root);
  REQUIRE(current.lexeme == "root");
  REQUIRE(current.leadingTrivia == "  ");
  REQUIRE(current.line == 2);
  REQUIRE(current.col == 50);
  REQUIRE(current.idxTokenList == 11);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::Index);
  REQUIRE(current.lexeme == "index");
  REQUIRE(current.leadingTrivia == "   ");
  REQUIRE(current.line == 2);
  REQUIRE(current.col == 57);
  REQUIRE(current.idxTokenList == 12);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::Return);
  REQUIRE(current.lexeme == "return");
  REQUIRE(current.leadingTrivia == "    ");
  REQUIRE(current.line == 2);
  REQUIRE(current.col == 66);
  REQUIRE(current.idxTokenList == 13);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::Alias);
  REQUIRE(current.lexeme == "alias");
  REQUIRE(current.leadingTrivia == " #this is  not   an   alias    token      \n     ");
  REQUIRE(current.line == 3);
  REQUIRE(current.col == 6);
  REQUIRE(current.idxTokenList == 14);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::Client_max_body_size);
  REQUIRE(current.lexeme == "client_max_body_size");
  REQUIRE(current.leadingTrivia == " ");
  REQUIRE(current.line == 3);
  REQUIRE(current.col == 12);
  REQUIRE(current.idxTokenList == 15);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::Error_page);
  REQUIRE(current.lexeme == "error_page");
  REQUIRE(current.leadingTrivia == "\n");
  REQUIRE(current.line == 4);
  REQUIRE(current.col == 1);
  REQUIRE(current.idxTokenList == 16);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::Allowed_methods);
  REQUIRE(current.lexeme == "allowed_methods");
  REQUIRE(current.leadingTrivia == "\n");
  REQUIRE(current.line == 5);
  REQUIRE(current.col == 1);
  REQUIRE(current.idxTokenList == 17);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::Client_body_temp_path);
  REQUIRE(current.lexeme == "client_body_temp_path");
  REQUIRE(current.leadingTrivia == " ");
  REQUIRE(current.line == 5);
  REQUIRE(current.col == 17);
  REQUIRE(current.idxTokenList == 18);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::Autoindex);
  REQUIRE(current.lexeme == "autoindex");
  REQUIRE(current.leadingTrivia == " ");
  REQUIRE(current.line == 5);
  REQUIRE(current.col == 39);
  REQUIRE(current.idxTokenList == 19);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::Eof);
  REQUIRE(current.lexeme == "");
  REQUIRE(current.leadingTrivia == "\n");
  REQUIRE(current.line == 6);
  REQUIRE(current.col == 1);
  REQUIRE(current.idxTokenList == 20);
  REQUIRE(current.error == false);
  REQUIRE(current.errorMessage.empty() == true);
}

TEST_CASE("GetTokenError() and SetTokenErrorTrue()", "[lexer]")
{
  std::string raw = "string";

  Lexer lexer;
  lexer.Lex(raw);

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

  Lexer lexer;
  lexer.Lex(raw);

  REQUIRE(lexer.GetTokenErrorMessage(0).empty() == true);
  lexer.SetTokenErrorMessage(0, "this is a test");
  REQUIRE(lexer.GetTokenErrorMessage(0).empty() == false);
  REQUIRE(lexer.GetTokenErrorMessage(0) == "this is a test");
}

TEST_CASE("GetSizeTokenList", "[lexer]")
{
  std::string raw0 = "";
  Lexer lexer0;
  lexer0.Lex(raw0);
  REQUIRE(lexer0.GetSizeTokenList() == 1);

  std::string raw1 = "1";
  Lexer lexer1;
  lexer1.Lex(raw1);
  REQUIRE(lexer1.GetSizeTokenList() == 2);

  std::string raw2 = " 9 9 9 9 9 9 9 9 9 ";
  Lexer lexer2;
  lexer2.Lex(raw2);
  REQUIRE(lexer2.GetSizeTokenList() == 10);
}

TEST_CASE("non-printable character", "[lexer]")
{
  std::string raw = " http autoindex \nlisten";

  std::stringstream buffer;
  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());
  Lexer lexer;
  lexer.Lex(raw);
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