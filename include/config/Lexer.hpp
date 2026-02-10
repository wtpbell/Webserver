/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   Lexer.hpp                                           :+:    :+:           */
/*                                                      +:+                   */
/*   By: jstuhrin <jstuhrin@student.ccodam.nl>          +#+                    */
/*                                                    +#+                     */
/*   Created: 2025/12/02 10:35:18 by jstuhrin       #+#    #+#                */
/*   Updated: 2025/12/02 10:35:23 by jstuhrin       ########   codam.nl        */
/*                                                                            */
/* ************************************************************************** */

#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>

enum class TokenKind
{
  LBrace,
  RBrace,
  Semicolon,
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
  Events,
  Http,
  Location,
  Server,
  String,
  Eof
};

struct Token
{
    TokenKind kind;
    std::string lexeme;
    std::string leadingTrivia;
    std::size_t line;
    std::size_t col;
    std::size_t idxTokenList;
    bool error;
    std::string errorMessage;
};

class Lexer
{
  public:
    Lexer();
    Lexer(const Lexer& other) = delete;
    Lexer(Lexer&& other) = delete;
    Lexer& operator=(const Lexer& other) = delete;
    Lexer& operator=(Lexer&& other) = delete;
    ~Lexer() = default;

    void Lex(std::string buffer);
    // print methods
    void PrintTokenList() const;
    void PrintConfigsDebug() const;
    void PrintErrorIdxs() const;
    void PrintErrorMessages() const;
    // consume token list
    Token Next();
    Token Current() const;
    // setters
    void SetTokenErrorTrue(std::size_t idxTokenList);
    void SetTokenErrorMessage(std::size_t idx, const std::string& errorMessage);
    // getters
    bool GetTokenError(std::size_t idxTokenList) const;
    std::size_t GetLine(std::size_t idxTokenList) const;
    std::size_t GetCol(std::size_t idxTokenList) const;
    std::string_view GetLexeme(std::size_t idxTokenList) const;
    std::size_t GetSizeTokenList() const;
    bool GetError() const;
    std::string_view GetTokenErrorMessage(std::size_t idx) const;


  private:
    bool Is(const char c) const;
    bool Is(const std::string& lexeme) const;
    bool IsSeparator(const char c) const;
    TokenKind DetermineKind() const;
    std::string ExtractLexeme();
    std::string DetermineLexeme(TokenKind kind);
    bool IsWhiteSpace(const std::size_t pos) const;
    bool IsHash(const std::size_t pos) const;
    bool IsEndComment(const std::size_t pos) const;
    std::string ExtractTrivia();
    std::string TokenKindToString(TokenKind kind) const;
    void ValidatePrintable(Token& token);
    void MakeToken();
    bool IsLineBreak() const;
    void Advance(const std::size_t steps = 1);
    void Traverse(const std::string& lexeme);
    void PrintToken(const Token& token) const;
    void PrintTriviaAndLexeme(const Token& token) const;

    std::string buffer_;
    std::vector<Token> tokenList_;
    std::size_t line_ = 1;
    std::size_t col_ = 1;
    std::size_t posBuffer_ = 0;
    std::size_t idxCurrentToken_ = 0;
    bool madeEofToken_ = false;
    bool error_ = false;
    static constexpr std::string_view kRed_ = "\033[31m";
    static constexpr std::string_view kReset_ = "\033[0m";
};

#endif
