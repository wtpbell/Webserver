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

#include <optional>
#include <charconv>
#include <array>
#include <sstream>
#include <limits>
#include <iostream>

#include "config/Lexer.hpp"
#include "config/Parser.hpp"
#include "config/ValidatorIpPort.hpp"

//////////////////// PUBLIC ////////////////////

std::optional<std::string> ValidatorIpPort::GetNormalizedIpv6(const std::string& ipv6) const
{
  if (normalizedIpv6Adresses_.count(ipv6) != 0)
  {
    return normalizedIpv6Adresses_.at(ipv6);
  }
  return {};
}

bool ValidatorIpPort::ValidateIpv4(const std::string& ipv4, std::string& errorMessage) const
{
  std::vector<std::string> octetts;
  std::string::size_type start = 0;
  std::string::size_type end = ipv4.find('.');
  while (end != std::string::npos)
  {
    octetts.emplace_back(ipv4.substr(start, end - start));
    start = end + 1;
    end = ipv4.find('.', start);
  }
  octetts.emplace_back(ipv4.substr(start));
  if (octetts.size() != 4)
  {
    errorMessage = "ipv4 address must have four octetts";
    return false;
  }
  for (const std::string& octett : octetts)
  {
    if (octett.empty())
    {
      errorMessage = "empty octett in ipv4 address";
      return false;
    }
    if (octett[0] == '0' && std::isdigit(static_cast<unsigned char>(octett[1])))
    {
      errorMessage = "leading zero";
      return false;
    }
    std::size_t octettNum{};
    auto [ptr, ec] = std::from_chars(octett.data(), octett.data() + octett.size(), octettNum);
    if (ptr != octett.data() + octett.size())
    {
      errorMessage = "illegal character";
      return false;
    }
    if (ec == std::errc::result_out_of_range)
    {
      errorMessage = "overflow";
      return false;
    }
    if (octettNum > 255)
    {
      errorMessage = "number too large";
      return false;
    }
  }
  return true;
}

bool ValidatorIpPort::ValidateDoubleColon(const std::string& ipv6, std::string& errorMessage) const
{
  std::string::size_type posDoubleColon = ipv6.find("::");
  if (posDoubleColon == std::string::npos)
  {
    return true;
  }
  posDoubleColon = ipv6.find("::", posDoubleColon + 1);
  if (posDoubleColon == std::string::npos)
  {
    return true;
  }
  errorMessage = "unexpected `::`";
  return false;
}

bool ValidatorIpPort::ExpandIpv6Left(const std::string& ipv6, std::array<std::string, 8>& quartets,
                               std::size_t& numQuartets, bool hasZeroCompression, std::string& errorMessage) const
{
  std::string::size_type start = 0;
  std::string::size_type end = ipv6.find("::");
  if (end == std::string::npos)
  {
    end = ipv6.size();
  }
  std::string ipv6Left = ipv6.substr(start, end - start);
  if (ipv6Left.empty())
  {
    return true;
  }
  if (ipv6Left.front() == ':' || ipv6Left.back() == ':')
  {
    errorMessage = "empty quartet in ipv6 address";
    return false;
  }
  end = ipv6Left.find(':');
  std::size_t i = 0;
  while (true)
  {
    if (end == std::string::npos)
    {
      end = ipv6Left.size();
    }
    ++numQuartets;
    if (numQuartets > 8 || (hasZeroCompression && numQuartets > 7))
    {
      errorMessage = "too many quartets in ipv6 address";
      return false;
    }
    quartets[i] = ipv6Left.substr(start, end - start);
    ++i;
    if (end == ipv6Left.size())
    {
      break;
    }
    start = end + 1;
    end = ipv6Left.find(':', start);
  }
  return true;
}

