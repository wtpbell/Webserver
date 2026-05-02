/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPUtils.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/09 13:31:19 by bewong        #+#    #+#                 */
/*   Updated: 2026/04/02 11:52:27 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "http/HTTPUtils.hpp"

#include <sys/stat.h>

#include <cctype>
#include <ctime>
#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>

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
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' || c == '/')
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
          std::size_t byte = 0;

          if (String::ConvertToNumber(str.substr(i + 1, 2), byte, 16) && byte <= 0xFF)
          {
            result.push_back(static_cast<char>(byte));
            i += 2;
            continue;
          }
        }
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

      if (!response.HasHeader("date"))
      {
        char date_buf[64];
        String::GMTCstring(date_buf, sizeof(date_buf));
        out += "Date: ";
        out += date_buf;
        out += HTTP::kCRLF;
      }

      out += "Content-Length: ";
      out += std::to_string(body.size());
      out += HTTP::kCRLF;

      out += HTTP::kCRLF;
      out += body;
      return out;
    }

    std::string GetLastModifiedHttpDate(const std::string& filepath)
    {
      std::error_code ec;
      auto ft = std::filesystem::last_write_time(filepath, ec);
      if (ec)
        return "";

      const std::time_t t = String::FileTimeToTimeT(ft);

      char buf[64];
      String::GMTCstringFromTime(t, buf, sizeof(buf));
      return std::string(buf);
    }
  }  // namespace wire
}  // namespace HTTP
