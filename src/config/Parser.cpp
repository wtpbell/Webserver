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

#include "config/Lexer.hpp"

Parser::Parser(Lexer& lexer) : lexer_(lexer), num_events_(0), num_http_(0)
{
  currentToken_ = lexer_.Current();
}

//  PUBLIC
void Parser::Parse()
{
  configs_.name = Identifier::Main;
  configs_.error = false;
  while (!IsEof())
  {
    if (IsDirective())
    {
      configs_.directives.emplace_back(ParseDirective(Identifier::Main));
    }
    else if (IsBlockDirective())
    {
      ValidateNumberOfEventsAndHtpp(configs_);
      ValidateContext(configs_);
      configs_.nestedBlocks.emplace_back(ParseBlockDirective(Identifier::Main));
    }

    else
    {
      Error("unexpected token", " expected: DIRECTIVE or BLOCKDIRECTIVE", true);
      configs_.error = true;
    }
  }
}

void Parser::PrintDetailedAST()
{
  size_t level = 0;
  std::cout << "\n###############\n##### AST #####\n###############\n\n";
  if (configs_.error == true)
  {
    std::cout << kRed_ << "ERROR in main >>>\n" << kReset_;
  }
  std::cout << "CONFIGS:\n";
  for (auto dir : configs_.directives)
  {
    PrintDirective(dir, level + 1);
  }
  for (auto blockDir : configs_.nestedBlocks)
  {
    PrintBlockDirective(blockDir, level + 1);
  }
  if (configs_.error == true)
  {
    std::cout << kRed_ << "<<< ERROR in main\n" << kReset_;
  }
  std::cout << "\n###################\n##### END AST #####\n###################\n\n";
}

// PRIVATE
// helper functions
bool Parser::IsEof()
{
  return (currentToken_.kind == TokenKind::Eof);
}

bool Parser::IsBlockDirective()
{
  return (currentToken_.kind == TokenKind::Events || currentToken_.kind == TokenKind::Http ||
          currentToken_.kind == TokenKind::Server || currentToken_.kind == TokenKind::Location);
}

bool Parser::IsHttp()
{
  return (currentToken_.kind == TokenKind::Http);
}

bool Parser::IsEvents()
{
  return (currentToken_.kind == TokenKind::Events);
}

bool Parser::IsServer()
{
  return (currentToken_.kind == TokenKind::Server);
}

bool Parser::IsLocation()
{
  return (currentToken_.kind == TokenKind::Location);
}

bool Parser::IsLBrace()
{
  return (currentToken_.kind == TokenKind::LBrace);
}

bool Parser::IsRBrace()
{
  return (currentToken_.kind == TokenKind::RBrace);
}

bool Parser::IsDirective()
{
  return (currentToken_.kind == TokenKind::String);
}

bool Parser::IsSemicolon()
{
  return (currentToken_.kind == TokenKind::Semicolon);
}

bool Parser::IsParam()
{
  return (currentToken_.kind == TokenKind::String || currentToken_.kind == TokenKind::Number);
}

Identifier Parser::TokenKindToIdentifier(TokenKind kind)
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
    default:
      assert(false && "Invalid TokenKind passed to kindToIdentifier");
      __builtin_unreachable();
  }
}

void Parser::Next()
{
  currentToken_ = lexer_.Next();
}

void Parser::PrintError(std::string_view errorType, std::string_view expectedMessage)
{
  std::cerr << currentToken_.line << ":" << currentToken_.col << ": " << kRed_ << "error:" << kReset_ << " "
            << errorType << ": ";
  if (currentToken_.kind == TokenKind::Eof)
  {
    std::cerr << kRed_ << "EOF" << kReset_;
  }
  else
  {
    std::cerr << "`" << kRed_ << currentToken_.lexeme << kReset_ << "`";
  }
  std::cerr << expectedMessage << "\n";
}

void Parser::Error(std::string_view error_type, std::string_view message, bool skip)
{
  PrintError(error_type, message);
  lexer_.SetTokenErrorTrue();
  if (skip && !IsEof())
  {
    Next();
  }
}

void Parser::ContextError(BlockDirective& blockDirective)
{
  Error("incorrect context", " not allowed in context " + IdentifierToString(blockDirective.name), false);
  blockDirective.error = true;
}

void Parser::ValidateContext(BlockDirective& blockDirective)
{
  switch (blockDirective.name)
  {
    case Identifier::Main:
    {
      if (!IsEvents() && !IsHttp())
      {
        ContextError(blockDirective);
      }
      break;
    }
    case Identifier::Events:
    {
      ContextError(blockDirective);
      break;
    }
    case Identifier::Http:
    {
      if (!IsServer())
      {
        ContextError(blockDirective);
      }
      break;
    }
    case Identifier::Server:
    {
      if (!IsLocation())
      {
        ContextError(blockDirective);
      }
      break;
    }
    case Identifier::Location:
    {
      ContextError(blockDirective);
      break;
    }
    default:
      assert(false && "Invalid Identifier passed to is_valid_context");
      __builtin_unreachable();
  }
}

