/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGIParser.hpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/29 14:09:49 by jboon         #+#    #+#                 */
/*   Updated: 2026/03/17 00:27:25 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIPARSER_H_
#define CGIPARSER_H_

#include <optional>
#include <string_view>

#include "cgi/CGIResponse.hpp"

namespace cgi
{
  class CGIParser
  {
    public:
      CGIParser(void) = delete;
      CGIParser(const CGIParser& other) = delete;
      CGIParser(CGIParser&& other) noexcept = delete;
      ~CGIParser(void) = delete;

      CGIParser& operator=(const CGIParser& other) = delete;
      CGIParser& operator=(CGIParser&& other) noexcept = delete;

      static bool IsToken(std::string_view token);
      static bool IsQuotedString(std::string_view quoted_string);
      static bool IsText(std::string_view text);
      static bool IsParameter(std::string_view parameter);
      static bool IsMediaType(std::string_view media_type);
      static bool IsFieldContent(std::string_view field);
      static bool IsLocation(std::string_view location);

      static std::optional<Status> ParseStatus(std::string_view status);
      static std::optional<CGIResponse> Parse(std::string_view raw_response);

    private:
      enum class HeaderFieldType
      {
        None = 0x00,         // None
        Invalid = 0x00,      // Invalid (same as None)
        Protocol = 0x01,     // any generic field that fits token will be considered a protocol field
        Extension = 0x02,    // X-CGI- prefix
        ContentType = 0x04,  // Content-Type: media-type NL
        Location = 0x08,     // Location: local-location | location NL
        Status = 0x10,       // Status: status-code SP reason-phrase NL
        CGIField = ContentType | Location | Status
      };

      struct Parser
      {
          bool NextLine(void);
          bool IsEndOfString(void);
          std::string_view SubstrRemaining(void);

          std::string_view data;
          std::string_view current_line;
          std::size_t last_pos{0};
      };

      struct HeaderField
      {
          void ExtractHeaderField(std::string_view line);
          std::string GetCanonicalizeHeader(void);

          std::string_view header;
          std::string_view value;
          HeaderFieldType type = HeaderFieldType::Invalid;
      };

      friend HeaderFieldType operator|(HeaderFieldType lhs, HeaderFieldType rhs);
      friend HeaderFieldType operator&(HeaderFieldType lhs, HeaderFieldType rhs);
      friend HeaderFieldType operator~(HeaderFieldType type);
      friend HeaderFieldType& operator|=(HeaderFieldType& lhs, HeaderFieldType rhs);
      friend HeaderFieldType& operator&=(HeaderFieldType& lhs, HeaderFieldType rhs);
  };
}  // namespace cgi
#endif  // CGIPARSER_H_
