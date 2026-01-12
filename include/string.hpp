/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   string.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/22 16:23:40 by jboon         #+#    #+#                 */
/*   Updated: 2026/01/08 19:57:31 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <string_view>
#include <string>

namespace String
{
  bool IsDigitOnly(const char* s);
  bool IsEmptyOrNull(const char* s);
  std::string_view Trim(std::string_view str);
  std::string ToLower(std::string_view str);
  void ToLowerInPlace(std::string& s);
  void AppendHex(std::string& result, unsigned char c);
  bool starts_with(std::string_view s, std::string_view prefix);
}  // namespace String
