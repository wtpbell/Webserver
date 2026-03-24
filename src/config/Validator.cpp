/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   Validator.cpp                                       :+:    :+:           */
/*                                                      +:+                   */
/*   By: jstuhrin <jstuhrin@student.codam.nl>          +#+                    */
/*                                                    +#+                     */
/*   Created: 2025/12/17 15:27:54 by jstuhrin       #+#    #+#                */
/*   Updated: 2025/12/17 15:27:55 by jstuhrin       ########   odam.nl        */
/*                                                                            */
/* ************************************************************************** */

#include "config/Validator.hpp"

#include <array>
#include <cassert>
#include <charconv>
#include <cstdint>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <vector>
#include <optional>

#include "config/Lexer.hpp"
#include "config/Parser.hpp"
#include "config/ValidatorIpPort.hpp"

Validator::Validator(Lexer& lexer, Parser& parser, ValidatorIpPort& validatorIpPort)
  : lexer_(lexer)
  , parser_(parser)
  , validatorIpPort_(validatorIpPort)
  , error_(false)
{
  std::string::size_type size = 25 + (supportedErrorPageCodesSet_.size() * 4);
  supportedErrorPageCodesStr_.reserve(size);
  supportedErrorPageCodesStr_.append(" supported status codes:");
  for (std::string_view s : supportedErrorPageCodesSet_)
  {
    supportedErrorPageCodesStr_.append(" ");
    supportedErrorPageCodesStr_.append(s);
  }

  size = 25 + (supportedReturnCodesSet_.size() * 4);
  supportedReturnCodesStr_.reserve(size);
  supportedReturnCodesStr_.append(" supported return codes:");
  for (std::string_view s : supportedReturnCodesSet_)
  {
    supportedReturnCodesStr_.append(" ");
    supportedReturnCodesStr_.append(s);
  }
}

//////////////////// PUBLIC ////////////////////

void Validator::ValidateAst()
{
  ValidateBlock(parser_.GetAst());
}

bool Validator::GetError() const
{
  return error_;
}

void Validator::SetErrorTrue()
{
  error_ = true;
}

//////////////////// PRIVATE ////////////////////
//////////////////// helper functions ////////////////////

bool Validator::IsParamsEmpty(Node& dir, std::string_view message)
{
  if (dir.params.empty())
  {
    Error("unexpected token", message, dir, dir.idxTokenListStart + 1);
    return true;
  }
  return false;
}

bool Validator::IsValidURL(Node& param)
{
  if (param.lexeme.front() == '/' || !param.lexeme.compare(0, 7, "http://") || !param.lexeme.compare(0, 8, "https://"))
  {
    return true;
  }
  return false;
}

std::string Validator::BuildErrorMessage(std::string_view errorType, std::string_view message, std::size_t idx)
{
  std::stringstream ss;

  std::size_t line = lexer_.GetLine(idx);
  std::size_t col = lexer_.GetCol(idx);
  std::string_view lexeme = lexer_.GetLexeme(idx);

  ss << line << ":" << col << ": " << kRed_ << "error:" << kReset_ << " " << errorType << ": " << "`" << kRed_ << lexeme
    << kReset_ << "`" << message << "\n";
  return ss.str();
}

void Validator::Error(std::string_view error_type, std::string_view message, Node& node, std::size_t idx)
{
  lexer_.SetTokenErrorMessage(idx, BuildErrorMessage(error_type, message, idx));
  lexer_.SetTokenErrorTrue(idx);
  node.error = true;
  error_ = true;
}

//////////////////// core validation logic ///////////////////

void Validator::ValidateContextDirs(Node& block)
{
  for (Node& dir : block.directives)
  {
    if (!validDirectives_.at(block.name).count(dir.name))
    {
      Error("invalid context", "", dir, dir.idxTokenListStart);
    }
  }
}

void Validator::ValidateContextBlocks(Node& block)
{
  for (Node& nestedBlock : block.nestedBlocks)
  {
    if (!validBlocks_.at(block.name).count(nestedBlock.name))
    {
      Error("invalid context", "", nestedBlock, nestedBlock.idxTokenListStart);
    }
  }
}

