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

/*HTTPResponse {
    status = 200
    reason = "OK"
    headers = { "content-length": "5" }
    body = "Hello"
  }
-->HTTP/1.1 200 OK\r\n
Content-Length: 5\r\n
\r\n
Hello

*/

#endif  // HTTPUTILS_HPP
