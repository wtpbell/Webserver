/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   string.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/22 16:23:40 by jboon         #+#    #+#                 */
/*   Updated: 2026/02/10 17:46:41 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <string_view>

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
  bool starts_with(std::string_view s, std::string_view prefix);
  bool ConvertToNumber(std::string_view sv, std::size_t& result, int base = 10);
  bool IsCloseToken(std::string_view v);
  std::string CanonicalizeHeader(std::string_view key);
  std::string& ReplaceOccurrence(std::string& str, char occurrence, char replacement);
  const char* GMTCstring(char* stime, std::size_t n);
}  // namespace String
