/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   Lexer.hpp                                           :+:    :+:           */
/*                                                      +:+                   */
/*   By: jstuhrin <jstuhrin@student.codam.nl>          +#+                    */
/*                                                    +#+                     */
/*   Created: 2025/12/02 10:35:18 by jstuhrin       #+#    #+#                */
/*   Updated: 2025/12/02 10:35:23 by jstuhrin       ########   codam.nl       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <string_view>
#include <vector>

enum class TokenKind
{
  kLBrace,
  kRBrace,
  kSemicolon,
  kListen,
  kServerName,
  kRoot,
  kIndex,
  kAlias,
  kClientMaxBodySize,
  kErrorPage,
  kReturn,
  kAllowedMethods,
  kAutoindex,
  kCgi,
  kCgiExtension,
  kHttp,
  kLocation,
  kServer,
  kString,
  kEof
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

  Token(TokenKind kind, std::string lexeme, std::string leadingTrivia, std::size_t line, std::size_t col, std::size_t idxCurrentToken_)
  	: kind(kind)
  	, lexeme(std::move(lexeme))
  	, leadingTrivia(std::move(leadingTrivia))
  	, line(line)
  	, col(col)
  	, idxTokenList(idxCurrentToken_)
  	, error(false)
  {}
};

class Lexer
{
  public:
    Lexer(std::string buffer);
    Lexer(const Lexer& other) = delete;
    Lexer(Lexer&& other) = delete;
    Lexer& operator=(const Lexer& other) = delete;
    Lexer& operator=(Lexer&& other) = delete;
    ~Lexer() = default;

    void PrintTokenList() const;
    void PrintConfigsDebug() const;
    void PrintErrorIdxs() const;
    void PrintErrorMessages() const;
    void SetTokenErrorTrue(std::size_t idxTokenList);
    void SetTokenErrorMessage(std::size_t idx, const std::string& errorMessage);
    TokenKind GetTokenKind(std::size_t idxTokenList) const;
    bool GetTokenError(std::size_t idxTokenList) const;
    std::size_t GetLine(std::size_t idxTokenList) const;
    std::size_t GetCol(std::size_t idxTokenList) const;
    std::string_view GetLexeme(std::size_t idxTokenList) const;
    std::string_view GetTrivia(std::size_t idxTokenList) const;
    std::size_t GetSizeTokenList() const;
    bool GetError() const;
    std::string_view GetTokenErrorMessage(std::size_t idx) const;

  private:
    bool Is(const char c) const;
    bool Is(const std::string& lexeme) const;
    bool IsSeparator(const char c) const;
    std::size_t LexemeLen() const;
    TokenKind DetermineKind() const;
    std::string ExtractLexeme() const;
    bool IsWhiteSpace(const std::size_t pos) const;
    bool IsHash(const std::size_t pos) const;
    bool IsEndComment(const std::size_t pos) const;
    std::string ExtractTrivia() const;
    std::string TokenKindToString(TokenKind kind) const;
    void ValidatePrintable(Token& token);
    void MakeToken();
    bool IsLineBreak() const;
    void Traverse(const std::string& lexeme);
    void PrintToken(const Token& token) const;
    void PrintTriviaAndLexeme(const Token& token) const;

    std::size_t line_;
    std::size_t col_;
    std::size_t posBuffer_;
    std::size_t idxCurrentToken_;
    bool madeEofToken_;
    bool error_;
    std::string buffer_;
    std::vector<Token> tokenList_;
    static constexpr std::string_view kRed_ = "\033[31m";
    static constexpr std::string_view kReset_ = "\033[0m";
};

#endif
