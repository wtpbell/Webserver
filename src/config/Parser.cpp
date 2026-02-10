/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   Parser.cpp                                          :+:    :+:           */
/*                                                      +:+                   */
/*   By: jstuhrin <jstuhrin@student.ccodam.nl>          +#+                    */
/*                                                    +#+                     */
/*   Created: 2025/12/02 10:37:01 by jstuhrin       #+#    #+#                */
/*   Updated: 2025/12/02 10:37:02 by jstuhrin       ########   codam.nl        */
/*                                                                            */
/* ************************************************************************** */

#include "config/Parser.hpp"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "config/Lexer.hpp"

Parser::Parser(Lexer& lexer) : lexer_(lexer) {}

//////////////////// PUBLIC ////////////////////
void Parser::Parse()
{
  currentToken_ = lexer_.Current();
  ast_.name = Identifier::Main;
  while (!IsEof())
  {
    if (IsDirective())
    {
      ast_.directives.emplace_back(ParseDirective(Identifier::Main));
    }
    else if (IsBlockDirective())
    {
      ast_.nestedBlocks.emplace_back(ParseBlockDirective(Identifier::Main));
    }

    else
    {
      Error("unexpected token", " expected: DIRECTIVE or BLOCKDIRECTIVE", true, ast_);
    }
  }
}

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
  return (ast_);
}

bool Parser::GetError() const
{
  return (error_);
}

std::string Parser::IdentifierToString(Identifier identifier) const
{
  switch (identifier)
  {
    case Identifier::Main:
      return ("main");
    case Identifier::Listen:
      return ("listen");
    case Identifier::Server_name:
      return ("server_name");
    case Identifier::Root:
      return ("root");
    case Identifier::Index:
      return ("index");
    case Identifier::Alias:
      return ("alias");
    case Identifier::Client_max_body_size:
      return ("client_max_body_size");
    case Identifier::Client_body_temp_path:
      return ("client_body_temp_path");
    case Identifier::Error_page:
      return ("error_page");
    case Identifier::Return:
      return ("return");
    case Identifier::Allowed_methods:
      return ("allowed_methods");
    case Identifier::Autoindex:
      return ("autoindex");
    case Identifier::Cgi:
      return ("cgi");
    case Identifier::Events:
      return ("events");
    case Identifier::Http:
      return ("http");
    case Identifier::Server:
      return ("server");
    case Identifier::Location:
      return ("location");
    case Identifier::Param:
      return ("param");
  }
  assert(false && "Invalid Identifier passed to IdentifierToString");
  __builtin_unreachable();
}

//////////////////// PRIVATE ////////////////////
//////////////////// helper functions ////////////////////
bool Parser::IsEof() const
{
  return (currentToken_.kind == TokenKind::Eof);
}

bool Parser::IsBlockDirective() const
{
  return (currentToken_.kind == TokenKind::Events || currentToken_.kind == TokenKind::Http ||
          currentToken_.kind == TokenKind::Server || currentToken_.kind == TokenKind::Location);
}

bool Parser::IsHttp() const
{
  return (currentToken_.kind == TokenKind::Http);
}

bool Parser::IsEvents() const
{
  return (currentToken_.kind == TokenKind::Events);
}

bool Parser::IsServer() const
{
  return (currentToken_.kind == TokenKind::Server);
}

bool Parser::IsLocation() const
{
  return (currentToken_.kind == TokenKind::Location);
}

bool Parser::IsLBrace() const
{
  return (currentToken_.kind == TokenKind::LBrace);
}

bool Parser::IsRBrace() const
{
  return (currentToken_.kind == TokenKind::RBrace);
}

bool Parser::IsDirective() const
{
  return (currentToken_.kind == TokenKind::Listen || currentToken_.kind == TokenKind::Server_name ||
          currentToken_.kind == TokenKind::Root || currentToken_.kind == TokenKind::Index ||
          currentToken_.kind == TokenKind::Alias || currentToken_.kind == TokenKind::Client_max_body_size ||
          currentToken_.kind == TokenKind::Client_body_temp_path || currentToken_.kind == TokenKind::Error_page ||
          currentToken_.kind == TokenKind::Return || currentToken_.kind == TokenKind::Allowed_methods ||
          currentToken_.kind == TokenKind::Autoindex || currentToken_.kind == TokenKind::Cgi);
}

bool Parser::IsSemicolon() const
{
  return (currentToken_.kind == TokenKind::Semicolon);
}

bool Parser::IsParam() const
{
  return (currentToken_.kind == TokenKind::String);
}

