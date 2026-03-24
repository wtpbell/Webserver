/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   Validator.cpp                                       :+:    :+:           */
/*                                                      +:+                   */
/*   By: jstuhrin <jstuhrin@student.codam.nl>          +#+                    */
/*                                                    +#+                     */
/*   Created: 2026/02/12 10:14:00 by jstuhrin       #+#    #+#                */
/*   Updated: 2026/02/12 10:14:00 by jstuhrin       ########   odam.nl        */
/*                                                                            */
/* ************************************************************************** */

#include <cstdint>
#include <string>
#include <filesystem>
#include <map>
#include <vector>
#include <utility>
#include <cassert>
#include <charconv>
#include <cctype>
#include <string_view>
#include <sstream>

#include "config/Lexer.hpp"
#include "config/Parser.hpp"
#include "config/Validator.hpp"
#include "config/Builder.hpp"

Builder::Builder( Lexer& lexer, Parser& parser, ValidatorIpPort& validatorIpPort)
  : lexer_(lexer)
  , parser_(parser)
  , validatorIpPort_(validatorIpPort)
  , error_(false)
  , defaultPort_(80)
  , defaultIp_("::")
  , defaultReturnCode_(302)
{}

//////////////////// PUBLIC ////////////////////

void Builder::Build()
{
  ExtractHttpData();
  PopulateRouteViewMap();
  PopulateRouteViewMapIp();
}

std::size_t Builder::GetServerCount() const
{
  return servers_.size();
}

const ServerView& Builder::GetServerView(std::size_t i) const
{
  assert(i < servers_.size());
  return servers_[i];
}

const RouteView* Builder::GetRouteView(const std::string& hostName, const std::string& targetPath) const
{
  auto hostIt = routeViewMap_.find(hostName);
  if (hostIt == routeViewMap_.end())
  {
    return nullptr;
  }
  std::size_t longestMatch = 0;
  RouteView* routeView = nullptr;
  for (auto it = hostIt->second.begin(); it != hostIt->second.end(); ++it)
  {
    std::size_t currentMatch = GetLenMatch(targetPath, it->first);
    if (currentMatch > longestMatch)
    {
      routeView = it->second;
      longestMatch = currentMatch;
    }
  }
  return routeView;
}

const RouteView* Builder::GetRouteView(const std::string& ip, const uint16_t port, const std::string& hostName, const std::string& targetPath) const
{
  auto ipPortIt = routeViewMapIp_.find({ip, port});
  if (ipPortIt == routeViewMapIp_.end())
  {
    return nullptr;
  }
  for (auto it = ipPortIt->second.begin(); it != ipPortIt->second.end(); ++it)
  {
    if (it->first == hostName)
    {
      return GetRouteView(hostName, targetPath);
    }
  }
  return nullptr;
}

bool Builder::GetError() const
{
  return error_;
}

//////////////////// PRIVATE ////////////////////

std::size_t Builder::GetLenMatch(const std::string& targetPath, const std::string& locationPrefix) const
{
  if (locationPrefix.size() > targetPath.size())
  {
    return 0;
  }
  if (targetPath.compare(0, locationPrefix.size(), locationPrefix) != 0)
  {
    return 0;
  }
  if (locationPrefix.back() == '/' && targetPath[locationPrefix.size() - 1] == '/')
  {
    return locationPrefix.size();
  }
  if (targetPath[locationPrefix.size()] != '/' && targetPath[locationPrefix.size()] != '\0')
  {
    return 0;
  }
  return locationPrefix.size();
}

void Builder::PopulateRouteViewMap()
{
  for (ServerView& serverView : servers_)
  {
    for (std::string& hostName : serverView.hostNames)
    {
      for (RouteView& routeView : serverView.routes)
      {
        routeViewMap_[hostName].emplace(routeView.locationPrefix, &routeView);
      }
    }
  }
}