void Validator::ValidateNumDirs(Node& block)
{
  std::set<Identifier> directivesSet;
  for (Node& dir : block.directives)
  {
    if (duplicatesIllegal_.count(dir.name) != 0)
    {
      auto result = directivesSet.insert(dir.name);
      if (result.second == false)
      {
        Error("invalid duplicate", "", dir, dir.idxTokenListStart);
      }
    }
    if (block.name == Identifier::Location)
    {
      if ((dir.name == Identifier::Root && directivesSet.count(Identifier::Alias)) ||
          (dir.name == Identifier::Alias && directivesSet.count(Identifier::Root)))
      {
        Error("unexpected token", " location block must not contain both `alias` and `root`", dir, dir.idxTokenListStart);
      }
    }
  }
}

void Validator::ValidateNumBlocks(Node& block)
{
  if (block.name == Identifier::Main)
  {
    std::size_t http = 0;
    for (Node& nestedBlock : block.nestedBlocks)
    {
      if (nestedBlock.name == Identifier::Http)
      {
        ++http;
        if (http > 1)
        {
          Error("duplicate http", "", block, block.idxTokenListEnd);
        }
      }
    }
    if (http == 0)
    {
      Error("unexpected token", " expected `http`", block, block.idxTokenListEnd);
    }
  }
  else if (block.name == Identifier::Http)
  {
    std::size_t server = 0;
    for (Node& nestedBlock : block.nestedBlocks)
    {
      if (nestedBlock.name == Identifier::Server)
      {
        ++server;
        break;
      }
    }
    if (server == 0)
    {
      Error("unexpected token", " expected `server`", block, block.idxTokenListEnd);
    }
  }
}

void Validator::ValidateNestedDirs(Node& block)
{
  for (Node& dir : block.directives)
  {
    switch (dir.name)
    {
      case Identifier::Listen:
        ValidateListen(dir);
        break;
      case Identifier::Server_name:
        ValidateServer_name(dir);
        break;
      case Identifier::Root:
        ValidateRoot(dir);
        break;
      case Identifier::Index:
        ValidateIndex(dir);
        break;
      case Identifier::Alias:
        ValidateAlias(dir);
        break;
      case Identifier::Client_max_body_size:
        ValidateClient_max_body_size(dir);
        break;
      case Identifier::Client_body_temp_path:
        ValidateClient_body_temp_path(dir);
        break;
      case Identifier::Error_page:
        ValidateError_page(dir);
        break;
      case Identifier::Return:
        ValidateReturn(dir);
        break;
      case Identifier::Allowed_methods:
        ValidateAllowed_methods(dir);
        break;
      case Identifier::Autoindex:
        ValidateAutoindex(dir);
        break;
      case Identifier::Cgi:
        ValidateCgi(dir);
        break;
      case Identifier::Cgi_root:
        ValidateCgi_root(dir);
        break;
      case Identifier::Cgi_alias:
        ValidateCgi_alias(dir);
        break;
      case Identifier::Cgi_extension:
      	ValidateCgi_extension(dir);
      	break;
      default:
        assert(false && "Invalid node in block.directives in ValidateNestedDirs");
        __builtin_unreachable();
    }
  }
}

void Validator::ValidateNestedBlocks(Node& block)
{
  for (Node& nestedBlock : block.nestedBlocks)
  {
    ValidateBlock(nestedBlock);
  }
}

void Validator::ValidateBlock(Node& block)
{
  ValidateContextDirs(block);
  ValidateNumDirs(block);
  ValidateNestedDirs(block);
  ValidateContextBlocks(block);
  ValidateNumBlocks(block);
  ValidateNestedBlocks(block);
}

//////////////////// validation individual directive //////////////////
//////////////////// listen //////////////////