Identifier Parser::TokenKindToIdentifier(TokenKind kind) const
{
  switch (kind)
  {
    case TokenKind::Events:
      return (Identifier::Events);
    case TokenKind::Http:
      return (Identifier::Http);
    case TokenKind::Server:
      return (Identifier::Server);
    case TokenKind::Location:
      return (Identifier::Location);
    case TokenKind::Listen:
      return (Identifier::Listen);
    case TokenKind::Server_name:
      return (Identifier::Server_name);
    case TokenKind::Root:
      return (Identifier::Root);
    case TokenKind::Index:
      return (Identifier::Index);
    case TokenKind::Alias:
      return (Identifier::Alias);
    case TokenKind::Client_max_body_size:
      return (Identifier::Client_max_body_size);
    case TokenKind::Client_body_temp_path:
      return (Identifier::Client_body_temp_path);
    case TokenKind::Error_page:
      return (Identifier::Error_page);
    case TokenKind::Return:
      return (Identifier::Return);
    case TokenKind::Allowed_methods:
      return (Identifier::Allowed_methods);
    case TokenKind::Autoindex:
      return (Identifier::Autoindex);
    case TokenKind::Cgi:
      return (Identifier::Cgi);
    default:
      assert(false && "Invalid TokenKind passed to TokenKindToIdentifier");
      __builtin_unreachable();
  }
}

void Parser::Next()
{
  currentToken_ = lexer_.Next();
}

std::string Parser::BuildErrorMessage(std::string_view errorType, std::string_view expectedMessage)
{
  std::stringstream ss;
  ss << currentToken_.line << ":" << currentToken_.col << ": " << kRed_ << "error:" << kReset_ << " "
            << errorType << ": ";
  if (currentToken_.kind == TokenKind::Eof)
  {
    ss << kRed_ << "EOF" << kReset_;
  }
  else
  {
    ss << "`" << kRed_ << currentToken_.lexeme << kReset_ << "`";
  }
  ss << expectedMessage << "\n";
  return (ss.str());
}

void Parser::Error(std::string_view error_type, std::string_view message, bool skip, Node& node)
{
  lexer_.SetTokenErrorMessage(currentToken_.idxTokenList, BuildErrorMessage(error_type, message));
  lexer_.SetTokenErrorTrue(currentToken_.idxTokenList);
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
  Node directive;

  directive.name = TokenKindToIdentifier(currentToken_.kind);
  directive.context = context;
  directive.idxTokenList = currentToken_.idxTokenList;
  directive.lexeme = currentToken_.lexeme;
  directive.line = currentToken_.line;
  directive.col = currentToken_.col;
  Next();
  while (!IsSemicolon() && !IsEof())
  {
    if (!IsParam())
    {
      Error("unexpected token", " expected: PARAMETER", true, directive);
    }
    else
    {
      Node param;
      param.name = Identifier::Param;
      param.context = directive.name;
      param.idxTokenList = currentToken_.idxTokenList;
      param.lexeme = currentToken_.lexeme;
      param.line = currentToken_.line;
      param.col = currentToken_.col;
      directive.params.emplace_back(param);
      Next();
    }
  }
  if (IsSemicolon())
  {
    Next();
  }
  else
  {
    Error("unexpected token", " expected: `;`", true, directive);
  }
  return (directive);
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
      Error("unexpected token", " expected: DIRECTIVE or BLOCKDIRECTIVE", true, blockDirective);
    }
  }
  ParseRBrace(blockDirective);
}

void Parser::ParseLocationParam(Node& blockDirective)
{
  if (blockDirective.name != Identifier::Location)
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
    Node param;
    param.name = Identifier::Param;
    param.context = Identifier::Location;
    param.idxTokenList = currentToken_.idxTokenList;
    param.lexeme = currentToken_.lexeme;
    param.line = currentToken_.line;
    param.col = currentToken_.col;
    blockDirective.params.emplace_back(param);
    Next();
  }
}

Node Parser::ParseBlockDirective(Identifier context)
{
  Node blockDirective;

  blockDirective.name = TokenKindToIdentifier(currentToken_.kind);
  blockDirective.context = context;
  blockDirective.idxTokenList = currentToken_.idxTokenList;
  blockDirective.lexeme = currentToken_.lexeme;
  blockDirective.line = currentToken_.line;
  blockDirective.col = currentToken_.col;
  Next();
  ParseLocationParam(blockDirective);
  ParseBlock(blockDirective);
  return (blockDirective);
}

//////////////////// print detailed AST ////////////////////
std::string Parser::MakeSpaces(std::size_t level) const
{
  std::string spaces;
  for (std::size_t i = 0; i < level; ++i)
  {
    spaces.append("    ");
  }
  return (spaces);
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
            << "   Name: " << IdentifierToString(directive.name) << "   Context: " << IdentifierToString(directive.context) << "\n";
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
