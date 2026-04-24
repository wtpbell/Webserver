/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   Parser.cpp                                          :+:    :+:           */
/*                                                      +:+                   */
/*   By: jstuhrin <jstuhrin@student.codam.nl>          +#+                    */
/*                                                    +#+                     */
/*   Created: 2025/12/02 10:37:01 by jstuhrin       #+#    #+#                */
/*   Updated: 2025/12/02 10:37:02 by jstuhrin      ########   codam.nl        */
/*                                                                            */
/* ************************************************************************** */

#include "config/Parser.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "config/Lexer.hpp"

Parser::Parser(Lexer& lexer)
  : lexer_(lexer)
  , currentTokenIdx_{}
  , error_(false)
  , recursionDepth_(0)
  , recursionTooDeep_(false)
{
  ast_.name = Identifier::kMain;
  ast_.context = Identifier::kMain;
  while (!IsEof())
  {
    if (IsDirective())
    {
      ast_.directives.emplace_back(ParseDirective(Identifier::kMain));
    }
    else if (IsBlockDirective())
    {
      ast_.nestedBlocks.emplace_back(ParseBlockDirective(Identifier::kMain));
    }
    else
    {
      Error("unexpected token", " expected: DIRECTIVE or BLOCKDIRECTIVE or EOF", true, ast_);
    }
  }
  ast_.idxTokenListEnd = currentTokenIdx_;
}

//////////////////// PUBLIC ////////////////////

void Parser::PrintDetailedAST() const
{
  std::size_t level = 0;
  std::cout << "\n###############\n##### AST #####\n###############\n\n";
  if (ast_.error == true)
  {
    std::cout << kRed_ << "ERROR in main >>>\n" << kReset_;
  }
  std::cout << "CONFIGS:\n";
  for (const Node& dir : ast_.directives)
  {
    PrintDirective(dir, level + 1);
  }
  for (const Node& blockDir : ast_.nestedBlocks)
  {
    PrintBlockDirective(blockDir, level + 1);
  }
  if (ast_.error == true)
  {
    std::cout << kRed_ << "<<< ERROR in main\n" << kReset_;
  }
  std::cout << "\n###################\n##### END AST #####\n###################\n\n";
}

Node& Parser::GetAst()
{
  return ast_;
}

bool Parser::GetError() const
{
  return error_;
}

//////////////////// PRIVATE ////////////////////
//////////////////// helper functions ////////////////////
bool Parser::IsEof() const
{
  return lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::kEof;
}

bool Parser::IsBlockDirective() const
{
  return lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::kHttp ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::kServer ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::kLocation;
}

bool Parser::IsHttp() const
{
  return lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::kHttp;
}

bool Parser::IsServer() const
{
  return lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::kServer;
}

bool Parser::IsLocation() const
{
  return lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::kLocation;
}

bool Parser::IsLBrace() const
{
  return lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::kLBrace;
}

bool Parser::IsRBrace() const
{
  return lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::kRBrace;
}

bool Parser::IsDirective() const
{
  return lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::kListen ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::kServerName ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::kRoot ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::kIndex ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::kAlias ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::kClientMaxBodySize ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::kErrorPage ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::kReturn ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::kAllowedMethods ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::kAutoindex ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::kCgi ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::kCgiExtension;
}

bool Parser::IsSemicolon() const
{
  return lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::kSemicolon;
}

bool Parser::IsParam() const
{
  return lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::kString ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::kDefaultServer;
}

Identifier Parser::TokenKindToIdentifier(TokenKind kind) const
{
  switch (kind)
  {
    case TokenKind::kHttp:
      return Identifier::kHttp;
    case TokenKind::kServer:
      return Identifier::kServer;
    case TokenKind::kLocation:
      return Identifier::kLocation;
    case TokenKind::kListen:
      return Identifier::kListen;
    case TokenKind::kServerName:
      return Identifier::kServerName;
    case TokenKind::kRoot:
      return Identifier::kRoot;
    case TokenKind::kIndex:
      return Identifier::kIndex;
    case TokenKind::kAlias:
      return Identifier::kAlias;
    case TokenKind::kClientMaxBodySize:
      return Identifier::kClientMaxBodySize;
    case TokenKind::kErrorPage:
      return Identifier::kErrorPage;
    case TokenKind::kReturn:
      return Identifier::kReturn;
    case TokenKind::kAllowedMethods:
      return Identifier::kAllowedMethods;
    case TokenKind::kAutoindex:
      return Identifier::kAutoindex;
    case TokenKind::kCgi:
      return Identifier::kCgi;
    case TokenKind::kCgiExtension:
      return Identifier::kCgiExtension;
    case TokenKind::kDefaultServer:
      return Identifier::kDefaultServer;
    case TokenKind::kString:
      return Identifier::kParam;
    default:
      assert(false && "Invalid TokenKind passed to TokenKindToIdentifier");
      __builtin_unreachable();
  }
}