void Builder::PopulateRouteViewMapIp()
{
  for (ServerView& serverView : servers_)
  {
    for (std::pair<std::string, std::uint16_t>& ipPort : serverView.ipPortList)
    {
      for (std::string& hostName : serverView.hostNames)
      {
        for (RouteView& routeView : serverView.routes)
        {
          routeViewMapIp_[ipPort][hostName].emplace(routeView.locationPrefix, &routeView);
        }
      }
    }
  }
}

void Builder::Error(Node& dir, std::string_view errorType, std::string_view message)
{
  std::stringstream ss;

  ss << lexer_.GetLine(dir.params[0].idxTokenListStart) << ":" << lexer_.GetCol(dir.params[0].idxTokenListStart) << ": " << kRed_ << "error:"
    << kReset_ << " " << errorType << ": " << "`" << kRed_ << dir.params[0].lexeme << kReset_ << "`" << message << "\n";
  lexer_.SetTokenErrorMessage(dir.params[0].idxTokenListStart, ss.str());
  lexer_.SetTokenErrorTrue(dir.params[0].idxTokenListStart);
  dir.error = true;
  error_ = true;
}

void Builder::ValidateIpPortDuplicate(Node& dir, const std::string& ip, const std::uint16_t port, const ServerView& currentServerView)
{
  for (const ServerView& serverView : servers_)
  {
    for (const std::pair<std::string, std::uint16_t>& ipPort : serverView.ipPortList)
    {
      if (ipPort.first == ip && ipPort.second == port)
      {
        Error(dir, "unexpected token", " identical IP-port pair in different servers");
      }
    }
  }
  for (const std::pair<std::string, std::uint16_t>& ipPort : currentServerView.ipPortList)
  {
    if (ipPort.first == ip && ipPort.second == port)
    {
      Error(dir, "unexpected token", " duplicate IP-port pair in server");
    }
  }
}

void Builder::ExtractListen(Node& dir, ServerView& serverView)
{
  const std::string& lexeme = dir.params[0].lexeme;
  std::string ip;
  std::string portNum;
  std::uint16_t portInt;

  if (lexeme[0] == '[')
  {
    std::size_t startIp = 1;
    std::size_t lenIp = lexeme.find(']') - startIp;
    ip = validatorIpPort_.GetNormalizedIpv6(lexeme.substr(startIp, lenIp)).value();
    if (lexeme[lenIp + 2] == ':')
    {
      std::size_t startPortNum = lenIp + 3;
      std::size_t lenPortNum = lexeme.size() - startPortNum;
      portNum = lexeme.substr(startPortNum, lenPortNum);
      std::from_chars(portNum.data(), portNum.data() + portNum.size(), portInt);
    }
    else
    {
      portInt = defaultPort_;
    }
  }
  else if (lexeme.find('.') != std::string::npos)
  {
    if (lexeme.find(':') == std::string::npos)
    {
      ip = lexeme.substr(0, lexeme.size());
      portInt = defaultPort_;
    }
    else
    {
      std::size_t startIp = 0;
      std::size_t lenIp = lexeme.find(':');
      std::size_t startPortNum = lenIp + 1;
      std::size_t lenPortNum = lexeme.size() - startPortNum;
      ip = lexeme.substr(startIp, lenIp);
      portNum = lexeme.substr(startPortNum, lenPortNum);
      std::from_chars(portNum.data(), portNum.data() + portNum.size(), portInt);
    }
  }
  else
  {
    ip = defaultIp_;
    portNum = lexeme;
    std::from_chars(portNum.data(), portNum.data() + portNum.size(), portInt);
  }
  ValidateIpPortDuplicate(dir, ip, portInt, serverView);
  serverView.ipPortList.emplace_back(ip, portInt);
}

void Builder::ExtractServer_name(const Node& dir, ServerView& serverView)
{
  for (const Node& param : dir.params)
  {
    serverView.hostNames.emplace_back(param.lexeme);
  }
}

