/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   Validator.hpp                                       :+:    :+:           */
/*                                                      +:+                   */
/*   By: jstuhrin <jstuhrin@student.codam.nl>          +#+                    */
/*                                                    +#+                     */
/*   Created: 2025/12/17 15:27:41 by jstuhrin       #+#    #+#                */
/*   Updated: 2025/12/17 15:27:43 by jstuhrin       ########   odam.nl        */
/*                                                                            */
/* ************************************************************************** */


#ifndef VALIDATOR_HPP
#define VALIDATOR_HPP

#include "Lexer.hpp"
#include "Parser.hpp"
#include <vector>
#include <array>
#include <set>
#include <map>

class Validator
{
public:
  Validator(Lexer& lexer, Parser& parser);
  Validator(const Validator& other) = delete;
  Validator(Validator&& other) = delete;
  Validator& operator=(const Validator& other) = delete;
  Validator& operator=(Validator&& other) = delete;
  ~Validator() = default;

  void ValidateAst();
  bool GetError() const;

private:
  // helper functions
  bool IsParamsEmpty(Node& dir, std::string_view message);
  void ValidatePath(Node& node);
  bool IsValidURL(Node& param);
  std::string BuildErrorMessage(std::string_view errorType, std::string_view message, Node& node, bool displayNextToken);
  void Error(std::string_view error_type, std::string_view message, Node& node, bool display_next_token);

  // core validation logic
  void ValidateNestedBlocks(Node& block);
  void ValidateNestedDirs(Node& block);
  void ValidateContextBlocks(Node& block);
  void ValidateContextDirs(Node& block);
  void ValidateNumDirs(Node& block);
  void ValidateNumBlocks(Node& block);
  void ValidateBlock(Node& block);

  // validation individual directives
  void ValidateIpv4(Node& param, const std::string& ipv4);
  bool ValidateDoubleColon(Node& param, const std::string& ipv6);
  bool ExpandIpv6Left(Node& param, const std::string& ipv6, std::array<std::string, 8>&  quartets, std::size_t& numQuartets);
  bool ExpandIpv6Right(Node& param, const std::string& ipv6, std::array<std::string, 8>& quartets, std::size_t& numQuartets);
  bool ExpandIpv6(Node& param, const std::string& ipv6, std::array<std::string, 8>& quartets);
  void ValidateQuartets(Node& param, std::array<std::string, 8>& quartets);
  void ValidateIpv6(Node& param, const std::string& ipv6);
  void ValidatePortNum(Node& param, const std::string& portNum);
  bool ExtractIpv4(Node& param, std::string& ipv4);
  bool ExtractIpv6(Node& param, std::string& ipv6);
  bool ExtractPortNum(Node& param, std::string& portNum);
  void ValidateListen(Node& dir);
  void ValidateName(Node& param);
  void ValidateServer_name(Node& dir);
  void ValidateRoot(Node& dir);
  void ValidateIndex(Node& dir);
  void ValidateAlias(Node& dir);
  bool ValidateUnitPrefix(Node& param, std::string& unitPrefix);
  bool ValidateNumPart(Node& param, const std::string& numPart, std::size_t& number);
  void ValidateNumTimesUnitPrefix(Node& param, const std::size_t& number, const std::string& unitPrefix);
  void ValidateSize(Node& param);
  void ValidateClient_max_body_size(Node& dir);
  void ValidateClient_body_temp_path(Node& dir);
  void ValidateError_page(Node& dir);
  bool IsValidReturnCode(Node& param);
  void ValidateReturn(Node& dir);
  bool IsAllowedMethod(std::string_view lexeme);
  bool IsDuplicate(std::string_view lexeme, std::array<std::size_t, 3>& counts);
  void ValidateAllowed_methods(Node& dir);
  void ValidateOnOffParam(Node& dir);
  void ValidateAutoindex(Node& dir);
  void ValidateCgi(Node& dir);

  bool error_ = false;
  Lexer& lexer_;
  Parser& parser_;
  static constexpr std::string_view kRed_ = "\033[31m";
  static constexpr std::string_view kReset_ = "\033[0m";

  const std::array<std::string_view, 3> allowedMethods_{
    "GET",
    "POST",
    "DELETE"
  };

  const std::set<std::string_view> supportedErrorPageCodesSet_{
    "301",
    "400",
    "401",
    "402",
    "404",
    "500",
    "502",
    "503",
    "504"
  };

  std::string supportedErrorPageCodesStr_;
  
  const std::set<std::string_view> supportedReturnCodesSet_{
    "100",
    "301"
  };

  std::string supportedReturnCodesStr_;

  const std::map<Identifier, std::set<Identifier>> valid_dirs_{
    {
      Identifier::Main, {}},
    {
      Identifier::Events, {}},
    {
      Identifier::Http,
      {
        Identifier::Index,
        Identifier::Client_max_body_size,
        Identifier::Client_body_temp_path,
        Identifier::Error_page,
        Identifier::Autoindex,
        Identifier::Cgi
      }
    },
    {
      Identifier::Server,
      {
        Identifier::Listen,
        Identifier::Server_name,
        Identifier::Root,
        Identifier::Index,
        Identifier::Client_max_body_size,
        Identifier::Client_body_temp_path,
        Identifier::Error_page,
        Identifier::Return,
        Identifier::Allowed_methods,
        Identifier::Autoindex,
        Identifier::Cgi
      }
    },
    {
      Identifier::Location,
      {
        Identifier::Root,
        Identifier::Index,
        Identifier::Alias,
        Identifier::Client_max_body_size,
        Identifier::Client_body_temp_path,
        Identifier::Error_page,
        Identifier::Return,
        Identifier::Allowed_methods,
        Identifier::Autoindex
      }
    }
  };
  
  const std::map<Identifier, std::set<Identifier>> valid_blocks_{
    {
      Identifier::Main,
      {
        Identifier::Events,
        Identifier::Http
      }
    },
    {
      Identifier::Events, {}},
    {
      Identifier::Http,
      {
        Identifier::Server
      }
    },
    {
      Identifier::Server,
      {
        Identifier::Location
      }
    },
    {
      Identifier::Location, {}}
  };
};

#endif
