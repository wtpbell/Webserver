/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPUtils.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/09 13:31:19 by bewong        #+#    #+#                 */
/*   Updated: 2026/02/06 17:46:39 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "http/HTTPUtils.hpp"

#include <sys/stat.h>

#include <cctype>
#include <ctime>
#include <string>
#include <string_view>

#include "http/HTTPResponse.hpp"
#include "string.hpp"

namespace HTTP
{
  namespace wire
  {
    //  A-Z, a-z, 0-9 and - _ . ~ do not need encoding
    // space, ?, &, =, %, /, # must be encoded
    // space -> %20, ? -> %3F, & -> %26, = -> %3D
    // eg input "hello world?test=1&value=2" -> output "hello%20world%3Ftest%3D1%26value%3D2"
    std::string URLEncode(std::string_view str)
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

    // if no . in the path, return default MIME type application/octet-stream
    std::string_view GetMimeType(const std::string_view path)
    {
      static const std::unordered_map<std::string, std::string> mimeTypes = {
          {"html", "text/html"},        {"htm", "text/html"},
          {"css", "text/css"},          {"js", "application/javascript"},
          {"json", "application/json"}, {"png", "image/png"},
          {"jpg", "image/jpeg"},        {"jpeg", "image/jpeg"},
          {"gif", "image/gif"},         {"ico", "image/x-icon"},
          {"svg", "image/svg+xml"},     {"pdf", "application/pdf"},
          {"txt", "text/plain"},        {"xml", "application/xml"}};

      constexpr std::string_view DEFAULT_MIME = "application/octet-stream";
      std::size_t dot = path.find_last_of('.');
      if (dot == std::string::npos)
        return DEFAULT_MIME;
      std::string ext(path.substr(dot + 1));
      String::ToLowerInPlace(ext);
      auto it = mimeTypes.find(ext);
      if (it == mimeTypes.end())
        return DEFAULT_MIME;
      return it->second;
    }

    std::string SerializeResponse(const HTTPResponse& response)
    {
      const std::string& body = response.GetBody();
      std::string out;
      out.reserve(128 + body.size());

      out += response.GetVersion();
      out += " ";
      out += std::to_string(response.GetStatusCode());
      out += " ";
      out += response.GetReason();
      out += HTTP::kCRLF;

      // Write headers except content-length (we compute it)
      for (const auto& [key, values] : response.GetHeaders())
      {
        if (key == "content-length" || key == "transfer-encoding")
          continue;

        const std::string printedKey = String::CanonicalizeHeader(key);

        for (const auto& v : values)
        {
          out += printedKey;
          out += ": ";
          out += v;
          out += HTTP::kCRLF;
        }
      }

      out += "Content-Length: ";
      out += std::to_string(body.size());
      out += HTTP::kCRLF;

      out += HTTP::kCRLF;
      out += body;
      return out;
    }

  }  // namespace wire
}  // namespace HTTP