bool ValidatorIpPort::ExpandIpv6Right(const std::string& ipv6, std::array<std::string, 8>& quartets,
                                std::size_t& numQuartets, bool hasZeroCompression, std::string& errorMessage) const
{
  if (ipv6.find("::") == std::string::npos)
  {
    return true;
  }
  std::string::size_type start = ipv6.find("::") + 2;
  std::string::size_type end = ipv6.size();
  std::string ipv6Right = ipv6.substr(start, end - start);
  if (ipv6Right.empty())
  {
    return true;
  }
  if (ipv6Right.front() == ':' || ipv6Right.back() == ':')
  {
    errorMessage = "empty quartet in ipv6 address";
    return false;
  }
  std::string::size_type colon = ipv6Right.find_last_of(':');
  start = colon + 1;
  end = ipv6Right.size();
  std::size_t i = 7;
  while (true)
  {
    if (colon == std::string::npos)
    {
      start = 0;
    }
    ++numQuartets;
    if (numQuartets > 8 || (hasZeroCompression && numQuartets > 7))
    {
      errorMessage = "too many quartets in ipv6 address";
      return false;
    }
    quartets[i] = ipv6Right.substr(start, end - start);
    --i;
    if (start == 0)
    {
      break;
    }
    end = colon;
    colon = ipv6Right.find_last_of(':', colon - 1);
    start = colon + 1;
  }
  return true;
}

bool ValidatorIpPort::ExpandIpv6(const std::string& ipv6, std::array<std::string, 8>& quartets, std::string& errorMessage) const
{
  std::string::size_type end = ipv6.find(':');
  if (end == std::string::npos)
  {
    errorMessage = "invalid ipv6 address";
    return false;
  }
  std::size_t numQuartets = 0;
  bool hasZeroCompression = ipv6.find("::") == std::string::npos ? false : true;
  if (!ExpandIpv6Left(ipv6, quartets, numQuartets, hasZeroCompression, errorMessage) ||
      !ExpandIpv6Right(ipv6, quartets, numQuartets, hasZeroCompression, errorMessage))
  {
    return false;
  }
  if (!hasZeroCompression && numQuartets < 8)
  {
    errorMessage = "too few quartets in ipv6 address";
    return false;
  }
  return true;
}

//////////////////// PRIVATE ////////////////////
//////////////////// validation ////////////////////

bool ValidatorIpPort::ValidateQuartets(const std::array<std::string, 8>& quartets, std::string& errorMessage) const
{
  for (const std::string& q : quartets)
  {
    if (q.size() > 4)
    {
      errorMessage = "ipv6 quartet too long";
      return false;
    }
    for (const char c : q)
    {
      if (!std::isxdigit(static_cast<unsigned char>(c)))
      {
        errorMessage = "illegal character in ipv6 quartet";
        return false;
      }
    }
  }
  return true;
}

bool ValidatorIpPort::ValidateIpv6(const std::string& ipv6, std::string& errorMessage)
{
  if (ipv6.empty())
  {
    errorMessage = "empty ipv6 address";
    return false;
  }
  if (!ValidateDoubleColon(ipv6, errorMessage))
  {
    return false;
  }
  std::array<std::string, 8> quartets;
  for (std::string& q : quartets)
  {
    q = "0";
  }
  if (!ExpandIpv6(ipv6, quartets, errorMessage))
  {
    return false;
  }
  if (!ValidateQuartets(quartets, errorMessage))
  {
    return false;
  }
  normalizedIpv6Adresses_.emplace(ipv6, NormalizeIpv6(quartets));
  return true;
}

bool ValidatorIpPort::ValidatePortNum(const std::string& portNum, std::string& errorMessage) const
{
  if (portNum.size() > 1 && portNum[0] == '0')
  {
    errorMessage = "leading zero";
    return false;
  }
  std::size_t num{};
  auto [ptr, ec] = std::from_chars(portNum.data(), portNum.data() + portNum.size(), num);
  if (ptr != portNum.data() + portNum.size())
  {
    errorMessage = "illegal character";
    return false;
  }
  if (ec == std::errc::result_out_of_range)
  {
    errorMessage = "overflow";
    return false;
  }
  if (num > 65535)
  {
    errorMessage = "port number out of range";
    return false;
  }
  return true;
}

bool ValidatorIpPort::ExtractIpv4(const std::string& lexeme, std::string& ipv4) const
{
  if (lexeme.front() == '[' || lexeme.find('.') == std::string::npos)
  {
    return false;
  }
  std::string::size_type end = lexeme.find(':');
  if (end == std::string::npos)
  {
    ipv4 = lexeme;
    return true;
  }
  ipv4 = lexeme.substr(0, end);
  return true;
}

