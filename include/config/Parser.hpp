/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   Parser.hpp                                          :+:    :+:           */
/*                                                      +:+                   */
/*   By: jstuhrin <jstuhrin@student.codam.nl>          +#+                    */
/*                                                    +#+                     */
/*   Created: 2025/12/02 10:35:32 by jstuhrin       #+#    #+#                */
/*   Updated: 2025/12/02 10:35:34 by jstuhrin       ########   odam.nl        */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>

#include "Lexer.hpp"

enum class Identifier
{
  Main,
  Directive,
  Events,
  Http,
  Server,
  Location
};

struct Directive
{
    std::string name;
    Identifier context;
    std::vector<std::string> params;
    bool error;
};

struct BlockDirective
{
    Identifier name;
    Identifier context;
    std::vector<std::string> params;
    std::vector<Directive> directives;
    std::vector<BlockDirective> nestedBlocks;
    bool error;
};

class Parser
{
  public:
    Parser(Lexer& lexer);
    Parser(const Parser& other) = delete;
    Parser(Parser&& other) noexcept = delete;
    Parser& operator=(const Parser& other) = delete;
    Parser& operator=(Parser&& other) noexcept = delete;
    ~Parser() = default;

    void Parse(void);
    void PrintDetailedAST(void);

  private:
    // helper functions
    bool IsEof(void);
    bool IsBlockDirective(void);
    bool IsHttp(void);
    bool IsEvents(void);
    bool IsServer(void);
    bool IsLocation(void);
    bool IsLBrace(void);
    bool IsRBrace(void);
    bool IsDirective(void);
    bool IsSemicolon(void);
    bool IsParam(void);
    Identifier TokenKindToIdentifier(TokenKind kind);
    void Next(void);
    void PrintError(std::string_view errorType, std::string_view expectedMessage = "");
    void Error(std::string_view error_type, std::string_view message, bool skip);
    void ContextError(BlockDirective& blockDirective);
    void ValidateContext(BlockDirective& blockDirective);
    void ValidateNumberOfEventsAndHtpp(BlockDirective& blockDirective);

    // parsing logic
    Directive ParseDirective(Identifier context);
    void ParseLBrace(BlockDirective& blockDirective);
    void ParseRBrace(BlockDirective& blockDirective);
    void ParseBlock(BlockDirective& blockDirective);
    void ParseLocationParam(BlockDirective& blockDirective);
    BlockDirective ParseBlockDirective(Identifier context);

    // printAST
    std::string MakeSpaces(size_t level);
    std::string IdentifierToString(Identifier identifier);
    void PrintParams(const std::vector<std::string>& params, size_t level);
    void PrintDirective(const Directive& directive, size_t level);
    void PrintBlockDirective(const BlockDirective& blockDirective, size_t level);

    BlockDirective configs_;
    Lexer& lexer_;
    Token currentToken_;
    int num_events_;
    int num_http_;
    static constexpr std::string_view kRed_ = "\033[31m";
    static constexpr std::string_view kReset_ = "\033[0m";
};

#endif
