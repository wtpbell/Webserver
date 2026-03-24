/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   Parser.hpp                                          :+:    :+:           */
/*                                                      +:+                   */
/*   By: jstuhrin <jstuhrin@student.ccodam.nl>          +#+                    */
/*                                                    +#+                     */
/*   Created: 2025/12/02 10:35:32 by jstuhrin       #+#    #+#                */
/*   Updated: 2025/12/02 10:35:34 by jstuhrin       ########   codam.nl        */
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
  Events,
  Http,
  Server,
  Location,
  Listen,
  Server_name,
  Root,
  Index,
  Alias,
  Client_max_body_size,
  Client_body_temp_path,
  Error_page,
  Return,
  Allowed_methods,
  Autoindex,
  Cgi,
  Cgi_root,
  Cgi_alias,
  Cgi_extension,
  Param
};

struct Node
{
  Identifier name;
  Identifier context;
  std::string lexeme;
  std::vector<Node> params;
  std::vector<Node> directives;
  std::vector<Node> nestedBlocks;
  std::size_t idxTokenListStart;
  std::size_t idxTokenListEnd;
  bool error;

  Node(Identifier name, Identifier context, const Lexer& lexer, std::size_t currentTokenIdx)
    : name(name)
    , context(context)
    , lexeme(lexer.GetLexeme(currentTokenIdx))
    , idxTokenListStart(currentTokenIdx)
    , idxTokenListEnd(currentTokenIdx)
    , error(false)
  {}

  Node()
    : idxTokenListStart(0)
    , idxTokenListEnd(0)
    , error(false)
  {}
};

class Parser
{
  public:
    Parser(Lexer& lexer);
    Parser(const Parser& other) = delete;
    Parser(Parser&& other) = delete;
    Parser& operator=(const Parser& other) = delete;
    Parser& operator=(Parser&& other) = delete;
    ~Parser() = default;

    void Parse();
    void PrintDetailedAST() const;
    Node& GetAst();
    bool GetError() const;
    std::string IdentifierToString(Identifier identifier) const;

  private:
    // helper functions
    bool IsEof() const;
    bool IsBlockDirective() const;
    bool IsHttp() const;
    bool IsEvents() const;
    bool IsServer() const;
    bool IsLocation() const;
    bool IsLBrace() const;
    bool IsRBrace() const;
    bool IsDirective() const;
    bool IsSemicolon() const;
    bool IsParam() const;
    Identifier TokenKindToIdentifier(TokenKind kind) const;
    void Next();
    std::string BuildErrorMessage(std::string_view errorType, std::string_view expectedMessage = "");
    void Error(std::string_view error_type, std::string_view message, bool skip, Node& node);

    // parsing logic
    Node ParseDirective(Identifier context);
    void ParseLBrace(Node& blockDirective);
    void ParseRBrace(Node& blockDirective);
    void ParseBlock(Node& blockDirective);
    void ParseLocationParam(Node& blockDirective);
    Node ParseBlockDirective(Identifier context);

    // printAST
    std::string MakeSpaces(std::size_t level) const;
    void PrintParam(const Node& param, std::size_t level) const;
    void PrintDirective(const Node& directive, std::size_t level) const;
    void PrintBlockDirective(const Node& blockDirective, std::size_t level) const;

    Lexer& lexer_;
    std::size_t currentTokenIdx_;
    bool error_;
    Node ast_;
    static constexpr std::string_view kRed_ = "\033[31m";
    static constexpr std::string_view kReset_ = "\033[0m";
};

#endif