void Builder::ExtractCgi(const Node& dir, RouteView& routeView)
{
  dir.params[0].lexeme == "on" ? routeView.cgi = true : routeView.cgi = false;
}

void Builder::ExtractCgi_extension(const Node& dir, RouteView& routeView)
{
  if (!routeView.cgiExePaths.has_value())
  {
    routeView.cgiExePaths.emplace();
  }
  routeView.cgiExePaths->insert_or_assign(dir.params[0].lexeme, std::filesystem::path(dir.params[1].lexeme));
}

void Builder::ExtractIndex(const Node& dir, RouteView& routeView)
{
  routeView.index = dir.params[0].lexeme;
}

void Builder::ExtractAutoindex(const Node& dir, RouteView& routeView)
{
  dir.params[0].lexeme == "on" ? routeView.autoindex = true : routeView.autoindex = false;
}

void Builder::ExtractClientMaxBodySize(const Node& dir, RouteView& routeView)
{
  const std::string& numberLexeme = dir.params[0].lexeme;
  std::size_t number{};
  std::from_chars(numberLexeme.data(), numberLexeme.data() + numberLexeme.size(), number);
  char last = std::tolower(numberLexeme.back());
  switch (last)
  {
  case 'k':
    number *= 1024;
    break;
  case 'm':
    number *= 1024 * 1024;
    break;
  case 'g':
    number *= 1024 * 1024 * 1024;
    break;
  }
  routeView.clientMaxBody = number;
}

void Builder::ExtractAllowed_methods(const Node& dir, RouteView& routeView)
{
  routeView.allowedMask = RouteView::MethodMask::None;
  for (const Node& param : dir.params)
  {
    if (param.lexeme == "GET")
    {
      routeView.allowedMask = routeView.allowedMask | RouteView::MethodMask::Get;
    }
    else if (param.lexeme == "POST")
    {
      routeView.allowedMask = routeView.allowedMask | RouteView::MethodMask::Post;
    }
    else if (param.lexeme == "DELETE")
    {
      routeView.allowedMask = routeView.allowedMask | RouteView::MethodMask::Delete;
    }
  }
}

void Builder::ExtractReturn(const Node& dir, RouteView& routeView)
{
  routeView.returnRule.emplace(RouteView::ReturnRule{});
  if (std::isdigit(static_cast<unsigned char>(dir.params[0].lexeme.front())))
  {
    std::from_chars(dir.params[0].lexeme.data(), dir.params[0].lexeme.data() + dir.params[0].lexeme.size(), routeView.returnRule->code);
    if (dir.params.size() > 1)
    {
      routeView.returnRule->target = dir.params[1].lexeme;
    }
  }
  else
  {
    routeView.returnRule->code = defaultReturnCode_;
    routeView.returnRule->target = dir.params[0].lexeme;
  }
}

void Builder::ExtractRoot(const Node& dir, RouteView& routeView)
{
  routeView.root = dir.params[0].lexeme;
}

void Builder::ExtractAlias(const Node& dir, RouteView& routeView)
{
  routeView.alias.emplace(std::filesystem::path(dir.params[0].lexeme));
}

void Builder::ExtractError_page(const Node& dir, RouteView& routeView)
{
  for (std::size_t i = 0; i < dir.params.size() - 1; ++i)
  {
    std::uint16_t code{};
    std::from_chars(dir.params[i].lexeme.data(), dir.params[i].lexeme.data() - dir.params[i].lexeme.size(), code);
    routeView.errorPages[code] = dir.params[dir.params.size() - 1].lexeme;
  }
}

void Builder::ExtractLocationPrefix(const Node& location, RouteView& routeView)
{
  routeView.locationPrefix = location.params[0].lexeme;
}