bool Validator::HandleIpv4(Node& dir)
{
  std::string ipv4;
  std::string errorMessage;
  if (validatorIpPort_.ExtractIpv4(dir.params[0].lexeme, ipv4))
  {
    if (!validatorIpPort_.ValidateIpv4(ipv4, errorMessage))
    {
      Error(errorMessage, "", dir.params[0], dir.params[0].idxTokenListStart);
    }
    return true;
  }
  return false;
}

void Validator::HandleIpv6(Node& dir)
{
  std::string ipv6;
  std::string errorMessage;
  if (validatorIpPort_.ExtractIpv6(dir.params[0].lexeme, ipv6, errorMessage))
  {
    if (!validatorIpPort_.ValidateIpv6(ipv6, &errorMessage))
    {
      Error(errorMessage, "", dir.params[0], dir.params[0].idxTokenListStart);
    }
  }
  else if (!errorMessage.empty())
  {
    Error(errorMessage, "", dir.params[0], dir.params[0].idxTokenListStart);
  }
}

void Validator::HandlePortNum(Node& dir)
{
  std::string portNum;
  std::string errorMessage;
  if (validatorIpPort_.ExtractPortNum(dir.params[0].lexeme, portNum, errorMessage))
  {
    if (!validatorIpPort_.ValidatePortNum(portNum, errorMessage))
    {
      Error(errorMessage, "", dir.params[0], dir.params[0].idxTokenListStart);
    }
  }
  else if (!errorMessage.empty())
  {
    Error(errorMessage, "", dir.params[0], dir.params[0].idxTokenListStart);
  }
}

void Validator::ValidateListen(Node& dir)
{
  /*
  Syntax: listen address[:port];
          listen port;
  Default:  listen *:80 | *:8000;
  Context:  server

  Examples: listen 127.0.0.1:8000;
            listen 127.0.0.1;
            listen 8000;
            listen [::]:8000;
            listen [::1];
  */

  if (IsParamsEmpty(dir, " expected <ADDRESS> | <ADDRESS:PORT> | <PORT>"))
  {
    return;
  }
  if (!HandleIpv4(dir))
  {
    HandleIpv6(dir);
  }
  HandlePortNum(dir);
  for (std::size_t i = 1; i < dir.params.size(); ++i)
  {
    Error("unexpected token", " expected: `;`", dir.params[i], dir.params[i].idxTokenListStart);
  }
}

//////////////////// server_name //////////////////

void Validator::ValidateName(Node& param)
{
  if (param.lexeme.size() > 253)
  {
    Error("name exceeds maximum length of 253 characters", "", param, param.idxTokenListStart);
    return;
  }
  std::vector<std::string> labels;
  std::string::size_type start = 0;
  std::string::size_type end = param.lexeme.find('.');
  while (end != std::string::npos)
  {
    labels.emplace_back(param.lexeme.substr(start, end - start));
    start = end + 1;
    end = param.lexeme.find('.', start);
  }
  labels.emplace_back(param.lexeme.substr(start));
  for (const std::string& label : labels)
  {
    for (char c : label)
    {
      if (!islower(static_cast<unsigned char>(c)) && !std::isdigit(static_cast<unsigned char>(c)) && c != '-')
      {
        Error("name contains illegal character", "", param, param.idxTokenListStart);
        return;
      }
    }
    if (label.size() == 0)
    {
      Error("label has length zero", "", param, param.idxTokenListStart);
      return;
    }
    if (label.size() > 63)
    {
      Error("label exceeds maximum length of 63 characters", "", param, param.idxTokenListStart);
      return;
    }
    if (label.front() == '-')
    {
      Error("label starts with hyphen", "", param, param.idxTokenListStart);
      return;
    }
    if (label.back() == '-')
    {
      Error("label ends with hyphen", "", param, param.idxTokenListStart);
      return;
    }
  }
}

void Validator::ValidateServer_name(Node& dir)
{
  /*
  Syntax:   server_name name ...;
  Default:  server_name "";
  Context:  server

  https://man7.org/linux/man-pages/man7/hostname.7.html
  */
  if (IsParamsEmpty(dir, " expected: NAME ..."))
  {
    return;
  }
  for (Node& param : dir.params)
  {
    ValidateName(param);
  }
}

