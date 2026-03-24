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
#include <sstream>
#include <string>
#include <vector>

#include "config/Lexer.hpp"

Parser::Parser(Lexer& lexer)
  : lexer_(lexer)
  , currentTokenIdx_{}
  , error_(false)
{}

//////////////////// PUBLIC ////////////////////
void Parser::Parse()
{
  ast_.name = Identifier::Main;
  ast_.context = Identifier::Main;
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
  ast_.idxTokenListEnd = currentTokenIdx_;
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
  return ast_;
}

bool Parser::GetError() const
{
  return error_;
}

std::string Parser::IdentifierToString(Identifier identifier) const
{
  switch (identifier)
  {
    case Identifier::Main:
      return "main";
    case Identifier::Listen:
      return "listen";
    case Identifier::Server_name:
      return "server_name";
    case Identifier::Root:
      return "root";
    case Identifier::Index:
      return "index";
    case Identifier::Alias:
      return "alias";
    case Identifier::Client_max_body_size:
      return "client_max_body_size";
    case Identifier::Client_body_temp_path:
      return "client_body_temp_path";
    case Identifier::Error_page:
      return "error_page";
    case Identifier::Return:
      return "return";
    case Identifier::Allowed_methods:
      return "allowed_methods";
    case Identifier::Autoindex:
      return "autoindex";
    case Identifier::Cgi:
      return "cgi";
    case Identifier::Cgi_root:
      return "cgi_root";
    case Identifier::Cgi_alias:
      return "cgi_alias";
    case Identifier::Cgi_extension:
    	return "cgi_extension";
    case Identifier::Events:
      return "events";
    case Identifier::Http:
      return "http";
    case Identifier::Server:
      return "server";
    case Identifier::Location:
      return "location";
    case Identifier::Param:
      return "param";
  }
  assert(false && "Invalid Identifier passed to IdentifierToString");
  __builtin_unreachable();
}

//////////////////// PRIVATE ////////////////////
//////////////////// helper functions ////////////////////
bool Parser::IsEof() const
{
  return lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Eof;
}

bool Parser::IsBlockDirective() const
{
  return lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Events ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Http ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Server ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Location;
}

bool Parser::IsHttp() const
{
  return lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Http;
}

bool Parser::IsEvents() const
{
  return lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Events;
}

bool Parser::IsServer() const
{
  return lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Server;
}

bool Parser::IsLocation() const
{
  return lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Location;
}

bool Parser::IsLBrace() const
{
  return lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::LBrace;
}

bool Parser::IsRBrace() const
{
  return lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::RBrace;
}

bool Parser::IsDirective() const
{
  return lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Listen ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Server_name ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Root ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Index ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Alias ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Client_max_body_size ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Client_body_temp_path ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Error_page ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Return ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Allowed_methods ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Autoindex ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Cgi ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Cgi_root ||
         lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Cgi_alias ||
  			 lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Cgi_extension;
}

bool Parser::IsSemicolon() const
{
  return lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Semicolon;
}

bool Parser::IsParam() const
{
  return lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::String;
}

Identifier Parser::TokenKindToIdentifier(TokenKind kind) const
{
  switch (kind)
  {
    case TokenKind::Events:
      return Identifier::Events;
    case TokenKind::Http:
      return Identifier::Http;
    case TokenKind::Server:
      return Identifier::Server;
    case TokenKind::Location:
      return Identifier::Location;
    case TokenKind::Listen:
      return Identifier::Listen;
    case TokenKind::Server_name:
      return Identifier::Server_name;
    case TokenKind::Root:
      return Identifier::Root;
    case TokenKind::Index:
      return Identifier::Index;
    case TokenKind::Alias:
      return Identifier::Alias;
    case TokenKind::Client_max_body_size:
      return Identifier::Client_max_body_size;
    case TokenKind::Client_body_temp_path:
      return Identifier::Client_body_temp_path;
    case TokenKind::Error_page:
      return Identifier::Error_page;
    case TokenKind::Return:
      return Identifier::Return;
    case TokenKind::Allowed_methods:
      return Identifier::Allowed_methods;
    case TokenKind::Autoindex:
      return Identifier::Autoindex;
    case TokenKind::Cgi:
      return Identifier::Cgi;
    case TokenKind::Cgi_root:
      return Identifier::Cgi_root;
    case TokenKind::Cgi_alias:
      return Identifier::Cgi_alias;
    case TokenKind::Cgi_extension:
    	return Identifier::Cgi_extension;
    default:
      assert(false && "Invalid TokenKind passed to TokenKindToIdentifier");
      __builtin_unreachable();
  }
}

void Parser::Next()
{
  ++currentTokenIdx_;
}

std::string Parser::BuildErrorMessage(std::string_view errorType, std::string_view expectedMessage)
{
  std::stringstream ss;
  ss << lexer_.GetLine(currentTokenIdx_) << ":" << lexer_.GetCol(currentTokenIdx_) << ": " << kRed_ << "error:" << kReset_ << " " << errorType
     << ": ";
  if (lexer_.GetTokenKind(currentTokenIdx_) == TokenKind::Eof)
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
      directive.params.emplace_back(Node(Identifier::Param, directive.name, lexer_, currentTokenIdx_));
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
    blockDirective.params.emplace_back(Node(Identifier::Param, Identifier::Location, lexer_, currentTokenIdx_));
    Next();
  }
}

Node Parser::ParseBlockDirective(Identifier context)
{
  Node blockDirective(TokenKindToIdentifier(lexer_.GetTokenKind(currentTokenIdx_)), context, lexer_, currentTokenIdx_);
  Next();
  ParseLocationParam(blockDirective);
  ParseBlock(blockDirective);
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
