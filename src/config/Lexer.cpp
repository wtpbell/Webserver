/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   Lexer.cpp                                           :+:    :+:           */
/*                                                      +:+                   */
/*   By: jstuhrin <jstuhrin@student.codam.nl>          +#+                    */
/*                                                    +#+                     */
/*   Created: 2025/12/02 10:36:50 by jstuhrin       #+#    #+#                */
/*   Updated: 2025/12/02 10:36:51 by jstuhrin       ########   codam.nl       */
/*                                                                            */
/* ************************************************************************** */

#include "config/Lexer.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

Lexer::Lexer(std::string buffer)
    : line_(1),
      col_(1),
      posBuffer_(0),
      idxCurrentToken_(0),
      madeEofToken_(false),
      error_(false),
      buffer_(std::move(buffer))
{
  while (!madeEofToken_)
  {
    MakeToken();
    ++idxCurrentToken_;
  }
  idxCurrentToken_ = 0;
}

//////////////////// PUBLIC ////////////////////

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

void Lexer::SetTokenErrorTrue(std::size_t idxTokenList)
{
  tokenList_[idxTokenList].error = true;
}

TokenKind Lexer::GetTokenKind(std::size_t idxTokenList) const
{
  return tokenList_[idxTokenList].kind;
}

bool Lexer::GetTokenError(std::size_t idxTokenList) const
{
  return tokenList_[idxTokenList].error;
}

std::size_t Lexer::GetLine(std::size_t idxTokenList) const
{
  return tokenList_[idxTokenList].line;
}

std::size_t Lexer::GetCol(std::size_t idxTokenList) const
{
  return tokenList_[idxTokenList].col;
}

std::string_view Lexer::GetLexeme(std::size_t idxTokenList) const
{
  return tokenList_[idxTokenList].lexeme;
}

std::string_view Lexer::GetTrivia(std::size_t idxTokenList) const
{
  return tokenList_[idxTokenList].leadingTrivia;
}

std::size_t Lexer::GetSizeTokenList() const
{
  return tokenList_.size();
}

bool Lexer::GetError() const
{
  return error_;
}

void Lexer::SetTokenErrorMessage(std::size_t idx, const std::string& errorMessage)
{
  tokenList_[idx].errorMessage.append(errorMessage);
}

std::string_view Lexer::GetTokenErrorMessage(std::size_t idx) const
{
  return tokenList_[idx].errorMessage;
}

//////////////////// PRIVATE ////////////////////
bool Lexer::Is(const char c) const
{
  return buffer_[posBuffer_] == c;
}

bool Lexer::Is(const std::string& lexeme) const
{
  return !buffer_.compare(posBuffer_, lexeme.length(), lexeme);
}

bool Lexer::IsSeparator(const char c) const
{
  return std::isspace(static_cast<unsigned char>(c)) || c == '{' || c == '}' || c == ';' || c == '#' || c == '\0';
}

std::size_t Lexer::LexemeLen() const
{
  if (Is('{') || Is('}') || Is(';') || Is('\0'))
  {
    return 1;
  }
  std::size_t len = 1;
  while (!IsSeparator(buffer_[posBuffer_ + len]))
  {
    ++len;
  }
  return len;
}

TokenKind Lexer::DetermineKind() const
{
  std::size_t len = LexemeLen();
  switch (len)
  {
    case 1:
      if (Is('{'))
        return TokenKind::kLBrace;
      if (Is('}'))
        return TokenKind::kRBrace;
      if (Is(';'))
        return TokenKind::kSemicolon;
      if (Is('\0'))
        return TokenKind::kEof;
      break;
    case 3:
      if (Is("cgi"))
        return TokenKind::kCgi;
      break;
    case 4:
      if (Is("root"))
        return TokenKind::kRoot;
      if (Is("http"))
        return TokenKind::kHttp;
      break;
    case 5:
      if (Is("index"))
        return TokenKind::kIndex;
      if (Is("alias"))
        return TokenKind::kAlias;
      break;
    case 6:
      if (Is("listen"))
        return TokenKind::kListen;
      if (Is("return"))
        return TokenKind::kReturn;
      if (Is("server"))
        return TokenKind::kServer;
      break;
    case 8:
      if (Is("location"))
        return TokenKind::kLocation;
      break;
    case 9:
      if (Is("autoindex"))
        return TokenKind::kAutoindex;
      break;
    case 10:
      if (Is("error_page"))
        return TokenKind::kErrorPage;
      break;
    case 11:
      if (Is("server_name"))
        return TokenKind::kServerName;
      break;
    case 13:
      if (Is("cgi_extension"))
        return TokenKind::kCgiExtension;
      break;
    case 14:
      if (Is("default_server"))
        return TokenKind::kDefaultServer;
      break;
    case 15:
      if (Is("allowed_methods"))
        return TokenKind::kAllowedMethods;
      break;
    case 20:
      if (Is("client_max_body_size"))
        return TokenKind::kClientMaxBodySize;
      break;
  }
  return TokenKind::kString;
}