//////////////////// root //////////////////

void Validator::ValidateRoot(Node& dir)
{
  /*
  Syntax:   root path;
  Default:  root -
  Context:  http, server, location
  */
  if (IsParamsEmpty(dir, " expected: PATH"))
  {
    return;
  }
  for (std::size_t i = 1; i < dir.params.size(); ++i)
  {
    Error("unexpected token", " expected: `;`", dir.params[i], dir.params[i].idxTokenListStart);
  }
}

//////////////////// index //////////////////

void Validator::ValidateIndex(Node& dir)
{
  /*
  Syntax:   index file ...;
  Default:  index index.html;
  Context:  http, server, location
  */
  if (IsParamsEmpty(dir, " expected: FILE"))
  {
    return;
  }
}

//////////////////// alias //////////////////

void Validator::ValidateAlias(Node& dir)
{
  /*
  Syntax: alias path;
  Default:  —
  Context:  location
  */
  if (IsParamsEmpty(dir, " expected: PATH"))
  {
    return;
  }
  for (std::size_t i = 1; i < dir.params.size(); ++i)
  {
    Error("unexpected token", " expected: `;`", dir.params[i], dir.params[i].idxTokenListStart);
  }
}

//////////////////// client_max_body_size //////////////////

bool Validator::ValidateUnitPrefix(Node& param, std::string& unitPrefix)
{
  if (unitPrefix.empty())
  {
    return true;
  }
  if (unitPrefix.size() > 1)
  {
    Error("invalid unit prefix", " valid unit prefixes: `k`, `m`, `g`, `K`, `M`, `G`", param, param.idxTokenListStart);
    return false;
  }
  unitPrefix.front() = std::tolower(static_cast<unsigned char>(unitPrefix.front()));
  if (unitPrefix != "k" && unitPrefix != "m" && unitPrefix != "g")
  {
    Error("invalid unit prefix", " valid unit prefixes: `k`, `m`, `g`, `K`, `M`, `G`", param, param.idxTokenListStart);
    return false;
  }
  return true;
}

bool Validator::ValidateNumPart(Node& param, const std::string& numPart, std::size_t& number)
{
  if (numPart.empty())
  {
    Error("invalid size parameter", "", param, param.idxTokenListStart);
    return false;
  }
  if (numPart.front() == '0' && std::isdigit(static_cast<unsigned char>(numPart[1])))
  {
    Error("leading zeros", "", param, param.idxTokenListStart);
    return false;
  }
  auto [ptr, ec] = std::from_chars(numPart.data(), numPart.data() + numPart.size(), number);
  if (ec == std::errc::result_out_of_range)
  {
    Error("overflow", "", param, param.idxTokenListStart);
    return false;
  }
  if (ec == std::errc() && number == 0)
  {
    Error("size must not be zero", "", param, param.idxTokenListStart);
    return false;
  }
  return true;
}

void Validator::ValidateNumTimesUnitPrefix(Node& param, const std::size_t& number, const std::string& unitPrefix)
{
  if (!unitPrefix.empty())
  {
    if ((unitPrefix == "k" && number > SIZE_MAX / 1024) || (unitPrefix == "m" && number > SIZE_MAX / 1048576) ||
        (unitPrefix == "g" && number > SIZE_MAX / 1073741824))
    {
      Error("overflow", "", param, param.idxTokenListStart);
    }
  }
}

void Validator::ValidateSize(Node& param)
{
  std::size_t posFirstNonDigit = param.lexeme.find_first_not_of("0123456789");
  std::string numPart = param.lexeme.substr(0, posFirstNonDigit);
  std::string unitPrefix = posFirstNonDigit == std::string::npos
                               ? ""
                               : param.lexeme.substr(posFirstNonDigit, param.lexeme.size() - posFirstNonDigit);
  std::size_t number{};

  if (!ValidateNumPart(param, numPart, number))
  {
    return;
  }
  if (!ValidateUnitPrefix(param, unitPrefix))
  {
    return;
  }
  ValidateNumTimesUnitPrefix(param, number, unitPrefix);
}

