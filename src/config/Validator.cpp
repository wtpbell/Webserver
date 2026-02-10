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


#include "config/Lexer.hpp"
#include "config/Parser.hpp"
#include "config/Validator.hpp"

#include <cassert>
#include <iostream>
#include <vector>
#include <array>
#include <set>
#include <map>
#include <cstdint>
#include <charconv>
#include <sstream>

Validator::Validator(Lexer& lexer, Parser& parser)
  : lexer_(lexer)
  , parser_(parser)
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
  supportedReturnCodesStr_.append(" supported status code:");
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

//////////////////// PRIVATE ////////////////////
//////////////////// helper functions ////////////////////

bool Validator::IsParamsEmpty(Node& dir, std::string_view message)
{
  if (dir.params.empty())
  {
    Error("unexpected token", message, dir, true);
    return true;
  }
  return false;
}

void Validator::ValidatePath(Node& node)
{
  if (node.lexeme.front() != '/')
  {
    Error("invalid path", " path must begin with `/`", node, false);
  }
}

bool Validator::IsValidURL(Node& param)
{
  if (param.lexeme.front() == '/' || param.lexeme.substr(0, 7) == "http://")
  {
    return true;
  }
  return false;
}

std::string Validator::BuildErrorMessage(std::string_view errorType, std::string_view message, Node& node, bool displayNextToken)
{
  std::stringstream ss;

  std::size_t line = displayNextToken ? lexer_.GetLine(node.idxTokenList + 1) : node.line;
  std::size_t col = displayNextToken ? lexer_.GetCol(node.idxTokenList + 1) : node.col;
  std::string_view lexeme = displayNextToken ? lexer_.GetLexeme(node.idxTokenList + 1) : node.lexeme;
  Identifier context = displayNextToken ? node.name : node.context;

  ss << line << ":" << col << ": " << kRed_ << "error:" << kReset_ << " "
            << errorType  << ": ";
  if (!lexeme.empty())
  {
    ss << "`" << kRed_ << lexeme << kReset_ << "`";
  }
  ss << message << " context: " << parser_.IdentifierToString(context) << "\n";
  return ss.str();
}

void Validator::Error(std::string_view error_type, std::string_view message, Node& node, bool displayNextToken)
{
  ssize_t idx = displayNextToken ? node.idxTokenList + 1 : node.idxTokenList;
  lexer_.SetTokenErrorMessage(idx, BuildErrorMessage(error_type, message, node, displayNextToken));
  lexer_.SetTokenErrorTrue(idx);
  node.error = true;
  error_ = true;
}

//////////////////// core validation logic ///////////////////

void Validator::ValidateContextDirs(Node& block)
{
  for (Node& dir : block.directives)
  {
    if (!valid_dirs_.at(block.name).count(dir.name))
    {
      Error("invalid context", "", dir, false);
    }
  }
}

void Validator::ValidateContextBlocks(Node& block)
{
  for (Node& nestedBlock : block.nestedBlocks)
  {
    if (!valid_blocks_.at(block.name).count(nestedBlock.name))
    {
      Error("invalid context", "", nestedBlock, false);
    }
  }
}

void Validator::ValidateNumDirs(Node& block)
{
  (void)block;
}

