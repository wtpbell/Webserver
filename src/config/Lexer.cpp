/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   Lexer.cpp                                           :+:    :+:           */
/*                                                      +:+                   */
/*   By: jstuhrin <jstuhrin@student.ccodam.nl>          +#+                    */
/*                                                    +#+                     */
/*   Created: 2025/12/02 10:36:50 by jstuhrin       #+#    #+#                */
/*   Updated: 2025/12/02 10:36:51 by jstuhrin       ########   codam.nl        */
/*                                                                            */
/* ************************************************************************** */

#include "config/Lexer.hpp"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

Lexer::Lexer() {}

//////////////////// PUBLIC ////////////////////
void Lexer::Lex(std::string buffer)
{
  buffer_ = buffer;
  while (!madeEofToken_)
  {
    MakeToken();
    ++idxCurrentToken_;
  }
  idxCurrentToken_ = 0;
}

void Lexer::PrintTokenList() const
{
  std::cout << "\n#####################\n##### TOKENLIST #####\n#####################\n\n";
  for (const Token& token : tokenList_)
  {
    PrintToken(token);
  }
  std::cout << "#########################\n##### END TOKENLIST #####\n#########################\n\n";
}

void Lexer::PrintConfigsDebug() const
{
  std::cout << "\n##############################\n##### CONFIGS FILE DEBUG #####\n##############################\n\n";
  for (const Token& token : tokenList_)
  {
    PrintTriviaAndLexeme(token);
  }
  std::cout << "\n\n##################################\n##### END CONFIGS FILE DEBUG "
               "#####\n##################################\n\n";
}

void Lexer::PrintErrorIdxs() const
{
  std::cout << "\nIndexes of tokens flagged as ERROR:\n";
  bool isFirst = true;
  for (const Token& token : tokenList_)
  {
    if (token.error == true)
    {
      if (isFirst)
      {
        isFirst = false;
      }
      else
      {
        std::cout << ", ";
      }
      std::cout << token.idxTokenList;
    }
  }
  std::cout << "\n";
}

void Lexer::PrintErrorMessages() const
{
  for (const Token& token : tokenList_)
  {
    std::cerr << token.errorMessage;
  }
}

Token Lexer::Next()
{
  return (tokenList_[++idxCurrentToken_]);
}

Token Lexer::Current() const
{
  return (tokenList_[idxCurrentToken_]);
}

void Lexer::SetTokenErrorTrue(std::size_t idxTokenList)
{
  tokenList_[idxTokenList].error = true;
}

bool Lexer::GetTokenError(std::size_t idxTokenList) const
{
  return (tokenList_[idxTokenList].error);
}

std::size_t Lexer::GetLine(std::size_t idxTokenList) const
{
  return (tokenList_[idxTokenList].line);
}

std::size_t Lexer::GetCol(std::size_t idxTokenList) const
{
  return (tokenList_[idxTokenList].col);
}

std::string_view Lexer::GetLexeme(std::size_t idxTokenList) const
{
  return (tokenList_[idxTokenList].lexeme);
}

std::size_t Lexer::GetSizeTokenList() const
{
  return tokenList_.size();
}

bool Lexer::GetError() const
{
  return (error_);
}

void Lexer::SetTokenErrorMessage(std::size_t idx, const std::string& errorMessage)
{
  tokenList_[idx].errorMessage.append(errorMessage);
}

std::string_view Lexer::GetTokenErrorMessage(std::size_t idx) const
{
  return (tokenList_[idx].errorMessage);
}

//////////////////// PRIVATE ////////////////////
bool Lexer::Is(const char c) const
{
  return (buffer_[posBuffer_] == c);
}

bool Lexer::Is(const std::string& lexeme) const
{
  return (!buffer_.compare(posBuffer_, lexeme.length(), lexeme) && IsSeparator(buffer_[posBuffer_ + lexeme.length()]));
}

bool Lexer::IsSeparator(const char c) const
{
  return (std::isspace(static_cast<unsigned char>(c)) || c == '{' || c == '}' || c == ';' || c == '#' || c == '\0');
}

TokenKind Lexer::DetermineKind() const
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
  if (Is("listen"))
  {
    return (TokenKind::Listen);
  }
  if (Is("server_name"))
  {
    return (TokenKind::Server_name);
  }
  if (Is("root"))
  {
    return (TokenKind::Root);
  }
  if (Is("index"))
  {
    return (TokenKind::Index);
  }
  if (Is("alias"))
  {
    return (TokenKind::Alias);
  }
  if (Is("client_max_body_size"))
  {
    return (TokenKind::Client_max_body_size);
  }
  if (Is("client_body_temp_path"))
  {
    return (TokenKind::Client_body_temp_path);
  }
  if (Is("error_page"))
  {
    return (TokenKind::Error_page);
  }
  if (Is("return"))
  {
    return (TokenKind::Return);
  }
  if (Is("allowed_methods"))
  {
    return (TokenKind::Allowed_methods);
  }
  if (Is("autoindex"))
  {
    return (TokenKind::Autoindex);
  }
  if (Is("cgi"))
  {
    return (TokenKind::Cgi);
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
  if (Is('\0'))
  {
    return (TokenKind::Eof);
  }
  return (TokenKind::String);
}

