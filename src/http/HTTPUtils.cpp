/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPUtils.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/09 13:31:19 by bewong        #+#    #+#                 */
/*   Updated: 2026/01/15 16:20:07 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "http/HTTPUtils.hpp"

#include <sys/stat.h>

#include <cctype>
#include <ctime>
#include <string_view>

#include "http/HTTPUtils.hpp"
#include "string.hpp"

namespace HTTP::wire
{
  //  A-Z, a-z, 0-9 and - _ . ~ do not need encoding
  // space, ?, &, =, %, /, # must be encoded
  // space -> %20, ? -> %3F, & -> %26, = -> %3D
  // eg input "hello world?test=1&value=2" -> output "hello%20world%3Ftest%3D1%26value%3D2"
  std::string URLEncode(std::string_view str)  // didnt use
  {
    std::string result;
    result.reserve(str.length() * 3);

    for (unsigned char c : str)
    {
      if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
        result.push_back(static_cast<char>(c));
      else
        String::AppendHex(result, c);
    }
    return result;
  }

  std::string URLDecode(std::string_view str)
  {
    std::string result;
    result.reserve(str.size());

    for (std::size_t i = 0; i < str.size(); ++i)
    {
      if (str[i] == '%' && i + 2 < str.size())
      {
        unsigned int byte{0};
        const char* start = str.data() + i + 1;

        if (String::ConvertToNumber(std::string_view(start, 2), reinterpret_cast<std::size_t&>(byte), 16) &&
            byte <= 0xFF)
        {
          result.push_back(static_cast<char>(byte));
          i += 2;
        }
      }
      if (str[i] == '+')
        result.push_back(' ');
      else
        result.push_back(str[i]);
    }
    return result;
  }
}  // namespace HTTP::wire
