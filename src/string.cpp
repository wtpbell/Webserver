/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   string.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/22 16:22:46 by jboon         #+#    #+#                 */
/*   Updated: 2026/02/06 17:42:48 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <cctype>
#include <charconv>
#include <string>
#include <string_view>
#include <system_error>

#include "http/HTTPTypes.hpp"

namespace String
{
  bool IsDigitOnly(const char* s)
  {
    while (*s != '\0')
    {
      if (!std::isdigit(static_cast<unsigned char>(*s)))
        return (false);
      ++s;
    }
    return (true);
  }

  bool IsEmptyOrNull(const char* s)
  {
    return (s == nullptr || *s == '\0');
  }

  std::string_view Trim(std::string_view str)
  {
    std::size_t start = str.find_first_not_of(HTTP::kWHITESPACE);
    if (start == std::string::npos)
      return {};
    std::size_t end = str.find_last_not_of(HTTP::kWHITESPACE);
    return str.substr(start, end - start + 1);
  }

  std::string ToLower(std::string_view str)
  {
    std::string out;
    out.reserve(str.size());

    for (char c : str)
    {
      unsigned char uc = static_cast<unsigned char>(c);
      out.push_back(static_cast<char>(std::tolower(uc)));
    }
    return out;
  }

  void ToLowerInPlace(std::string& str)
  {
    for (char& c : str)
    {
      unsigned char uc = static_cast<unsigned char>(c);
      c = static_cast<char>(std::tolower(uc));
    }
  }

  void AppendHex(std::string& result, unsigned char c)
  {
    static const char hex[] = "0123456789ABCDEF";

    result += '%';
    result += hex[c >> 4];
    result += hex[c & 0x0F];
  }

  bool starts_with(std::string_view s, std::string_view prefix)
  {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
  }

  bool ConvertToNumber(std::string_view sv, std::size_t& result, int base = 10)
  {
    const char* end = sv.data() + sv.length();
    auto [ptr, ec] = std::from_chars(sv.data(), end, result, base);
    return (ec == std::errc{} && ptr == end);
  }

  bool IsCloseToken(std::string_view v)
  {
    constexpr std::string_view close = "close";

    v = String::Trim(v);
    if (v.size() != close.size())
      return false;

    for (std::size_t i = 0; i < close.size(); ++i)
    {
      const unsigned char c = static_cast<unsigned char>(v[i]);
      if (close[i] != static_cast<char>(std::tolower(c)))
        return false;
    }
    return true;
  }

  std::string CanonicalizeHeader(std::string_view key)
  {
    std::string out;
    out.reserve(key.size());

    bool upper = true;
    for (unsigned char ch : key)
    {
      if (upper && std::isalpha(ch))
        out.push_back(static_cast<char>(std::toupper(ch)));
      else
        out.push_back(static_cast<char>(std::tolower(ch)));

      upper = (ch == '-');
    }
    return out;
  }

}  // namespace String
