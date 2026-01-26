/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPUtils.hpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/02 15:29:39 by bewong        #+#    #+#                 */
/*   Updated: 2025/12/02 15:29:39 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPUTILS_HPP
#define HTTPUTILS_HPP

#include <string>
#include <string_view>

#include "HTTPTypes.hpp"

class HTTPResponse;

namespace HTTP
{
  namespace wire
  {
    // these are also “wire-format helpers”
    std::string URLEncode(std::string_view str);
    std::string URLDecode(std::string_view str);

  }  // namespace wire
}  // namespace HTTP

#endif  // HTTPUTILS_HPP
