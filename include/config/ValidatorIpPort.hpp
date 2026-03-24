/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   ValidatorIpPort.cpp                                 :+:    :+:           */
/*                                                      +:+                   */
/*   By: jstuhrin <jstuhrin@student.codam.nl>          +#+                    */
/*                                                    +#+                     */
/*   Created: 2026/03/17 11:49:00 by jstuhrin       #+#    #+#                */
/*   Updated: 2026/03/17 11:49:10 by jstuhrin       ########   odam.nl        */
/*                                                                            */
/* ************************************************************************** */

#include <map>
#include <string>
#include <optional>

#include "Parser.hpp"

#ifndef VALIDATORIPPORT_HPP
#define VALIDATORIPPORT_HPP

class ValidatorIpPort
{
  public:
    ValidatorIpPort();
    ValidatorIpPort(const ValidatorIpPort& other) = delete;
    ValidatorIpPort& operator=(const ValidatorIpPort& other) = delete;
    ValidatorIpPort(ValidatorIpPort&& other) = delete;
    ValidatorIpPort&& operator=(ValidatorIpPort&& other) = delete;
    ~ValidatorIpPort() = default;

    std::optional<std::string> GetNormalizedIpv6(const std::string& ipv6) const;
    bool ValidateIpv4(const std::string& ipv4, std::string& errorMessage);
    bool ValidateIpv6(const std::string& ipv6, std::string* errorMessage = nullptr, std::string* normalizedIpv6 = nullptr);
    bool ValidatePortNum(const std::string& portNum, std::string& errorMessage);
    bool ExtractIpv4(const std::string& lexeme, std::string& ipv4);
    bool ExtractIpv6(const std::string& lexeme, std::string& ipv6, std::string& errorMessage);
    bool ExtractPortNum(const std::string& lexeme, std::string& portNum, std::string& errorMessage);

  private:
    bool ValidateQuartets(const std::array<std::string, 8>& quartets, std::string* errorMessage);
    bool IsZero(const std::string& quartet);
    bool LongestConsecutiveZeros(const std::array<std::string, 8>& quartets, std::size_t& lenZeros, std::size_t& startZeros);
    void NormalizeQuartet(std::string& quartet);
    const std::string NormalizeIpv6(std::array<std::string, 8>& quartets);
    bool ValidateDoubleColon(const std::string& ipv6, std::string* errorMessage);
    bool ExpandIpv6Left(const std::string& ipv6, std::array<std::string, 8>& quartets,
                        std::size_t& numQuartets, bool hasZeroCompression, std::string* errorMessage);
    bool ExpandIpv6Right(const std::string& ipv6, std::array<std::string, 8>& quartets,
                         std::size_t& numQuartets, bool hasZeroCompression, std::string* errorMessage);
    bool ExpandIpv6(const std::string& ipv6, std::array<std::string, 8>& quartets, std::string* errorMessage);
    std::map<const std::string, const std::string> normalizedIpv6Adresses_;

    static constexpr std::string_view kRed_ = "\033[31m";
    static constexpr std::string_view kReset_ = "\033[0m";
};

#endif