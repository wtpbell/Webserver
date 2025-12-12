/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   Lexer.cpp                                           :+:    :+:           */
/*                                                      +:+                   */
/*   By: jstuhrin <jstuhrin@student.codam.nl>          +#+                    */
/*                                                    +#+                     */
/*   Created: 2025/12/02 10:36:50 by jstuhrin       #+#    #+#                */
/*   Updated: 2025/12/02 10:36:51 by jstuhrin       ########   odam.nl        */
/*                                                                            */
/* ************************************************************************** */

#include "config/Lexer.hpp"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

// constructor, destructor
Lexer::Lexer(const std::string& buffer)
    : buffer_(buffer), line_(1), col_(1), posBuffer_(0), idxCurrentToken_(0), madeEofToken_(false)
{
}

// public
void Lexer::Lex()
{
  while (!madeEofToken_)
  {
    MakeToken();
  }
}

void Lexer::PrintTokenList()
{
  std::cout << "\n#####################\n##### TOKENLIST #####\n#####################\n\n";
  for (auto token : tokenList_)
  {
    PrintToken(token);
  }
  std::cout << "#########################\n##### END TOKENLIST #####\n#########################\n\n";
}

void Lexer::PrintConfigsDebug()
{
  std::cout << "\n##############################\n##### CONFIGS FILE DEBUG #####\n##############################\n\n";
  for (auto token : tokenList_)
  {
    PrintTriviaAndLexeme(token);
  }
  std::cout << "\n\n##################################\n##### END CONFIGS FILE DEBUG "
               "#####\n##################################\n\n";
}

Token& Lexer::Next()
{
  return (tokenList_[++idxCurrentToken_]);
}

Token& Lexer::Current()
{
  return (tokenList_[idxCurrentToken_]);
}

void Lexer::SetTokenErrorTrue()
{
  tokenList_[idxCurrentToken_].error = true;
}

// private
bool Lexer::Is(char c)
{
  return (buffer_[posBuffer_] == c);
}

bool Lexer::Is(std::string_view lexeme)
{
  return (!buffer_.compare(posBuffer_, lexeme.length(), lexeme) && IsSeparator(buffer_[posBuffer_ + lexeme.length()]));
}

bool Lexer::IsSeparator(char c)
{
  return (std::isspace(static_cast<unsigned char>(c)) || c == '{' || c == '}' || c == ';' || c == '#' || c == '\0');
}

TokenKind Lexer::DetermineKind()
{
  if (Is('{'))
  {
    return (TokenKind::LBrace);
  }
  if (Is('}'))
  {
    return (TokenKind::RBrace);
  }
  if (Is(';'))
  {
    return (TokenKind::Semicolon);
  }
  if (Is('\0'))
  {
    return (TokenKind::Eof);
  }
  if (std::isdigit(buffer_[posBuffer_]))
  {
    return (TokenKind::Number);
  }
  if (Is("events"))
  {
    return (TokenKind::Events);
  }
  if (Is("http"))
  {
    return (TokenKind::Http);
  }
  if (Is("server"))
  {
    return (TokenKind::Server);
  }
  if (Is("location"))
  {
    return (TokenKind::Location);
  }
  return (TokenKind::String);
}

std::string Lexer::ExtractLexeme()
{
  size_t start = posBuffer_;
  size_t end = posBuffer_;
  while (!IsSeparator(buffer_[end]))
  {
    ++end;
  }
  return (buffer_.substr(start, end - start));
}

std::string Lexer::DetermineLexeme(TokenKind kind)
{
  switch (kind)
  {
    case TokenKind::LBrace:
      return ("{");
    case TokenKind::RBrace:
      return ("}");
    case TokenKind::Semicolon:
      return (";");
    case TokenKind::Events:
      return ("events");
    case TokenKind::Http:
      return ("http");
    case TokenKind::Location:
      return ("location");
    case TokenKind::Server:
      return ("server");
    case TokenKind::String:
      return (ExtractLexeme());
    case TokenKind::Number:
      return (ExtractLexeme());
    case TokenKind::Eof:
      return ("\0");
  }
  assert(false && "Invalid TokenKind passed to detLexeme");
  __builtin_unreachable();
}

bool Lexer::IsWhiteSpace(size_t pos)
{
  return (std::isspace(static_cast<unsigned char>(buffer_[pos])));
}

bool Lexer::IsHash(size_t pos)
{
  return ('#' == buffer_[pos]);
}

bool Lexer::IsEndComment(size_t pos)
{
  return (buffer_[pos] == '\n' || buffer_[pos] == '\v' || buffer_[pos] == '\0');
}

std::string Lexer::ExtractTrivia()
{
  size_t start = posBuffer_;
  size_t end = posBuffer_;
  while (IsWhiteSpace(end) || IsHash(end))
  {
    while (IsWhiteSpace(end))
    {
      ++end;
    }
    if (IsHash(end))
    {
      ++end;
      while (!IsEndComment(end))
      {
        ++end;
      }
    }
  }
  return (buffer_.substr(start, end - start));
}

std::string Lexer::TokenKindToString(TokenKind kind)
{
  switch (kind)
  {
    case TokenKind::LBrace:
      return ("LBrace");
    case TokenKind::RBrace:
      return ("RBrace");
    case TokenKind::Semicolon:
      return ("Semicolon");
    case TokenKind::Http:
      return ("Http");
    case TokenKind::Server:
      return ("Server");
    case TokenKind::Location:
      return ("Location");
    case TokenKind::Events:
      return ("Events");
    case TokenKind::String:
      return ("String");
    case TokenKind::Number:
      return ("Number");
    case TokenKind::Eof:
      return ("Eof");
  }
  assert(false && "Invalid Identifier passed to tokenKindToString");
  __builtin_unreachable();
}

void Lexer::MakeToken()
{
  Token token;

  token.leadingTrivia = ExtractTrivia();
  Traverse(token.leadingTrivia);
  token.kind = DetermineKind();
  token.lexeme = DetermineLexeme(token.kind);
  token.line = line_;
  token.col = col_;
  Traverse(token.lexeme);
  token.error = false;
  tokenList_.emplace_back(token);
  if (token.kind == TokenKind::Eof)
  {
    madeEofToken_ = true;
  }
}

bool Lexer::IsLineBreak()
{
  return (buffer_[posBuffer_] == '\n' || buffer_[posBuffer_] == '\v');
}

void Lexer::Advance(size_t steps)
{
  for (size_t i = 0; i < steps; ++i)
  {
    if (IsLineBreak())
    {
      ++line_;
      col_ = 1;
    }
    else
    {
      ++col_;
    }
    ++posBuffer_;
  }
}

void Lexer::Traverse(const std::string& str)
{
  if (!Is('\0'))
  {
    Advance(str.length());
  }
}

void Lexer::PrintToken(const Token& token)
{
  std::cout << "Token:\n";
  if (token.error == true)
  {
    std::cout << kRed_ << "  ERROR" << kReset_ << "\n";
  }
  std::cout << "  TokenKind: " << TokenKindToString(token.kind) << "\n"
            << "  lexeme:    '" << token.lexeme << "'"
            << "\n"
            << "  trivia:    '" << token.leadingTrivia << "'"
            << "\n"
            << "  line:      " << token.line << "\n"
            << "  col:       " << token.col << "\n\n";
}

void Lexer::PrintTriviaAndLexeme(const Token& token)
{
  if (token.error == true)
  {
    std::cout << token.leadingTrivia << kRed_ << token.lexeme << kReset_;
  }
  else
  {
    std::cout << token.leadingTrivia << token.lexeme;
  }
}