std::string Lexer::ExtractLexeme() const
{
  return buffer_.substr(posBuffer_, LexemeLen());
}

bool Lexer::IsWhiteSpace(const std::size_t pos) const
{
  return std::isspace(static_cast<unsigned char>(buffer_[pos]));
}

bool Lexer::IsHash(const std::size_t pos) const
{
  return '#' == buffer_[pos];
}

bool Lexer::IsEndComment(const std::size_t pos) const
{
  return buffer_[pos] == '\n' || buffer_[pos] == '\v' || buffer_[pos] == '\0';
}

std::string Lexer::ExtractTrivia() const
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
  return buffer_.substr(start, end - start);
}

std::string Lexer::TokenKindToString(TokenKind kind) const
{
  switch (kind)
  {
    case TokenKind::kLBrace:
      return "LBrace";
    case TokenKind::kRBrace:
      return "RBrace";
    case TokenKind::kSemicolon:
      return "Semicolon";
    case TokenKind::kListen:
      return "Listen";
    case TokenKind::kServerName:
      return "Server_name";
    case TokenKind::kRoot:
      return "Root";
    case TokenKind::kIndex:
      return "Index";
    case TokenKind::kAlias:
      return "Alias";
    case TokenKind::kClientMaxBodySize:
      return "Client_max_body_size";
    case TokenKind::kErrorPage:
      return "Error_page";
    case TokenKind::kReturn:
      return "Return";
    case TokenKind::kAllowedMethods:
      return "Allowed_methods";
    case TokenKind::kAutoindex:
      return "Autoindex";
    case TokenKind::kCgi:
      return "Cgi";
    case TokenKind::kCgiExtension:
      return "Cgi_extension";
    case TokenKind::kDefaultServer:
      return "Default_server";
    case TokenKind::kHttp:
      return "Http";
    case TokenKind::kServer:
      return "Server";
    case TokenKind::kLocation:
      return "Location";
    case TokenKind::kString:
      return "String";
    case TokenKind::kEof:
      return "Eof";
  }
  assert(false && "passed invalid TokenKind");
  __builtin_unreachable();
}

void Lexer::ValidatePrintable(Token& token)
{
  for (const unsigned char c : token.lexeme)
  {
    if (!std::isprint(c))
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
  std::string leadingTrivia = ExtractTrivia();
  Traverse(leadingTrivia);
  TokenKind kind = DetermineKind();
  std::string lexeme = ExtractLexeme();
  Token token(kind, std::move(lexeme), std::move(leadingTrivia), line_, col_, idxCurrentToken_);
  Traverse(token.lexeme);
  if (token.kind == TokenKind::kString)
  {
    ValidatePrintable(token);
  }
  if (token.kind == TokenKind::kEof)
  {
    madeEofToken_ = true;
  }
  tokenList_.emplace_back(std::move(token));
}

bool Lexer::IsLineBreak() const
{
  return buffer_[posBuffer_] == '\n' || buffer_[posBuffer_] == '\v';
}

void Lexer::Traverse(const std::string& str)
{
  if (Is('\0'))
  {
    return;
  }
  for (std::size_t i = 0; i < str.length(); ++i)
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
            << "  error message: '" << token.errorMessage << "'\n\n";
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