std::string Lexer::ExtractLexeme()
{
  std::size_t start = posBuffer_;
  std::size_t end = posBuffer_;
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
    case TokenKind::Listen:
      return ("listen");
    case TokenKind::Server_name:
      return ("server_name");
    case TokenKind::Root:
      return ("root");
    case TokenKind::Index:
      return ("index");
    case TokenKind::Alias:
      return ("alias");
    case TokenKind::Client_max_body_size:
      return ("client_max_body_size");
    case TokenKind::Client_body_temp_path:
      return ("client_body_temp_path");
    case TokenKind::Error_page:
      return ("error_page");
    case TokenKind::Return:
      return ("return");
    case TokenKind::Allowed_methods:
      return ("allowed_methods");
    case TokenKind::Autoindex:
      return ("autoindex");
    case TokenKind::Cgi:
      return ("cgi");
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
    case TokenKind::Eof:
      return ("\0");
  }
  assert(false && "passed invalid TokenKind");
  __builtin_unreachable();
}

bool Lexer::IsWhiteSpace(const std::size_t pos) const
{
  return (std::isspace(static_cast<unsigned char>(buffer_[pos])));
}

bool Lexer::IsHash(const std::size_t pos) const
{
  return ('#' == buffer_[pos]);
}

bool Lexer::IsEndComment(const std::size_t pos) const
{
  return (buffer_[pos] == '\n' || buffer_[pos] == '\v' || buffer_[pos] == '\0');
}

std::string Lexer::ExtractTrivia()
{
  std::size_t start = posBuffer_;
  std::size_t end = posBuffer_;
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

std::string Lexer::TokenKindToString(TokenKind kind) const
{
  switch (kind)
  {
    case TokenKind::LBrace:
      return ("LBrace");
    case TokenKind::RBrace:
      return ("RBrace");
    case TokenKind::Semicolon:
      return ("Semicolon");
    case TokenKind::Listen:
      return ("Listen");
    case TokenKind::Server_name:
      return ("Server_name");
    case TokenKind::Root:
      return ("Root");
    case TokenKind::Index:
      return ("Index");
    case TokenKind::Alias:
      return ("Alias");
    case TokenKind::Client_max_body_size:
      return ("Client_max_body_size");
    case TokenKind::Client_body_temp_path:
      return ("Client_body_temp_path");
    case TokenKind::Error_page:
      return ("Error_page");
    case TokenKind::Return:
      return ("Return");
    case TokenKind::Allowed_methods:
      return ("Allowed_methods");
    case TokenKind::Autoindex:
      return ("Autoindex");
    case TokenKind::Cgi:
      return ("Cgi");
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
    case TokenKind::Eof:
      return ("Eof");
  }
  assert(false && "passed invalid TokenKind");
  __builtin_unreachable();
}

void Lexer::ValidatePrintable(Token& token)
{
  for (const unsigned char c : token.lexeme)
  {
    if (c < 32 || c > 126)
    {
      std::stringstream ss;

      ss << token.line << ":" << token.col << ": " << kRed_ << "error:" << kReset_ << " "
            << "non-printable character: " << "`" << kRed_ << token.lexeme << kReset_ << "`\n";
      token.error = true;
      token.errorMessage.append(ss.str());
      error_ = true;
      return;
    }
  }
}

void Lexer::MakeToken()
{
  Token token;

  token.idxTokenList = idxCurrentToken_;
  token.leadingTrivia = ExtractTrivia();
  Traverse(token.leadingTrivia);
  token.kind = DetermineKind();
  token.lexeme = DetermineLexeme(token.kind);
  token.line = line_;
  token.col = col_;
  Traverse(token.lexeme);
  token.error = false;
  if (token.kind == TokenKind::String)
  {
    ValidatePrintable(token);
  }
  tokenList_.emplace_back(token);
  if (token.kind == TokenKind::Eof)
  {
    madeEofToken_ = true;
  }
}

bool Lexer::IsLineBreak() const
{
  return (buffer_[posBuffer_] == '\n' || buffer_[posBuffer_] == '\v');
}

void Lexer::Advance(const std::size_t steps)
{
  for (std::size_t i = 0; i < steps; ++i)
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

void Lexer::PrintToken(const Token& token) const
{
  std::cout << "Token:\n";
  if (token.error == true)
  {
    std::cout << kRed_ << "  ERROR" << kReset_ << "\n";
  }
  std::cout << "  TokenKind:     " << TokenKindToString(token.kind) << "\n"
            << "  lexeme:        '" << token.lexeme << "'"
            << "\n"
            << "  trivia:        '" << token.leadingTrivia << "'"
            << "\n"
            << "  line:          " << token.line << "\n"
            << "  col:           " << token.col << "\n"
            << "  idx:           " << token.idxTokenList << "\n"
            << "  error message: " << token.errorMessage << "\n\n";
}

void Lexer::PrintTriviaAndLexeme(const Token& token) const
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