void Validator::ValidateClient_max_body_size(Node& dir)
{
  /*
  Syntax:   client_max_body_size size;
  Default:  client_max_body_size 1m;
  Context:  http, server, location
  */

  if (IsParamsEmpty(dir, " expected: SIZE"))
  {
    return;
  }
  ValidateSize(dir.params[0]);
  for (std::size_t i = 1; i < dir.params.size(); ++i)
  {
    Error("unexpected token", " expected: `;`", dir.params[i], dir.params[i].idxTokenListStart);
  }
}

//////////////////// client_body_temp_path //////////////////

void Validator::ValidateClient_body_temp_path(Node& dir)
{
  /*
  Syntax:   client_body_temp_path path;
  Default:  client_body_temp_path client_body_temp;
  Context:  http, server, location
  */
  if (IsParamsEmpty(dir, " expected: /PATH"))
  {
    return;
  }
  for (std::size_t i = 1; i < dir.params.size(); ++i)
  {
    Error("unexpected token", " expected: `;`", dir.params[i], dir.params[i].idxTokenListStart);
  }
}

//////////////////// error_page //////////////////

void Validator::ValidateError_page(Node& dir)
{
  /*
  Syntax:   error_page code ... [=[response]] uri; // currently no support for [=[response]] - do we want that?
  Default:  —
  Context:  http, server, location
  */
  if (IsParamsEmpty(dir, " expected: CODE ... URL"))
  {
    return;
  }
  if (dir.params.size() < 2)
  {
    Error("unexpected token", " expected: CODE ... URI", dir.params[0], dir.params[0].idxTokenListStart + 1);
  }
  std::size_t i = 0;
  for (; i < dir.params.size() - 1; ++i)
  {
    if (supportedErrorPageCodesSet_.count(dir.params[i].lexeme) == 0)
    {
      Error("unknown http status code", supportedErrorPageCodesStr_, dir.params[i], dir.params[i].idxTokenListStart);
    }
  }
  if (!IsValidURL(dir.params[i]))
  {
    Error("invalid URL prefix", " URL may begin with `/` or `http://` or `https://`", dir.params[i], dir.params[i].idxTokenListStart);
  }
}

//////////////////// return //////////////////

bool Validator::IsValidReturnCode(Node& param)
{
  if (supportedReturnCodesSet_.count(param.lexeme) != 0)
  {
    return true;
  }
  return false;
}

void Validator::ValidateReturn(Node& dir)
{
  /*
  Syntax:   return code;
            return code URL;
            return URL;
  Default:  —
  Context:  server, location
  */
  if (IsParamsEmpty(dir, " expected: CODE or URL"))
  {
    return;
  }
  std::size_t i = 0;
  if (!IsValidReturnCode(dir.params[i]) && !IsValidURL(dir.params[i]))
  {
    Error("invalid parameter", " expected CODE or URL," + supportedReturnCodesStr_, dir.params[i], dir.params[i].idxTokenListStart);
    return;
  }
  if (IsValidReturnCode(dir.params[i]))
  {
    ++i;
    if (dir.params.size() >= 2 && !IsValidURL(dir.params[i]))
    {
      Error("invalid URL prefix", " URL may begin with `/` or `http://` or `https://`", dir.params[i], dir.params[i].idxTokenListStart);
    }
    ++i;
    for (; i < dir.params.size(); ++i)
    {
      Error("unexpected token", " expected `;`", dir.params[i], dir.params[i].idxTokenListStart);
    }
    return;
  }
  if (IsValidURL(dir.params[i]))
  {
    ++i;
    for (; i < dir.params.size(); ++i)
    {
      Error("unexpected token", " expected `;`", dir.params[i], dir.params[i].idxTokenListStart);
    }
  }
}

//////////////////// allowed_methods //////////////////

