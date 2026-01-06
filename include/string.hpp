/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   string.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/22 16:23:40 by jboon         #+#    #+#                 */
/*   Updated: 2025/12/22 16:26:05 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <string_view>

namespace String
{
  bool IsDigitOnly(const char* s);
  bool IsEmptyOrNull(const char* s);
}  // namespace String