void Parser::ValidateNumberOfEventsAndHtpp(BlockDirective& blockDirective)
{
  if (IsEvents())
  {
    ++num_events_;
    if (num_events_ > 1)
    {
      Error("duplicate `events`", "", false);
      blockDirective.error = true;
      return;
    }
  }
  if (IsHttp())
  {
    ++num_http_;
    if (num_http_ > 1)
    {
      Error("duplicate `http`", "", false);
      blockDirective.error = true;
      return;
    }
  }
}

// parsing logic
Directive Parser::ParseDirective(Identifier context)
{
  Directive directive;

  directive.name = currentToken_.lexeme;
  directive.context = context;
  directive.error = false;
  Next();
  while (!IsSemicolon() && !IsEof())
  {
    if (!IsParam())
    {
      Error("unexpected token", " expected: PARAMETER", true);
      directive.error = true;
    }
    else
    {
      directive.params.emplace_back(currentToken_.lexeme);
      Next();
    }
  }
  if (IsSemicolon())
  {
    Next();
  }
  else
  {
    Error("unexpected token", " expected: `;`", true);
    directive.error = true;
  }
  return (directive);
}

void Parser::ParseLBrace(BlockDirective& blockDirective)
{
  if (IsLBrace())
  {
    Next();
  }
  else
  {
    blockDirective.error = true;
    do
    {
      Error("unexpected token", " expected: `{`", true);
    } while (!IsLBrace() && !IsEof());
    if (IsLBrace())
    {
      Next();
    }
  }
}

void Parser::ParseRBrace(BlockDirective& blockDirective)
{
  if (IsRBrace())
  {
    Next();
  }
  else
  {
    blockDirective.error = true;
    Error("unexpected token", " expected: `}`", true);
  }
}

void Parser::ParseBlock(BlockDirective& blockDirective)
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
      ValidateContext(blockDirective);
      ValidateNumberOfEventsAndHtpp(blockDirective);
      blockDirective.nestedBlocks.emplace_back(ParseBlockDirective(blockDirective.name));
    }
    else
    {
      Error("unexpected token", " expected: DIRECTIVE or BLOCKDIRECTIVE", true);
      blockDirective.error = true;
    }
  }
  ParseRBrace(blockDirective);
}

void Parser::ParseLocationParam(BlockDirective& blockDirective)
{
  if (blockDirective.name != Identifier::Location)
  {
    return;
  }
  while (!IsParam())
  {
    if (IsEof() || IsLBrace())
    {
      Error("unexpected token", " expected: PARAMETER", false);
      blockDirective.error = true;
      return;
    }
    Error("unexpected token", " expected: PARAMETER", true);
  }
  if (IsParam())
  {
    blockDirective.params.emplace_back(currentToken_.lexeme);
    Next();
  }
}

BlockDirective Parser::ParseBlockDirective(Identifier context)
{
  BlockDirective blockDirective;

  blockDirective.error = false;
  blockDirective.name = TokenKindToIdentifier(currentToken_.kind);
  blockDirective.context = context;
  Next();
  ParseLocationParam(blockDirective);
  ParseBlock(blockDirective);
  return (blockDirective);
}

// print detailed AST
std::string Parser::MakeSpaces(size_t level)
{
  std::string spaces;
  for (size_t i = 0; i < level; ++i)
  {
    spaces.append("    ");
  }
  return (spaces);
}

std::string Parser::IdentifierToString(Identifier identifier)
{
  switch (identifier)
  {
    case Identifier::Main:
      return ("main");
    case Identifier::Directive:
      return ("directive");
    case Identifier::Events:
      return ("events");
    case Identifier::Http:
      return ("http");
    case Identifier::Server:
      return ("server");
    case Identifier::Location:
      return ("location");
  }
  assert(false && "Invalid Identifier passed to identifierToString");
  __builtin_unreachable();
}

void Parser::PrintParams(const std::vector<std::string>& params, size_t level)
{
  std::string spaces = MakeSpaces(level);
  for (auto param : params)
  {
    std::cout << spaces << "PARAM:   " << param << "\n";
  }
}

void Parser::PrintDirective(const Directive& directive, size_t level)
{
  std::string spaces = MakeSpaces(level);
  if (directive.error == true)
  {
    std::cout << "\n" << spaces << kRed_ << "ERROR >>>" << kReset_;
  }
  std::cout << "\n"
            << spaces << "DIRECTIVE:"
            << "   Name: " << directive.name << "   Context: " << IdentifierToString(directive.context) << "\n";
  PrintParams(directive.params, level + 1);
  if (directive.error == true)
  {
    std::cout << spaces << kRed_ << "<<< ERROR\n" << kReset_;
  }
}

void Parser::PrintBlockDirective(const BlockDirective& blockDirective, size_t level)
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
  PrintParams(blockDirective.params, level + 1);
  for (auto dir : blockDirective.directives)
  {
    PrintDirective(dir, level + 1);
  }
  for (auto blockDir : blockDirective.nestedBlocks)
  {
    PrintBlockDirective(blockDir, level + 1);
  }
  if (blockDirective.error == true)
  {
    std::cout << spaces << kRed_ << "<<< ERROR in " << IdentifierToString(blockDirective.name) << kReset_ << "\n";
  }
}