bool Validator::IsAllowedMethod(std::string_view lexeme)
{
  for (std::string_view method : allowedMethods_)
  {
    if (method == lexeme)
    {
      return true;
    }
  }
  return false;
}

bool Validator::IsDuplicate(std::string_view lexeme, std::array<std::size_t, 3>& counts)
{
  for (std::size_t i = 0; i < 3; ++i)
  {
    if (lexeme == allowedMethods_[i])
    {
      ++counts[i];
      if (counts[i] > 1)
      {
        return true;
      }
    }
  }
  return false;
}

void Validator::ValidateAllowed_methods(Node& dir)
{
  /*
  Syntax:  allowed_methods 1*3method;
  Default: GET
  Context: location
  Note:    each method must be unique
  */
  if (IsParamsEmpty(dir, " expected: `GET` | `POST` | `DELETE`"))
  {
    return;
  }
  std::array<std::size_t, 3> counts{};
  for (Node& param : dir.params)
  {
    if (!IsAllowedMethod(param.lexeme))
    {
      Error("unexpected token", " expected: GET | POST | DELETE", param, param.idxTokenListStart);
    }
    else if (IsDuplicate(param.lexeme, counts))
    {
      Error("duplicate token", "", param, param.idxTokenListStart);
    }
  }
}

//////////////////// validate autoindex and cgi param: on | off //////////////////

void Validator::ValidateOnOffParam(Node& dir)
{
  if (IsParamsEmpty(dir, " expected: `on` | `off`"))
  {
    return;
  }
  if (dir.params.front().lexeme != "on" && dir.params.front().lexeme != "off")
  {
    Error("unexpected token", " expected: `on` | `off`", dir.params.front(), dir.params.front().idxTokenListStart);
  }
  for (std::size_t i = 1; i < dir.params.size(); ++i)
  {
    Error("unexpected token", " expected: ';'", dir.params[i], dir.params[i].idxTokenListStart);
  }
}

//////////////////// autoindex //////////////////

void Validator::ValidateAutoindex(Node& dir)
{
  /*
  Syntax:   autoindex on | off;
  Default:  autoindex off;
  Context:  http, server, location
  */

  ValidateOnOffParam(dir);
}

//////////////////// cgi //////////////////

void Validator::ValidateCgi(Node& dir)
{
  /*
  Syntax:   cgi on | off;
  Default:  cgi off;
  Context:  http, server
  */

  ValidateOnOffParam(dir);
}

//////////////////// cgi_root //////////////////

void Validator::ValidateCgi_root(Node& dir)
{
  /*
  Syntax:   cgi_root path;
  Default:  cgi_root html;
  Context:  server, location
  */
  if (IsParamsEmpty(dir, " expected: PATH"))
  {
    return;
  }
  for (std::size_t i = 1; i < dir.params.size(); ++i)
  {
    Error("unexpected token", " expected: `;`", dir.params[i], dir.params[i].idxTokenListStart);
  }
}

//////////////////// cgi_alias //////////////////

void Validator::ValidateCgi_alias(Node& dir)
{
  /*
  Syntax:   cgi_alias path;
  Default:  —
  Context:  location
  */
  if (IsParamsEmpty(dir, " expected: PATH"))
  {
    return;
  }
  for (std::size_t i = 1; i < dir.params.size(); ++i)
  {
    Error("unexpected token", " expected: `;`", dir.params[i], dir.params[i].idxTokenListStart);
  }
}

//////////////////// cgi_extension //////////////////

void Validator::ValidateCgi_extension(Node& dir)
{
	/*
	Syntax: cgi_extension type path;
	Default: -
	Context: location
	*/
	if (IsParamsEmpty(dir, " expected: TYPE PATH"))
  {
    return;
  }
  if (dir.params.size() == 1)
  {
  	Error("unexpected token", " expected: PATH", dir.params[0], dir.params[0].idxTokenListStart + 1);
  	return;
  }
  for (std::size_t i = 2; i < dir.params.size(); ++i)
  {
    Error("unexpected token", " expected: `;`", dir.params[i], dir.params[i].idxTokenListStart);
  }
}