/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPUtils.hpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/02 15:29:39 by bewong        #+#    #+#                 */
/*   Updated: 2026/05/04 14:47:58 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPUTILS_HPP
#define HTTPUTILS_HPP

#include <string>
#include <string_view>

#include "http/HTTPResponse.hpp"

namespace HTTP
{
  namespace wire
  {
    std::string URLEncode(std::string_view str);
    std::string URLDecode(std::string_view str);
    std::string SerializeResponse(const HTTPResponse& response);
    std::string_view GetMimeType(const std::string_view path);
    std::string GetLastModifiedHttpDate(const std::string& filepath);
    bool NormalizePath(std::string_view in, std::string& out);
  }  // namespace wire

}  // namespace HTTP

#endif  // HTTPUTILS_HPP