std::string Parser::IdentifierToString(Identifier identifier) const
{
  switch (identifier)
  {
    case Identifier::kMain:
      return "main";
    case Identifier::kListen:
      return "listen";
    case Identifier::kServerName:
      return "server_name";
    case Identifier::kRoot:
      return "root";
    case Identifier::kIndex:
      return "index";
    case Identifier::kAlias:
      return "alias";
    case Identifier::kClientMaxBodySize:
      return "client_max_body_size";
    case Identifier::kErrorPage:
      return "error_page";
    case Identifier::kReturn:
      return "return";
    case Identifier::kAllowedMethods:
      return "allowed_methods";
    case Identifier::kAutoindex:
      return "autoindex";
    case Identifier::kCgi:
      return "cgi";
    case Identifier::kCgiExtension:
      return "cgi_extension";
    case Identifier::kDefaultServer:
      return "default_server";
    case Identifier::kHttp:
      return "http";
    case Identifier::kServer:
      return "server";
    case Identifier::kLocation:
      return "location";
    case Identifier::kParam:
      return "param";
  }
  assert(false && "Invalid Identifier passed to IdentifierToString");
  __builtin_unreachable();
}

void Parser::Next()
{
  ++currentTokenIdx_;
}

void Parser::JumpToEof()
{
  currentTokenIdx_ = lexer_.GetSizeTokenList() - 1;
}

std::string Parser::BuildErrorMessage(std::string_view errorType, std::string_view expectedMessage)
{
  std::stringstream ss;
  ss << lexer_.GetLine(currentTokenIdx_) << ":" << lexer_.GetCol(currentTokenIdx_) << ": " << kRed_
     << "error:" << kReset_ << " " << errorType << ": ";
  if (lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::kEof)
  {
    ss << kRed_ << "EOF" << kReset_;
  }
  else
  {
    ss << "`" << kRed_ << lexer_.GetLexeme(currentTokenIdx_) << kReset_ << "`";
  }
  ss << expectedMessage << "\n";
  return ss.str();
}

void Parser::Error(std::string_view error_type, std::string_view message, bool skip, Node& node)
{
  if (recursionTooDeep_)
  {
    return;
  }
  lexer_.SetTokenErrorMessage(currentTokenIdx_, BuildErrorMessage(error_type, message));
  lexer_.SetTokenErrorTrue(currentTokenIdx_);
  node.error = true;
  error_ = true;
  if (skip && !IsEof())
  {
    Next();
  }
}

//////////////////// parsing logic ////////////////////
Node Parser::ParseDirective(Identifier context)
{
  Node directive(TokenKindToIdentifier(lexer_.GetTokenKind(currentTokenIdx_)), context, lexer_, currentTokenIdx_);
  Next();
  while (!IsSemicolon() && !IsEof())
  {
    if (!IsParam())
    {
      Error("unexpected token", " expected: PARAMETER", true, directive);
    }
    else
    {
      directive.params.emplace_back(
          Node(TokenKindToIdentifier(lexer_.GetTokenKind(currentTokenIdx_)), directive.name, lexer_, currentTokenIdx_));
      Next();
    }
  }
  directive.idxTokenListEnd = currentTokenIdx_;
  if (IsSemicolon())
  {
    Next();
  }
  else
  {
    Error("unexpected token", " expected: `;`", true, directive);
  }
  return directive;
}

void Parser::ParseLBrace(Node& blockDirective)
{
  if (IsLBrace())
  {
    Next();
  }
  else
  {
    do
    {
      Error("unexpected token", " expected: `{`", true, blockDirective);
    } while (!IsLBrace() && !IsEof());
    if (IsLBrace())
    {
      Next();
    }
  }
}

void Parser::ParseRBrace(Node& blockDirective)
{
  blockDirective.idxTokenListEnd = currentTokenIdx_;
  if (IsRBrace())
  {
    Next();
  }
  else
  {
    Error("unexpected token", " expected: `}`", true, blockDirective);
  }
}

