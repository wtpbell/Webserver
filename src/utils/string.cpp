/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   string.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/22 16:22:46 by jboon         #+#    #+#                 */
/*   Updated: 2026/04/24 15:58:58 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <algorithm>
#include <cctype>
#include <charconv>
#include <chrono>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

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

  bool IsDigitOnly(std::string_view sv)
  {
    return std::all_of(sv.begin(), sv.end(),
                       [](unsigned char c)
                       {
                         return std::isdigit(c);
                       });
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

  std::string_view RightTrim(std::string_view sv, std::size_t last, std::size_t to = 0, std::string_view space = " \t")
  {
    if (last > sv.length())
      last = sv.length();
    if (to > last)
      to = last;
    while (last > to && space.find(sv[last - 1]) != sv.npos)
      --last;
    return sv.substr(to, last - to);
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

  std::string& ToUpper(std::string& str)
  {
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c)
                   {
                     return std::toupper(c);
                   });
    return str;
  }

  void AppendHex(std::string& result, unsigned char c)
  {
    static const char hex[] = "0123456789ABCDEF";

    result += '%';
    result += hex[c >> 4];
    result += hex[c & 0x0F];
  }

  bool StartsWith(std::string_view s, std::string_view prefix)
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

  std::string& ReplaceOccurrence(std::string& str, char occurrence, char replacement)
  {
    std::transform(str.begin(), str.end(), str.begin(),
                   [occurrence, replacement](char c)
                   {
                     return c == occurrence ? replacement : c;
                   });
    return str;
  }

  const char* GMTCstring(char* stime, std::size_t n)
  {
    using sclock = std::chrono::system_clock;

    auto now = sclock::now();
    const std::time_t time = sclock::to_time_t(now);
    std::tm tm{};

    gmtime_r(&time, &tm);
    if (std::strftime(stime, n, "%a, %d %b %Y %H:%M:%S GMT", &tm) > 0)
      return stime;
    return std::strncpy(stime, "Fri, 13 Dec 1901 20:45:52 GMT", n);
  }

  const char* GMTCstringFromTime(std::time_t time, char* stime, std::size_t n)
  {
    std::tm tm{};
    gmtime_r(&time, &tm);

    if (std::strftime(stime, n, "%a, %d %b %Y %H:%M:%S GMT", &tm) > 0)
      return stime;

    if (n > 0)
      stime[0] = '\0';
    return stime;
  }

  std::time_t FileTimeToTimeT(const std::filesystem::file_time_type& ft)
  {
    using namespace std::chrono;

    const auto sctp = time_point_cast<system_clock::duration>(ft - std::filesystem::file_time_type::clock::now() +
                                                              system_clock::now());

    return system_clock::to_time_t(sctp);
  }

  ///@note: https://stackoverflow.com/a/26032303
  std::vector<char*> ConvertToCstrVector(const std::vector<std::string>& v)
  {
    std::vector<char*> c_vec;
    c_vec.reserve(v.size());
    for (const std::string& s : v)
      c_vec.emplace_back(const_cast<char*>(s.c_str()));
    c_vec.emplace_back(nullptr);
    return c_vec;
  }

}  // namespace String
