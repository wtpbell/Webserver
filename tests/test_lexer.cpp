#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "catch_amalgamated.hpp"
#include "config/Lexer.hpp"

TEST_CASE("lexer.current() returns current token", "[lexer]")
{
  std::string raw = "string";

  Lexer lexer(raw);
  lexer.Lex();

  Token& current = lexer.Current();

  REQUIRE(current.kind == TokenKind::String);
  REQUIRE(current.lexeme == "string");
  REQUIRE(current.leadingTrivia == "");
  REQUIRE(current.line == 1);
  REQUIRE(current.col == 1);
  REQUIRE(current.error == false);
}

TEST_CASE("token list is correct and lexer.next() returns next token", "[lexer]")
{
  std::string raw = "string 1234 ; { } location http events server";

  Lexer lexer(raw);
  lexer.Lex();

  Token& current = lexer.Current();

  REQUIRE(current.kind == TokenKind::String);
  REQUIRE(current.lexeme == "string");
  REQUIRE(current.leadingTrivia == "");
  REQUIRE(current.line == 1);
  REQUIRE(current.col == 1);
  REQUIRE(current.error == false);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::Number);
  REQUIRE(current.lexeme == "1234");
  REQUIRE(current.leadingTrivia == " ");
  REQUIRE(current.line == 1);
  REQUIRE(current.col == 8);
  REQUIRE(current.error == false);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::Semicolon);
  REQUIRE(current.lexeme == ";");
  REQUIRE(current.leadingTrivia == " ");
  REQUIRE(current.line == 1);
  REQUIRE(current.col == 13);
  REQUIRE(current.error == false);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::LBrace);
  REQUIRE(current.lexeme == "{");
  REQUIRE(current.leadingTrivia == " ");
  REQUIRE(current.line == 1);
  REQUIRE(current.col == 15);
  REQUIRE(current.error == false);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::RBrace);
  REQUIRE(current.lexeme == "}");
  REQUIRE(current.leadingTrivia == " ");
  REQUIRE(current.line == 1);
  REQUIRE(current.col == 17);
  REQUIRE(current.error == false);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::Location);
  REQUIRE(current.lexeme == "location");
  REQUIRE(current.leadingTrivia == " ");
  REQUIRE(current.line == 1);
  REQUIRE(current.col == 19);
  REQUIRE(current.error == false);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::Http);
  REQUIRE(current.lexeme == "http");
  REQUIRE(current.leadingTrivia == " ");
  REQUIRE(current.line == 1);
  REQUIRE(current.col == 28);
  REQUIRE(current.error == false);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::Events);
  REQUIRE(current.lexeme == "events");
  REQUIRE(current.leadingTrivia == " ");
  REQUIRE(current.line == 1);
  REQUIRE(current.col == 33);
  REQUIRE(current.error == false);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::Server);
  REQUIRE(current.lexeme == "server");
  REQUIRE(current.leadingTrivia == " ");
  REQUIRE(current.line == 1);
  REQUIRE(current.col == 40);
  REQUIRE(current.error == false);
  current = lexer.Next();
  REQUIRE(current.kind == TokenKind::Eof);
  REQUIRE(current.lexeme == "\0");
  REQUIRE(current.leadingTrivia == "");
  REQUIRE(current.line == 1);
  REQUIRE(current.col == 46);
  REQUIRE(current.error == false);
}

TEST_CASE("lexer.PrintTokenList() prints expected token list for test.conf", "[lexer]")
{
  std::ifstream infile("tests/config/lexer_test_files/valid/test.conf");
  REQUIRE(infile.is_open());

  std::stringstream configBuffer;
  configBuffer << infile.rdbuf();
  std::string configText = configBuffer.str();

  Lexer lexer(configText);
  lexer.Lex();

  std::stringstream buffer;

  auto* oldBuf = std::cout.rdbuf(buffer.rdbuf());
  lexer.PrintTokenList();
  std::cout.rdbuf(oldBuf);
  std::string actual = buffer.str();

  std::ifstream expectedFile("tests/config/lexer_test_files/valid/expected/test_expected");
  REQUIRE(expectedFile.is_open());

  std::stringstream expectedBuffer;
  expectedBuffer << expectedFile.rdbuf();
  std::string expected = expectedBuffer.str();

  REQUIRE(actual == expected);
}

TEST_CASE("lexer.PrintTokenList() prints expected token list for test_comments.conf", "[lexer]")
{
  std::ifstream infile("tests/config/lexer_test_files/valid/test_comments.conf");
  REQUIRE(infile.is_open());

  std::stringstream configBuffer;
  configBuffer << infile.rdbuf();
  std::string configText = configBuffer.str();

  Lexer lexer(configText);
  lexer.Lex();

  std::stringstream buffer;

  auto* oldBuf = std::cout.rdbuf(buffer.rdbuf());
  lexer.PrintTokenList();
  std::cout.rdbuf(oldBuf);
  std::string actual = buffer.str();

  std::ifstream expectedFile("tests/config/lexer_test_files/valid/expected/test_comments_expected");
  REQUIRE(expectedFile.is_open());

  std::stringstream expectedBuffer;
  expectedBuffer << expectedFile.rdbuf();
  std::string expected = expectedBuffer.str();

  REQUIRE(actual == expected);
}

TEST_CASE("lexer.PrintTokenList() prints expected token list for test_whitespace.conf", "[lexer]")
{
  std::ifstream infile("tests/config/lexer_test_files/valid/test_whitespace.conf");
  REQUIRE(infile.is_open());

  std::stringstream configBuffer;
  configBuffer << infile.rdbuf();
  std::string configText = configBuffer.str();

  Lexer lexer(configText);
  lexer.Lex();

  std::stringstream buffer;

  auto* oldBuf = std::cout.rdbuf(buffer.rdbuf());
  lexer.PrintTokenList();
  std::cout.rdbuf(oldBuf);
  std::string actual = buffer.str();

  std::ifstream expectedFile("tests/config/lexer_test_files/valid/expected/test_whitespace_expected");
  REQUIRE(expectedFile.is_open());

  std::stringstream expectedBuffer;
  expectedBuffer << expectedFile.rdbuf();
  std::string expected = expectedBuffer.str();

  REQUIRE(actual == expected);
}

TEST_CASE("Lexer setTokenErrorTrue sets error flag of current token to true", "[lexer]")
{
  std::string raw = "string";

  Lexer lexer(raw);
  lexer.Lex();

  Token& current = lexer.Current();

  REQUIRE(current.error == false);
  lexer.SetTokenErrorTrue();
  REQUIRE(current.error == true);
}