bool ValidatorIpPort::ExtractIpv6(const std::string& lexeme, std::string& ipv6, std::string& errorMessage) const
{
  if (lexeme.front() != '[')
  {
    return false;
  }
  std::string::size_type end = lexeme.find(']');
  if (end == std::string::npos)
  {
    errorMessage = "missing `]`";
    return false;
  }
  ipv6 = lexeme.substr(1, end - 1);
  return true;
}

bool ValidatorIpPort::ExtractPortNum(const std::string& lexeme, std::string& portNum, std::string& errorMessage) const
{
  if (lexeme.front() == '[')
  {
    std::string::size_type posRBrace = lexeme.find(']');
    if (posRBrace == std::string::npos)
    {
      return false;
    }
    if (posRBrace + 1 != lexeme.size() && lexeme[posRBrace + 1] != ':')
    {
      errorMessage = "illegal character";
      return false;
    }
    std::string::size_type start = lexeme.find(':', posRBrace);
    if (start == std::string::npos)
    {
      return false;
    }
    portNum = lexeme.substr(start + 1);
    return true;
  }
  if (lexeme.find('.') != std::string::npos)
  {
    std::string::size_type start = lexeme.find(':');
    if (start == std::string::npos)
    {
      return false;
    }
    portNum = lexeme.substr(start + 1);
    return true;
  }
  portNum = lexeme;
  return true;
}

//////////////////// ipv6 normalization ////////////////////

bool ValidatorIpPort::IsZero(const std::string& quartet) const
{
  for (const char c : quartet)
  {
    if (c != '0')
    {
      return false;
    }
  }
  return true;
}

bool ValidatorIpPort::LongestConsecutiveZeros(const std::array<std::string, 8>& quartets, std::size_t& lenZeros, std::size_t& startZeros) const
{
  std::size_t currentLen = 0;
  std::size_t startCurrent = 0;
  std::size_t longestLen = 0;
  std::size_t longestStart = 0;
  bool inZeroSequence = false;
  for (std::size_t i = 0; i < quartets.size(); ++i)
  {
    if (IsZero(quartets[i]))
    {
      if (inZeroSequence == true)
      {
        ++currentLen;
      }
      else
      {
        inZeroSequence = true;
        startCurrent = i;
        currentLen = 1;
      }
    }
    else
    {
      if (inZeroSequence == true)
      {
        if (currentLen > 1 && currentLen > longestLen)
        {
          longestStart = startCurrent;
          longestLen = currentLen;
        }
        inZeroSequence = false;
      }
    }
  }
  if (currentLen > 1 && currentLen > longestLen)
  {
    longestStart = startCurrent;
    longestLen = currentLen;
  }
  if (longestLen > 1)
  {
    lenZeros = longestLen;
    startZeros = longestStart;
    return true;
  } 
  return false;
}

void ValidatorIpPort::NormalizeQuartet(std::string& quartet) const
{
  std::string normalizedQuartet;
  std::size_t offset = 0;
  while (quartet[offset] == '0' && offset < quartet.size() - 1)
  {
    ++offset;
  }
  while (offset < quartet.size())
  {
    normalizedQuartet.push_back(std::tolower(static_cast<unsigned char>(quartet[offset++])));
  }
  quartet = normalizedQuartet;
}

const std::string ValidatorIpPort::NormalizeIpv6(std::array<std::string, 8>& quartets) const
{
  std::string normalizedIpv6;
  std::size_t lenZeros = 0;
  std::size_t startZeros = std::numeric_limits<size_t>::max();
  bool hasConsecutiveZeros = LongestConsecutiveZeros(quartets, lenZeros, startZeros);
  std::size_t i = 0;
  while (i < quartets.size() && i < startZeros)
  {
    if (i > 0)
    {
      normalizedIpv6.append(":");
    }
    NormalizeQuartet(quartets[i]);
    normalizedIpv6.append(quartets[i]);
    ++i;
  }
  if (hasConsecutiveZeros)
  {
    normalizedIpv6.append("::");
    i += lenZeros;
    while (i < quartets.size())
    {
      NormalizeQuartet(quartets[i]);
      normalizedIpv6.append(quartets[i]);
      if (i < quartets.size() - 1)
      {
        normalizedIpv6.append(":");
      }
      ++i;
    }
  }
  return normalizedIpv6;
}