void Validator::ValidateNumBlocks(Node& block)
{
  (void)block;
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
    if (nestedBlock.name == Identifier::Location && !nestedBlock.params.empty())
    {
      ValidatePath(nestedBlock.params.front());
    }
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

void Validator::ValidateIpv4(Node& param, const std::string& ipv4)
{
  std::vector<std::string> octetts;
  std::string::size_type start = 0;
  std::string::size_type end = ipv4.find('.');
  while (end != std::string::npos)
  {
    octetts.emplace_back(ipv4.substr(start, end - start));
    start = end + 1;
    end = ipv4.find('.', start);
  }
  octetts.emplace_back(ipv4.substr(start));
  if (octetts.size() != 4)
  {
    Error("ipv4 address must have four octetts", "", param, false);
    return;
  }
  for (const std::string& octett : octetts)
  {
    for (char c : octett)
    {
      if (!std::isdigit(static_cast<unsigned char>(c)))
      {
        Error("illegal character", "", param, false);
        return;
      }
    }
    if (octett.empty())
    {
      Error("empty octett in ipv4 address", "", param, false);
      return;
    }
    if (octett[0] == '0' && std::isdigit(static_cast<unsigned char>(octett[1])))
    {
      Error("leading zero", "", param, false);
      return;
    }
    std::size_t octettNum{};
    auto [ptr, ec] = std::from_chars(octett.data(), octett.data() + octett.size(), octettNum);
    if (ec == std::errc::result_out_of_range)
    {
      Error("overflow", "", param, false);
      return;
    }
    if (octettNum > 255)
    {
      Error("number too large", "", param, false);
      return;
    }
  }
}

bool Validator::ValidateDoubleColon(Node& param, const std::string& ipv6)
{
  std::string::size_type posDoubleColon = ipv6.find("::");
  if (posDoubleColon == std::string::npos)
  {
    return true;
  }
  posDoubleColon = ipv6.find("::", posDoubleColon + 1);
  if (posDoubleColon == std::string::npos)
  {
    return true;
  }
  Error("unexpected `::`", "", param, false);
  return false;
}

bool Validator::ExpandIpv6Left(Node& param, const std::string& ipv6, std::array<std::string, 8>&  quartets, std::size_t& numQuartets)
{
  std::string::size_type start = 0;
  std::string::size_type end = ipv6.find("::");
  if (end == std::string::npos)
  {
    end = ipv6.size();
  }
  else
  {
    ++numQuartets;
  }
  std::string ipv6Left = ipv6.substr(start, end - start);
  if (ipv6Left.empty())
  {
    return true;
  }
  if (ipv6Left.front() == ':' || ipv6Left.back() == ':')
  {
    Error("empty quartet in ipv6 address", "", param, false);
    return false;
  }
  end = ipv6Left.find(':');
  std::size_t i = 0;
  while (true)
  {
    if (end == std::string::npos)
    {
      end = ipv6Left.size();
    }
    ++numQuartets;
    if (numQuartets > 8)
    {
      Error("number of quartets too large", "", param, false);
      return false;
    }
    quartets[i] = ipv6Left.substr(start, end - start);
    
    ++i;
    if (end == ipv6Left.size())
    {
      break;
    }
    start = end + 1;
    end = ipv6Left.find(':', start);
  }
  return true;
}

bool Validator::ExpandIpv6Right(Node& param, const std::string& ipv6, std::array<std::string, 8>& quartets, std::size_t& numQuartets)
{
  if (ipv6.find("::") == std::string::npos)
  {
    return true;
  }
  std::string::size_type start = ipv6.find("::") + 2;
  std::string::size_type end = ipv6.size();
  std::string ipv6Right = ipv6.substr(start, end - start);
  if (ipv6Right.empty())
  {
    return true;
  }
  if (ipv6Right.front() == ':' || ipv6Right.back() == ':')
  {
    Error("empty quartet in ipv6 address", "", param, false);
    return false;
  }
  std::string::size_type colon = ipv6Right.find_last_of(':');
  start = colon + 1;
  end = ipv6Right.size();
  std::size_t i = 7;
  while (true)
  {
    if (colon == std::string::npos)
    {
      start = 0;
    }
    ++numQuartets;
    if (numQuartets > 8)
    {
      Error("number of quartets too large", "", param, false);
      return false;
    }
    quartets[i] = ipv6Right.substr(start, end - start);
    --i;
    if (start == 0)
    {
      break;
    }
    end = colon;
    colon = ipv6Right.find_last_of(':', colon - 1);
    start = colon + 1;
  }
  return true;
}

bool Validator::ExpandIpv6(Node& param, const std::string& ipv6, std::array<std::string, 8>& quartets)
{
  std::string::size_type end = ipv6.find(':');
  if (end == std::string::npos)
  {
    Error("invalid ipv6 address", "", param, false);
    return false;
  }
  std::size_t numQuartets = 0;
  bool hasZeroCompression = ipv6.find("::") == std::string::npos ? false : true;
  if (!ExpandIpv6Left(param, ipv6, quartets, numQuartets) || !ExpandIpv6Right(param, ipv6, quartets, numQuartets))
  {
    return false;
  }
  if (!hasZeroCompression && numQuartets < 8)
  {
    Error("too few quartets in ipv6 address", "", param, false);
    return false;
  }
  return true;
}

void Validator::ValidateQuartets(Node& param, std::array<std::string, 8>& quartets)
{
  for (const std::string& q : quartets)
  {
    if (q.size() > 4)
    {
      Error("ipv6 quartet too long", "", param, false);
      return;
    }
    for (const char c : q)
    {
      if (!std::isxdigit(static_cast<unsigned char>(c)))
      {
        Error("illegal character in ipv6 quartet", "", param, false);
        return;
      }
    }
  }
}

void Validator::ValidateIpv6(Node& param, const std::string& ipv6)
{
  if (ipv6.empty())
  {
    Error("empty ipv6 address", "", param, false);
    return;
  }
  if (!ValidateDoubleColon(param, ipv6))
  {
    return;
  }
  std::array<std::string, 8> quartets;
  for (std::string& q : quartets)
  {
    q = "0000";
  }
  if (!ExpandIpv6(param, ipv6, quartets))
  {
    return;
  }
  ValidateQuartets(param, quartets);
}

void  Validator::ValidatePortNum(Node& param, const std::string& portNum)
{
  for (const char c : portNum)
  {
    if (!std::isdigit(static_cast<unsigned char>(c)))
    {
      Error("illegal character", "", param, false);
      return;
    }
  }
  if (portNum.size() > 1 && portNum[0] == '0')
  {
    Error("leading zero", "", param, false);
    return;
  }
  std::size_t num{};
  auto [ptr, ec] = std::from_chars(portNum.data(), portNum.data() + portNum.size(), num);
  if (ec == std::errc())
  {
    if (num > 65535)
    {
      Error("port number out of range", "", param, false);
    }
  }
  else if (ec == std::errc::result_out_of_range)
  {
    Error("overflow", "", param, false);
  }
}

bool Validator::ExtractIpv4(Node& param, std::string& ipv4)
{
  if (param.lexeme.front() == '[' || param.lexeme.find('.') == std::string::npos)
  {
    return false;
  }
  std::string::size_type end = param.lexeme.find(':');
  if (end == std::string::npos)
  {
    ipv4 = param.lexeme;
    return true;
  }
  ipv4 = param.lexeme.substr(0, end);
  return true;
}

bool Validator::ExtractIpv6(Node& param, std::string& ipv6)
{
  if (param.lexeme.front() != '[')
  {
    return false;
  }
  std::string::size_type end = param.lexeme.find(']');
  if (end == std::string::npos)
  {
    Error("missing `]`", "", param, false);
    return false;
  }
  ipv6 = param.lexeme.substr(1, end - 1);
  return true;
}

bool Validator::ExtractPortNum(Node& param, std::string& portNum)
{
  if (param.lexeme.front() == '[')
  {
    std::string::size_type posRBrace = param.lexeme.find(']');
    if (posRBrace == std::string::npos)
    {
      return false;
    }
    if (posRBrace + 1 != param.lexeme.size() && param.lexeme[posRBrace + 1] != ':')
    {
      Error("illegal character", "", param, false);
    }
    std::string::size_type start = param.lexeme.find(':', posRBrace);
    if (start == std::string::npos)
    {
      return false;
    }
    portNum = param.lexeme.substr(start + 1);
    return true;
  }
  if (param.lexeme.find('.') != std::string::npos)
  {
    std::string::size_type start = param.lexeme.find(':');
    if (start == std::string::npos)
    {
      return false;
    }
    portNum = param.lexeme.substr(start + 1);
    return true;
  }
  portNum = param.lexeme;
  return true;
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
  std::string ipv4; 
  std::string ipv6;
  std::string portNum;
  if (ExtractIpv4(dir.params[0], ipv4))
  {
    ValidateIpv4(dir.params[0], ipv4);
  }
  else if (ExtractIpv6(dir.params[0], ipv6))
  {
    ValidateIpv6(dir.params[0], ipv6);
  }
  if(ExtractPortNum(dir.params[0], portNum))
  {
    ValidatePortNum(dir.params[0], portNum);
  }
  for (std::size_t i = 1; i < dir.params.size(); ++i)
  {
    Error("unexpected token", " expected: `;`", dir.params[i], false);
  }
}

//////////////////// server_name //////////////////

void Validator::ValidateName(Node& param)
{
  if (param.lexeme.size() > 253)
  {
    Error("name exceeds maximum length of 253 characters", "", param, false);
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
        Error("name contains illegal character", "", param, false);
        return;
      }
    }
    if (label.size() == 0)
    {
      Error("label has length zero", "", param, false);
      return;
    }
    if (label.size() > 63)
    {
      Error("label exceeds maximum length of 63 characters", "", param, false);
      return;
    }
    if (label.front() == '-')
    {
      Error("label starts with hyphen", "", param, false);
      return;
    }
    if (label.back() == '-')
    {
      Error("label ends with hyphen", "", param, false);
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
  Default:  root html;
  Context:  http, server, location, if in location
  */
  if (IsParamsEmpty(dir, " expected: PATH"))
  {
    return;
  }
  ValidatePath(dir.params[0]);
  for (std::size_t i = 1; i < dir.params.size(); ++i)
  {
    Error("unexpected token", " expected: `;`", dir.params[i], false);
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
  ValidatePath(dir.params[0]);
  for (std::size_t i = 1; i < dir.params.size(); ++i)
  {
    Error("unexpected token", " expected: `;`", dir.params[i], false);
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
    Error("invalid unit prefix", " valid unit prefixes: `k`, `m`, `g`, `K`, `M`, `G`", param, false);
    return false;
  }
  unitPrefix.front() = std::tolower(static_cast<unsigned char>(unitPrefix.front()));
  if (unitPrefix != "k" && unitPrefix != "m" && unitPrefix != "g")
  {
    Error("invalid unit prefix", " valid unit prefixes: `k`, `m`, `g`, `K`, `M`, `G`", param, false);
    return false;
  }
  return true;
}

bool Validator::ValidateNumPart(Node& param, const std::string& numPart, std::size_t& number)
{
  if (numPart.empty())
  {
    Error("invalid size parameter", "", param, false);
    return false;
  }
  if (numPart.front() == '0' && std::isdigit(static_cast<unsigned char>(numPart[1])))
  {
  Error("leading zeros", "", param, false);
  return false;
  }
  auto [ptr, ec] = std::from_chars(numPart.data(), numPart.data() + numPart.size(), number);
  if (ec == std::errc::result_out_of_range)
  {
    Error("overflow", "", param, false);
    return false;
  }
  if (ec == std::errc() && number == 0)
  {
    Error("size must not be zero", "", param, false);
    return false;
  }
  return true;
}

void Validator::ValidateNumTimesUnitPrefix(Node& param, const std::size_t& number, const std::string& unitPrefix)
{
  if (!unitPrefix.empty())
  {
    if ((unitPrefix == "k" && number > SIZE_MAX / 1024) ||
        (unitPrefix == "m" && number > SIZE_MAX / 1048576) ||
        (unitPrefix == "g" && number > SIZE_MAX / 1073741824))
    {
      Error("overflow", "", param, false);
    }
  }
}

void Validator::ValidateSize(Node& param)
{
  std::size_t posFirstNonDigit = param.lexeme.find_first_not_of("0123456789");
  std::string numPart = param.lexeme.substr(0, posFirstNonDigit);
  std::string unitPrefix = posFirstNonDigit == std::string::npos ? "" : param.lexeme.substr(posFirstNonDigit, param.lexeme.size() - posFirstNonDigit);
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
    Error("unexpected token", " expected: `;`", dir.params[i], false);
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
  ValidatePath(dir.params.front());
  for (std::size_t i = 1; i < dir.params.size(); ++i)
  {
    Error("unexpected token", " expected: `;`", dir.params[i], false);
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
    Error("unexpected token", " expected: CODE ... URI", dir.params[0], true);
  }
  std::size_t i = 0;
  for (; i < dir.params.size() - 1; ++i)
  {
    if (supportedErrorPageCodesSet_.count(dir.params[i].lexeme) == 0)
    {
      Error("unknown http status code", supportedErrorPageCodesStr_, dir.params[i], false);
    }
  }
  if (!IsValidURL(dir.params[i]))
  {
    Error("invalid URL prefix", " URL may begin with `/` or `http://`", dir.params[i], false);
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
  if (IsParamsEmpty(dir, " expected: CODE"))
  {
    return;
  }
  std::size_t i = 0;
  if (!IsValidReturnCode(dir.params[i]) && !IsValidURL(dir.params[i]))
  {
    Error("invalid parameter", " expected CODE or URL", dir.params[i], false);
    return;
  }
  if (IsValidReturnCode(dir.params[i]))
  {
    ++i;
    if (dir.params.size() >= 2 && !IsValidURL(dir.params[i]))
    {
      Error("invalid URL prefix", " URL may begin with `/` or `http://`", dir.params[i], false);
    }
    ++i;
    for (; i < dir.params.size(); ++i)
    {
      Error("unexpected token", " expected `;`", dir.params[i], false);
    }
    return;
  }
  if (IsValidURL(dir.params[i]))
  {
    ++i;
    for (; i < dir.params.size(); ++i)
    {
      Error("unexpected token", " expected `;`", dir.params[i], false);
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
      Error("unexpected token", " expected: GET | POST | DELETE", param, false);
    }
    else if (IsDuplicate(param.lexeme, counts))
    {
      Error("duplicate token", "", param, false);
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
    Error("unexpected token", " expected: `on` | `off`", dir.params.front(), false);
  }
  for (std::size_t i = 1; i < dir.params.size(); ++i)
  {
    Error("unexpected token", " expected: ';'", dir.params[i], false);
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