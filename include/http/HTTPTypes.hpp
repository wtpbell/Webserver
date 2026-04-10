/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPTypes.hpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/11 14:54:12 by bewong        #+#    #+#                 */
/*   Updated: 2026/04/02 09:56:57 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPTYPES_HPP
#define HTTPTYPES_HPP

#include <cstddef>
#include <cstdint>
#include <ctime>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace HTTP
{

  using Headers = std::unordered_map<std::string, std::vector<std::string>>;

  inline constexpr std::string_view kCRLF = "\r\n";
  inline constexpr std::string_view kWHITESPACE = " \t\v\r\n";
  inline constexpr std::string_view kVERSION = "HTTP/1.1";
  inline constexpr std::string_view kSERVER_NAME = "Webserv/1.0";
  inline constexpr std::size_t kMaxRequestLine = 8000;
  inline constexpr std::size_t kMaxHeaderSize = 8192;
  inline constexpr std::size_t kMaxBodySize = 1024 * 1024;    // 1 MB (example)
  inline constexpr std::size_t kMemoryThreshold = 64 * 1024;  // 64KB
  inline constexpr std::time_t kTTL = 1800;
  inline constexpr std::string_view HTTP_TCHAR =
      "!#$%&'*+-.^_`|~"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz"
      "0123456789";

  enum class Method : std::uint8_t
  {
    kGet,
    kPost,
    kDelete,
    kUnsupported
  };

  inline constexpr struct MethodEntry
  {
      HTTP::Method method;
      std::string_view name;
  } kMethodMap[] = {
      {HTTP::Method::kGet, "GET"},
      {HTTP::Method::kPost, "POST"},
      {HTTP::Method::kDelete, "DELETE"},
  };

  constexpr Method StringToMethod(std::string_view s)
  {
    for (const MethodEntry& p : kMethodMap)
      if (p.name == s)
        return p.method;
    return Method::kUnsupported;
  }

  constexpr std::string_view MethodToString(Method m)
  {
    for (const auto& e : kMethodMap)
      if (e.method == m)
        return e.name;
    return "UNSUPPORTED";
  }
}  // namespace HTTP
#endif  // HTTPTYPES_HPP