void Builder::ExtractHttpData()
{
  RouteView routeView;
  Node& http = parser_.GetAst().nestedBlocks[0];
  for (Node& dir : http.directives)
  {
    switch (dir.name)
    {
      case Identifier::Index:
        ExtractIndex(dir, routeView);
        break;
      case Identifier::Client_max_body_size:
        ExtractClientMaxBodySize(dir, routeView);
        break;
      case Identifier::Error_page:
        ExtractError_page(dir, routeView);
        break;
      case Identifier::Autoindex:
        ExtractAutoindex(dir, routeView);
        break;
      default:
        break;
    }
  }
  ExtractServerData(http, routeView);
}

void Builder::ExtractServerData(Node& http, const RouteView& routeView)
{
  std::size_t i = 0;
  for (Node& server : http.nestedBlocks)
  {
    ServerView serverView;
    for (Node& dir : server.directives)
    {
      switch (dir.name)
      {
        case Identifier::Listen:
          ExtractListen(dir, serverView);
          break;
        case Identifier::Server_name:
          ExtractServer_name(dir, serverView);
          break;
        default:
          break;
      }
    }
    SetServerViewDefaults(serverView);
    servers_.emplace_back(serverView);
    RouteView routeViewCopy(routeView);
    for (const Node& dir : server.directives)
    {
      switch (dir.name)
      {
        case Identifier::Allowed_methods:
          ExtractAllowed_methods(dir, routeViewCopy);
          break;
        case Identifier::Return:
          ExtractReturn(dir, routeViewCopy);
          break;
        case Identifier::Root:
          ExtractRoot(dir, routeViewCopy);
          break;
        case Identifier::Index:
          ExtractIndex(dir, routeViewCopy);
          break;
        case Identifier::Autoindex:
          ExtractAutoindex(dir, routeViewCopy);
          break;
        case Identifier::Client_max_body_size:
          ExtractClientMaxBodySize(dir, routeViewCopy);
          break;
        case Identifier::Error_page:
          ExtractError_page(dir, routeViewCopy);
          break;
        case Identifier::Cgi_extension:
          ExtractCgi_extension(dir, routeViewCopy);
          break;
        default:
          break;
      }
    }
    ExtractLocationData(server, routeViewCopy, i++);
  }
}

void Builder::ExtractLocationData(const Node& server, RouteView& routeView, std::size_t i)
{
  if (server.nestedBlocks.empty())
  {
    servers_[i].routes.emplace_back(routeView);
    return;
  }
  for (const Node& location : server.nestedBlocks)
  {
    RouteView copy(routeView);
    ExtractLocationPrefix(location, copy);
    for (const Node& dir : location.directives)
    {
      switch (dir.name)
      {
        case Identifier::Allowed_methods:
          ExtractAllowed_methods(dir, copy);
          break;
        case Identifier::Return:
          ExtractReturn(dir, copy);
          break;
        case Identifier::Root:
          ExtractRoot(dir, copy);
          break;
        case Identifier::Alias:
          ExtractAlias(dir, copy);
          break;
        case Identifier::Index:
          ExtractIndex(dir, copy);
          break;
        case Identifier::Autoindex:
          ExtractAutoindex(dir, copy);
          break;
        case Identifier::Client_max_body_size:
          ExtractClientMaxBodySize(dir, copy);
          break;
        case Identifier::Error_page:
          ExtractError_page(dir, copy);
          break;
        case Identifier::Cgi:
          ExtractCgi(dir, copy);
          break;
        case Identifier::Cgi_extension:
          ExtractCgi_extension(dir, copy);
          break;
        default:
          break;
      }
    }
    servers_[i].routes.emplace_back(copy);
  }
}

void Builder::SetServerViewDefaults(ServerView& serverView)
{
  if (serverView.hostNames.empty())
  {
    serverView.hostNames.emplace_back("");
  }
  if (serverView.ipPortList.empty())
  {
    serverView.ipPortList.emplace_back(defaultIp_, defaultPort_);
  }
}
