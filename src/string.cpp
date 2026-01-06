/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   string.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/22 16:22:46 by jboon         #+#    #+#                 */
/*   Updated: 2025/12/22 16:26:02 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <algorithm>
#include <cctype>
#include <string_view>

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
}  // namespace String
