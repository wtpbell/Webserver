/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGIParser.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/03/17 00:27:10 by jboon         #+#    #+#                 */
/*   Updated: 2026/03/17 00:27:13 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "cgi/CGIParser.hpp"

#include <algorithm>
#include <cctype>
#include <optional>
#include <string>
#include <string_view>

#include "cgi/CGIResponse.hpp"
#include "string.hpp"

namespace cgi
{
  // Parsing rules can be found here: https://datatracker.ietf.org/doc/html/rfc3875

  // token         = 1*<any CHAR except CTLs or separators>
  // CHAR          = alpha | digit | separator | "!" | "#" | "$" | "%" | "&" | "'" | "*" | "+" | "-" | "." |
  //                 "`" | "^" | "_" | "{" | "|" | "}" | "~" | CTL
  // CTL           = <any control character>
  // separator     = "(" | ")" | "<" | ">" | "@" | "," | ";" | ":" | "\" | <"> | "/" | "[" | "]" | "?" | "=" | "{" | "}"
  //                 | SP | HT
  bool CGIParser::IsToken(std::string_view input)
  {
    using namespace std::string_view_literals;
    return !input.empty() && std::all_of(input.begin(), input.end(),
                                         [](unsigned char uc)
                                         {
                                           return std::isalnum(uc) ||
                                                  "!#$%&'*+-.`^_{|}~"sv.find(uc) != std::string_view::npos;
                                         });
  }

  // quoted-string = <"> *qdtext <">
  // qdtext        = <any CHAR except <"> and CTLs but including LWSP>
  // CTL           = <any control character>
  // LWSP          = SP | HT | NL
  // @Note: NL is excluded in this function to make parsing easier
  bool CGIParser::IsQuotedString(std::string_view input)
  {
    return input.length() > 1 && input[0] == '"' && input[input.length() - 1] == '"' &&
           std::all_of(input.begin() + 1, input.end() - 1,
                       [](unsigned char uc)
                       {
                         return uc != '"' && (uc == '\t' || std::isprint(uc));
                       });
  }

  // TEXT          = <any printable character>
  bool CGIParser::IsText(std::string_view input)
  {
    return !input.empty() && std::all_of(input.begin(), input.end(),
                                         [](unsigned char uc)
                                         {
                                           return std::isprint(uc);
                                         });
  }

  // parameter    = attribute "=" value
  // attribute    = token
  // value        = token | quoted-string
  bool CGIParser::IsParameter(std::string_view input)
  {
    input = String::Trim(input);
    std::size_t equal_sign = input.find_first_of('=');
    if (equal_sign == input.npos)
      return false;

    std::string_view attribute{input.substr(0, equal_sign)};
    std::string_view value{input.substr(equal_sign + 1)};
    return IsToken(attribute) && (IsQuotedString(value) || IsToken(value));
  }

  // media-type   = type "/" subtype *( ";" parameter )
  // type         = token
  // subtype      = token
  // parameter    = attribute "=" value
  bool CGIParser::IsMediaType(std::string_view input)
  {
    if (input.empty())
      return false;

    std::size_t slash = input.find_first_of('/');
    if (slash == std::string_view::npos)
      return false;

    std::size_t semicolon = input.find_first_of(';', slash);
    if (semicolon == std::string_view::npos)
      semicolon = input.length();

    std::string_view type{input.substr(0, slash)};
    std::string_view subtype{String::RightTrim(input, semicolon, slash + 1)};
    if (!IsToken(type) || !IsToken(subtype))
      return false;

    while (semicolon != input.length())
    {
      std::size_t end_of_parameter = input.find_first_of(';', semicolon + 1);
      if (end_of_parameter == input.npos)
        end_of_parameter = input.length();

      if (!IsParameter(input.substr(semicolon + 1, end_of_parameter - semicolon - 1)))
        return false;

      semicolon = end_of_parameter;
    }

    return true;
  }

  // NL is excluded from this to keep parsing simple
  // field-content   = *( token | separator | quoted-string )
  bool CGIParser::IsFieldContent(std::string_view input)
  {
    using namespace std::string_view_literals;

    bool is_open_quote = false;
    for (unsigned char uc : input)
    {
      if (uc == '"')
      {
        is_open_quote = !is_open_quote;
        continue;
      }

      if ((!is_open_quote && (std::isalnum(uc) || "!#$%&'*+-.`^_{|}~"sv.find(uc) != std::string_view::npos)) ||
          (is_open_quote && std::isprint(uc)) || ("()<>@,;:\\/[]?={} \t"sv.find(uc) != std::string_view::npos))
      {
        continue;
      }
      return false;
    }
    return !is_open_quote;
  }

  /*
   * Location        = local-Location | client-Location
   * client-Location = "Location:" fragment-URI NL
   * local-Location  = "Location:" local-pathquery NL
   * fragment-URI    = absoluteURI [ "#" fragment ]
   * fragment        = *uric
   * local-pathquery = abs-path [ "?" query-string ]
   * abs-path        = "/" path-segments
   * path-segments   = segment *( "/" segment )
   * segment         = *pchar
   * pchar           = unreserved | escaped | extra
   * extra           = ":" | "@" | "&" | "=" | "+" | "$" | ","
   * hex             = digit | "A" | "B" | "C" | "D" | "E" | "F" | "a" | "b"
   *                   | "c" | "d" | "e" | "f"
   * escaped         = "%" hex hex
   * unreserved      = alpha | digit | mark
   * mark            = "-" | "_" | "." | "!" | "~" | "*" | "'" | "(" | ")"
   *
   * @note: The validating is simplified and therefor not as strict as the rfc
   */
  bool CGIParser::IsLocation(std::string_view input)
  {
    std::string_view allowed_chars{};
    int hex_count{0};

    if (input.compare(0, 8, "https://") == 0 || input.compare(0, 7, "http://") == 0)
    {
      allowed_chars = "-_.~:/?#[]@!$&'()*+,;=%<>\\^`{}";
      input.remove_prefix(input.find("//", 0) + 2);
    }
    else if (!input.empty() && input[0] == '/')
    {
      if (input.length() == 1)
      {
        return true;
      }
      allowed_chars = "-_.!~*'()%:@&=+$,/";
      input.remove_prefix(1);
    }
    else
    {
      return false;
    }

    if (input.empty())
    {
      return false;
    }

    for (unsigned char uc : input)
    {
      if (!std::isalnum(uc) && allowed_chars.find(uc) == allowed_chars.npos)
      {
        return false;
      }

      if (hex_count > 0)
      {
        if (!std::isxdigit(uc))
        {
          return false;
        }
        hex_count = ++hex_count == 3 ? 0 : hex_count;
      }

      if (uc == '%')
      {
        hex_count = 1;
      }
    }

    return hex_count == 0;
  }

  /*
   * Status Rule:
   * Status         = "Status:" status-code SP reason-phrase NL
   * status-code    = "200" | "302" | "400" | "501" | extension-code
   * extension-code = 3digit
   * reason-phrase  = *TEXT
   */
  std::optional<Status> CGIParser::ParseStatus(std::string_view input)
  {
    std::size_t status_code;
    if (input.length() < 3 || !String::ConvertToNumber(input.substr(0, 3), status_code))
      return std::nullopt;

    if (input.length() == 3)
      return Status{static_cast<int>(status_code), ""};

    if (input.length() == 4 || input[3] != ' ' || std::isspace(input[4]))
      return std::nullopt;

    std::string_view reason{input.substr(4)};
    if (!IsText(reason))
      return std::nullopt;
    return Status{static_cast<int>(status_code), std::string(reason)};
  }

  /*
   * header-field    = CGI-field | other-field
   * CGI-field       = Content-Type | Location | Status
   * other-field     = protocol-field | extension-field
   * protocol-field  = generic-field
   * extension-field = generic-field
   * generic-field   = field-name ":" [ field-value ] NL
   * field-name      = token
   * field-value     = *( field-content | LWSP )
   * field-content   = *( token | separator | quoted-string )
   */
  std::optional<CGIResponse> CGIParser::Parse(std::string_view raw_response)
  {
    using namespace std::string_view_literals;

    const HeaderFieldType mask = ~(HeaderFieldType::Protocol | HeaderFieldType::Extension);
    HeaderField field;
    HeaderFieldType has_field{HeaderFieldType::None};

    CGIResponse response;
    Parser parser{raw_response, {}, 0};
    bool has_empty_line = false;

    while (!parser.IsEndOfString() && parser.NextLine())
    {
      if (parser.current_line.empty())
      {
        response.SetBody(parser.SubstrRemaining());
        has_empty_line = true;
        break;
      }

      field.ExtractHeaderField(parser.current_line);
      bool is_duplicate_cgi_field = ((has_field & mask) & field.type) != HeaderFieldType::None;
      if (field.type == HeaderFieldType::Invalid || is_duplicate_cgi_field ||
          (field.type == HeaderFieldType::ContentType && !IsMediaType(field.value)) ||
          (field.type == HeaderFieldType::Location && !IsLocation(field.value)) ||
          ((field.type == HeaderFieldType::Protocol || field.type == HeaderFieldType::Extension) &&
           !IsFieldContent(field.value)))
      {
        return std::nullopt;
      }
      else if (field.type == HeaderFieldType::Status)
      {
        auto status = ParseStatus(field.value);
        if (!status.has_value())
        {
          return std::nullopt;
        }
        response.SetStatus(status.value());
      }
      else if (field.type == HeaderFieldType::Protocol && field.header.compare("Set-Cookie") == 0)
      {
        response.AddCookie(std::string(field.value));
      }
      else
      {
        response.EmplaceHeader(field.GetCanonicalizeHeader(), std::string(field.value));
      }
      has_field |= field.type;
    }

    if ((has_field & (HeaderFieldType::Location | HeaderFieldType::Status)) == HeaderFieldType::Location)
    {
      if (response.LocalRedirectTarget())
      {
        response.SetStatus({301, "Moved Permanently"});
      }
      else
      {
        response.SetStatus({302, "Found"});
      }
      if ((has_field & HeaderFieldType::ContentType) == HeaderFieldType::None)
      {
        response.EmplaceHeader("Content-Type", "text/plain; charset=utf-8");
      }
    }

    if (!has_empty_line ||
        (has_field & (HeaderFieldType::ContentType | HeaderFieldType::Location)) == HeaderFieldType::None)
    {
      return std::nullopt;
    }

    return response;
  }

  /* ===============================  PRIVATE =============================== */

  bool CGIParser::Parser::NextLine(void)
  {
    std::size_t nl_pos = data.find_first_of('\n', last_pos);
    if (nl_pos == std::string_view::npos)
    {
      last_pos = data.length();
      return false;
    }

    size_t carriage = nl_pos > 0 && data[nl_pos - 1] == '\r';
    current_line = data.substr(last_pos, nl_pos - carriage - last_pos);
    last_pos = nl_pos + 1;
    return true;
  }

  bool CGIParser::Parser::IsEndOfString(void)
  {
    return last_pos >= data.length();
  }

  std::string_view CGIParser::Parser::SubstrRemaining(void)
  {
    size_t pos = last_pos;
    last_pos = data.length();
    return data.substr(pos);
  }

  void CGIParser::HeaderField::ExtractHeaderField(std::string_view input)
  {
    using namespace std::string_view_literals;

    std::size_t delimiter{input.find_first_of(':')};
    if (delimiter == std::string_view::npos)
    {
      type = HeaderFieldType::Invalid;
      return;
    }

    header = input.substr(0, delimiter);
    value = String::Trim(input.substr(delimiter + 1));

    if (header == "Content-Type")
      type = HeaderFieldType::ContentType;
    else if (header == "Location")
      type = HeaderFieldType::Location;
    else if (header == "Status")
      type = HeaderFieldType::Status;
    else if (header.compare(0, "X-CGI-"sv.length(), "X-CGI-") == 0 && CGIParser::IsToken(header))
      type = HeaderFieldType::Extension;
    else if (CGIParser::IsToken(header))
      type = HeaderFieldType::Protocol;
    else
      type = HeaderFieldType::Invalid;
  }

  std::string CGIParser::HeaderField::GetCanonicalizeHeader(void)
  {
    const HeaderFieldType mask{HeaderFieldType::ContentType | HeaderFieldType::Status | HeaderFieldType::Location |
                               HeaderFieldType::Extension};
    if ((type & mask) != HeaderFieldType::None)
    {
      return std::string(header);
    }
    return String::CanonicalizeHeader(header);
  }

  /* ===============================  FRIEND  =============================== */

  CGIParser::HeaderFieldType operator|(CGIParser::HeaderFieldType lhs, CGIParser::HeaderFieldType rhs)
  {
    return static_cast<CGIParser::HeaderFieldType>(static_cast<int>(lhs) | static_cast<int>(rhs));
  }

  CGIParser::HeaderFieldType operator&(CGIParser::HeaderFieldType lhs, CGIParser::HeaderFieldType rhs)
  {
    return static_cast<CGIParser::HeaderFieldType>(static_cast<int>(lhs) & static_cast<int>(rhs));
  }

  CGIParser::HeaderFieldType operator~(CGIParser::HeaderFieldType type)
  {
    return static_cast<CGIParser::HeaderFieldType>(~static_cast<int>(type));
  }

  CGIParser::HeaderFieldType& operator|=(CGIParser::HeaderFieldType& lhs, CGIParser::HeaderFieldType rhs)
  {
    lhs = lhs | rhs;
    return lhs;
  }

  CGIParser::HeaderFieldType& operator&=(CGIParser::HeaderFieldType& lhs, CGIParser::HeaderFieldType rhs)
  {
    lhs = lhs & rhs;
    return lhs;
  }
}  // namespace cgi