void Parser::ParseBlock(Node& blockDirective)
{
  ParseLBrace(blockDirective);
  while (!IsRBrace() && !IsEof())
  {
    if (IsDirective())
    {
      blockDirective.directives.emplace_back(ParseDirective(blockDirective.name));
    }
    else if (IsBlockDirective())
    {
      blockDirective.nestedBlocks.emplace_back(ParseBlockDirective(blockDirective.name));
    }
    else
    {
      Error("unexpected token", " expected: DIRECTIVE or BLOCKDIRECTIVE or `}`", true, blockDirective);
    }
  }
  ParseRBrace(blockDirective);
}

void Parser::ParseLocationParam(Node& blockDirective)
{
  if (blockDirective.name != Identifier::kLocation)
  {
    return;
  }
  while (!IsParam())
  {
    if (IsEof() || IsLBrace())
    {
      Error("unexpected token", " expected: PARAMETER", false, blockDirective);
      return;
    }
    Error("unexpected token", " expected: PARAMETER", true, blockDirective);
  }
  if (IsParam())
  {
    blockDirective.params.emplace_back(Node(Identifier::kParam, Identifier::kLocation, lexer_, currentTokenIdx_));
    Next();
  }
}

Node Parser::ParseBlockDirective(Identifier context)
{
  Node blockDirective(TokenKindToIdentifier(lexer_.GetTokenKind(currentTokenIdx_)), context, lexer_, currentTokenIdx_);
  ++recursionDepth_;
  if (recursionDepth_ > 42)
  {
    Error("unexpected token", " blocks nested too deep", false, blockDirective);
    recursionTooDeep_ = true;
    JumpToEof();
    return blockDirective;
  }
  Next();
  ParseLocationParam(blockDirective);
  ParseBlock(blockDirective);
  --recursionDepth_;
  return blockDirective;
}

//////////////////// print detailed AST ////////////////////
std::string Parser::MakeSpaces(std::size_t level) const
{
  std::string spaces;
  for (std::size_t i = 0; i < level; ++i)
  {
    spaces.append("    ");
  }
  return spaces;
}

void Parser::PrintParam(const Node& param, std::size_t level) const
{
  std::string spaces = MakeSpaces(level);
  if (param.error == true)
  {
    std::cout << "\n" << spaces << kRed_ << "ERROR >>>" << kReset_;
  }
  std::cout << "\n" << spaces << "PARAM:   " << param.lexeme << "\n";
  if (param.error == true)
  {
    std::cout << spaces << kRed_ << "<<< ERROR\n" << kReset_;
  }
}

void Parser::PrintDirective(const Node& directive, std::size_t level) const
{
  std::string spaces = MakeSpaces(level);
  if (directive.error == true)
  {
    std::cout << "\n" << spaces << kRed_ << "ERROR >>>" << kReset_;
  }
  std::cout << "\n"
            << spaces << "DIRECTIVE:"
            << "   Name: " << IdentifierToString(directive.name)
            << "   Context: " << IdentifierToString(directive.context) << "\n";
  for (const Node& param : directive.params)
  {
    PrintParam(param, level + 1);
  }
  if (directive.error == true)
  {
    std::cout << spaces << kRed_ << "<<< ERROR\n" << kReset_;
  }
}

void Parser::PrintBlockDirective(const Node& blockDirective, std::size_t level) const
{
  std::string spaces = MakeSpaces(level);
  if (blockDirective.error == true)
  {
    std::cout << "\n" << spaces << kRed_ << "ERROR in " << IdentifierToString(blockDirective.name) << " >>>" << kReset_;
  }
  std::cout << "\n"
            << spaces << "BLOCKDIRECTIVE:"
            << "   Name: " << IdentifierToString(blockDirective.name)
            << "   Context: " << IdentifierToString(blockDirective.context) << "\n";
  for (const Node& param : blockDirective.params)
  {
    PrintParam(param, level + 1);
  }
  for (const Node& dir : blockDirective.directives)
  {
    PrintDirective(dir, level + 1);
  }
  for (const Node& blockDir : blockDirective.nestedBlocks)
  {
    PrintBlockDirective(blockDir, level + 1);
  }
  if (blockDirective.error == true)
  {
    std::cout << spaces << kRed_ << "<<< ERROR in " << IdentifierToString(blockDirective.name) << kReset_ << "\n";
  }
}
