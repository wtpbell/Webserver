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

#include <array>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <optional>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "ValidatorIpPort.hpp"

class Validator
{
  public:
    Validator(Lexer& lexer, Parser& parser, ValidatorIpPort& validatorIpPort);
    Validator(const Validator& other) = delete;
    Validator(Validator&& other) = delete;
    Validator& operator=(const Validator& other) = delete;
    Validator& operator=(Validator&& other) = delete;
    ~Validator() = default;

    bool GetError() const;

  private:
    // helper functions
    bool IsParamsEmpty(Node& dir, std::string_view message);
    bool IsValidURL(Node& param);
    std::string BuildErrorMessage(std::string_view errorType, std::string_view message, std::size_t idx);
    void Error(std::string_view error_type, std::string_view message, Node& node, std::size_t idx);

    // core validation logic
    void ValidateNestedBlocks(Node& block);
    void ValidateNestedDirs(Node& block);
    void ValidateContextBlocks(Node& block);
    void ValidateContextDirs(Node& block);
    void ValidateNumDirs(Node& block);
    void ValidateNumBlocks(Node& block);
    void ValidateBlock(Node& block);
    bool HandleIpv4(Node& dir);
    void HandleIpv6(Node& dir);
    void HandlePortNum(Node& dir);
    void ValidateListen(Node& dir);
    void ValidateName(Node& param);
    void ValidateServerName(Node& dir);
    void ValidateRoot(Node& dir);
    void ValidateIndex(Node& dir);
    void ValidateAlias(Node& dir);
    bool ValidateUnitPrefix(Node& param, std::string& unitPrefix);
    bool ValidateNumPart(Node& param, const std::string& numPart, std::size_t& number);
    void ValidateNumTimesUnitPrefix(Node& param, const std::size_t& number, const std::string& unitPrefix);
    void ValidateSize(Node& param);
    void ValidateClientMaxBodySize(Node& dir);
    void ValidateErrorPage(Node& dir);
    bool IsValidReturnCode(Node& param);
    void ValidateReturn(Node& dir);
    bool IsAllowedMethod(std::string_view lexeme);
    bool IsDuplicate(std::string_view lexeme, std::array<std::size_t, 3>& counts);
    void ValidateAllowedMethods(Node& dir);
    void ValidateOnOffParam(Node& dir);
    void ValidateAutoindex(Node& dir);
    void ValidateCgi(Node& dir);
    void ValidateCgiExtension(Node& dir);

    Lexer& lexer_;
    Parser& parser_;
    ValidatorIpPort& validatorIpPort_;
    bool error_;
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
      "403",
      "404",
      "405",
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

    const std::set<Identifier> duplicatesIllegal_{
      Identifier::kCgi,
      Identifier::kIndex,
      Identifier::kAutoindex,
      Identifier::kClientMaxBodySize,
      Identifier::kAllowedMethods,
      Identifier::kReturn,
      Identifier::kRoot,
      Identifier::kAlias,
    };

    const std::map<Identifier, std::set<Identifier>> validDirectives_{
      {
        Identifier::kMain, {}
      },
      {
        Identifier::kHttp,
        {
          Identifier::kIndex,
          Identifier::kClientMaxBodySize,
          Identifier::kErrorPage,
          Identifier::kAutoindex
        }
      },
      {
        Identifier::kServer,
        {
          Identifier::kListen,
          Identifier::kServerName,
          Identifier::kRoot,
          Identifier::kIndex,
          Identifier::kClientMaxBodySize,
          Identifier::kErrorPage,
          Identifier::kReturn,
          Identifier::kAllowedMethods,
          Identifier::kAutoindex,
          Identifier::kCgiExtension
        }
      },
      {
        Identifier::kLocation,
        {
          Identifier::kRoot,
          Identifier::kIndex,
          Identifier::kAlias,
          Identifier::kClientMaxBodySize,
          Identifier::kErrorPage,
          Identifier::kReturn,
          Identifier::kAllowedMethods,
          Identifier::kAutoindex,
          Identifier::kCgi,
          Identifier::kCgiExtension
        }
      }
    };
  
    const std::map<Identifier, std::set<Identifier>> validBlocks_{
      {
        Identifier::kMain,
        {
          Identifier::kHttp
        }
      },
      {
        Identifier::kHttp,
        {
          Identifier::kServer
        }
      },
      {
        Identifier::kServer,
        {
          Identifier::kLocation
        }
      },
      {
        Identifier::kLocation, {}
      }
    };
};

#endif
