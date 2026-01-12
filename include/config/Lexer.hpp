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
  Events,
  Http,
  Location,
  Server,
  String,
  Number,
  Eof
};

struct Token
{
    TokenKind kind;
    std::string lexeme;
    std::string leadingTrivia;
    size_t line;
    size_t col;
    bool error;
};

class Lexer
{
  public:
    Lexer(const std::string& buffer);
    Lexer(const Lexer& other) = delete;
    Lexer(Lexer&& other) noexcept = delete;
    Lexer& operator=(const Lexer& other) = delete;
    Lexer& operator=(Lexer&& other) noexcept = delete;
    ~Lexer() = default;

    void Lex(void);
    void PrintTokenList(void);
    void PrintConfigsDebug(void);
    Token& Next(void);
    Token& Current(void);
    void SetTokenErrorTrue(void);

  private:
    bool Is(char c);
    bool Is(std::string_view);
    bool IsSeparator(char c);
    TokenKind DetermineKind(void);
    std::string ExtractLexeme(void);
    std::string DetermineLexeme(TokenKind kind);
    bool IsWhiteSpace(size_t pos);
    bool IsHash(size_t pos);
    bool IsEndComment(size_t pos);
    std::string ExtractTrivia(void);
    std::string TokenKindToString(TokenKind kind);
    void MakeToken(void);
    bool IsLineBreak(void);
    void Advance(size_t steps = 1);
    void Traverse(const std::string& lexeme);
    void PrintToken(const Token& token);
    void PrintTriviaAndLexeme(const Token& token);

    std::string buffer_;
    std::vector<Token> tokenList_;
    size_t line_;
    size_t col_;
    size_t posBuffer_;
    size_t idxCurrentToken_;
    bool madeEofToken_;
    static constexpr std::string_view kRed_ = "\033[31m";
    static constexpr std::string_view kReset_ = "\033[0m";
};

#endif
