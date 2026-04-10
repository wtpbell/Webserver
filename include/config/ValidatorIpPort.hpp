/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ValidatorIpPort.hpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: jstuhrin <jstuhrin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/03/17 11:49:00 by jstuhrin      #+#    #+#                 */
/*   Updated: 2026/04/02 11:16:43 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef VALIDATORIPPORT_HPP
#define VALIDATORIPPORT_HPP

#include <array>
#include <map>
#include <optional>
#include <string>
#include <string_view>

class ValidatorIpPort
{
  public:
    ValidatorIpPort() = default;
    ValidatorIpPort(const ValidatorIpPort& other) = delete;
    ValidatorIpPort& operator=(const ValidatorIpPort& other) = delete;
    ValidatorIpPort(ValidatorIpPort&& other) = delete;
    ValidatorIpPort&& operator=(ValidatorIpPort&& other) = delete;
    ~ValidatorIpPort() = default;

    std::optional<std::string> GetNormalizedIpv6(const std::string& ipv6) const;
    bool ValidateIpv4(const std::string& ipv4, std::string& errorMessage) const;
    bool ValidateIpv6(const std::string& ipv6, std::string& errorMessage);
    bool ValidatePortNum(const std::string& portNum, std::string& errorMessage) const;
    bool ExtractIpv4(const std::string& lexeme, std::string& ipv4) const;
    bool ExtractIpv6(const std::string& lexeme, std::string& ipv6, std::string& errorMessage) const;
    bool ExtractPortNum(const std::string& lexeme, std::string& portNum, std::string& errorMessage) const;

  private:
    bool ValidateQuartets(const std::array<std::string, 8>& quartets, std::string& errorMessage) const;
    bool ValidateDoubleColon(const std::string& ipv6, std::string& errorMessage) const;
    bool ExpandIpv6Left(const std::string& ipv6, std::array<std::string, 8>& quartets, std::size_t& numQuartets,
                        bool hasZeroCompression, std::string& errorMessage) const;
    bool ExpandIpv6Right(const std::string& ipv6, std::array<std::string, 8>& quartets, std::size_t& numQuartets,
                         bool hasZeroCompression, std::string& errorMessage) const;
    bool ExpandIpv6(const std::string& ipv6, std::array<std::string, 8>& quartets, std::string& errorMessage) const;
    bool IsZero(const std::string& quartet) const;
    bool LongestConsecutiveZeros(const std::array<std::string, 8>& quartets, std::size_t& lenZeros,
                                 std::size_t& startZeros) const;
    void NormalizeQuartet(std::string& quartet) const;
    const std::string NormalizeIpv6(std::array<std::string, 8>& quartets) const;

    std::map<const std::string, const std::string> normalizedIpv6Adresses_;

    static constexpr std::string_view kRed_ = "\033[31m";
    static constexpr std::string_view kReset_ = "\033[0m";
};

#endif
