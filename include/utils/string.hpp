/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   string.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/22 16:23:40 by jboon         #+#    #+#                 */
/*   Updated: 2026/05/10 19:41:20 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <ctime>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace String
{
  bool IsDigitOnly(const char* s);
  bool IsDigitOnly(std::string_view sv);
  bool IsEmptyOrNull(const char* s);
  std::string_view Trim(std::string_view str);
  std::string_view RightTrim(std::string_view sv, std::size_t last, std::size_t to = 0, std::string_view space = " \t");
  std::string ToLower(std::string_view str);
  void ToLowerInPlace(std::string& s);
  std::string& ToUpper(std::string& str);
  void AppendHex(std::string& result, unsigned char c);
  bool StartsWith(std::string_view s, std::string_view prefix);
  bool ConvertToNumber(std::string_view sv, std::size_t& result, int base = 10);
  bool IsCloseToken(std::string_view v);
  std::string CanonicalizeHeader(std::string_view key);
  std::string& ReplaceOccurrence(std::string& str, char occurrence, char replacement);
  const char* GMTCstring(char* stime, std::size_t n);
  const char* GMTCstringFromTime(std::time_t time, char* stime, std::size_t n);
  std::time_t FileTimeToTimeT(const std::filesystem::file_time_type& ft);
  std::vector<char*> ConvertToCstrVector(const std::vector<std::string>& v);
}  // namespace String
