/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   Builder.cpp                                         :+:    :+:           */
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
#include <cassert>
#include <charconv>
#include <cctype>
#include <string_view>
#include <sstream>

#include "config/Lexer.hpp"
#include "config/Parser.hpp"
#include "config/Validator.hpp"
#include "config/ValidatorIpPort.hpp"
#include "config/Builder.hpp"
#include "config/ServerRegistry.hpp"
#include "config/RouteView.hpp"
#include "config/ServerView.hpp"

Builder::Builder(Lexer& lexer, Parser& parser, const ValidatorIpPort& validatorIpPort)
  : lexer_(lexer)
  , parser_(parser)
  , validatorIpPort_(validatorIpPort)
  , error_(false)
  , defaultPort_("8080")
  , defaultIp_("::")
  , defaultReturnCode_(302)
{
  ExtractHttpData();
  PopulateRouteViewMap();
}

//////////////////// PUBLIC ////////////////////

bool Builder::GetError() const
{
  return error_;
}

ServerRegistry Builder::BuildServerRegistry()
{
  return ServerRegistry(std::move(servers_), std::move(RouteViewMap_));
}

//////////////////// PRIVATE ////////////////////

void Builder::PopulateRouteViewMap()
{
  for (ServerView& serverView : servers_)
  {
    for (ServerView::IpPort& ipPort : serverView.ipPortList)
    {
      for (std::string& hostName : serverView.hostNames)
      {
        for (RouteView& routeView : serverView.routes)
        {
          RouteViewMap_[ServerView::IpPort(ipPort)][hostName].emplace(routeView.locationPrefix, &routeView);
        }
      }
    }
  }
}

void Builder::SetErrorMessage(std::size_t line, std::size_t col, std::string_view lexeme, std::string_view message, std::size_t tokenIndex)
{
  std::stringstream ss;
  ss << line << ":" << col << ": " << kRed_ << "error:" << kReset_ << " unexpected token: `" << kRed_ << lexeme << kReset_ << "`" << message << "\n";
  lexer_.SetTokenErrorMessage(tokenIndex, ss.str());
  lexer_.SetTokenErrorTrue(tokenIndex);
}

void Builder::Error(Node& node, std::string_view message)
{
  if (node.name == Identifier::kListen)
  {
    SetErrorMessage(lexer_.GetLine(node.params[0].idxTokenListStart), lexer_.GetCol(node.params[0].idxTokenListStart), node.params[0].lexeme, message, node.params[0].idxTokenListStart);
  }
  else if (node.name == Identifier::kServer)
  {
    SetErrorMessage(lexer_.GetLine(node.idxTokenListEnd), lexer_.GetCol(node.idxTokenListEnd), lexer_.GetLexeme(node.idxTokenListEnd), message, node.idxTokenListEnd);
  }
  node.error = true;
  error_ = true;
}

bool Builder::ValidateIpPortDuplicate(Node& node, const std::string& ip, const std::string& port, const ServerView& currentServerView)
{
  bool valid = true;
  for (ServerView& serverView : servers_)
  {
    for (const ServerView::IpPort& ipPort : serverView.ipPortList)
    {
      if (ipPort.ip == ip && ipPort.port == port)
      {
        Error(node, " duplicate IP-port pair in different servers");
        valid = false;
      }
    }
  }
  for (const ServerView::IpPort& ipPort : currentServerView.ipPortList)
  {
    if (ipPort.ip == ip && ipPort.port == port)
    {
      Error(node, " duplicate IP-port pair in server");
      valid = false;
    }
  }
  return valid;
}

void Builder::ExtractListen(Node& dir, ServerView& serverView)
{
  const std::string& lexeme = dir.params[0].lexeme;
  std::string ip;
  std::string port;

  if (lexeme[0] == '[')
  {
    std::size_t startIp = 1;
    std::size_t lenIp = lexeme.find(']') - startIp;
    ip = validatorIpPort_.GetNormalizedIpv6(lexeme.substr(startIp, lenIp)).value();
    if (lexeme[lenIp + 2] == ':')
    {
      std::size_t startPort = lenIp + 3;
      std::size_t lenPort = lexeme.size() - startPort;
      port = lexeme.substr(startPort, lenPort);
    }
    else
    {
      port = defaultPort_;
    }
  }
  else if (lexeme.find('.') != std::string::npos)
  {
    if (lexeme.find(':') == std::string::npos)
    {
      ip = lexeme.substr(0, lexeme.size());
      port = defaultPort_;
    }
    else
    {
      std::size_t startIp = 0;
      std::size_t lenIp = lexeme.find(':');
      std::size_t startPort = lenIp + 1;
      std::size_t lenPort = lexeme.size() - startPort;
      ip = lexeme.substr(startIp, lenIp);
      port = lexeme.substr(startPort, lenPort);
    }
  }
  else
  {
    ip = defaultIp_;
    port = lexeme;
  }
  if (ValidateIpPortDuplicate(dir, ip, port, serverView))
  {
    serverView.ipPortList.emplace_back(ServerView::IpPort{ip, port});
  }
}

void Builder::ExtractServerName(const Node& dir, ServerView& serverView)
{
  for (const Node& param : dir.params)
  {
    serverView.hostNames.emplace_back(param.lexeme);
  }
}

void Builder::ExtractCgi(const Node& dir, RouteView& routeView)
{
  routeView.cgi = dir.params[0].lexeme == "on";
}

void Builder::ExtractCgiExtension(const Node& dir, RouteView& routeView)
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
  routeView.autoindex = dir.params[0].lexeme == "on";
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

void Builder::ExtractAllowedMethods(const Node& dir, RouteView& routeView)
{
  routeView.allowedMask = RouteView::MethodMask::kNone;
  for (const Node& param : dir.params)
  {
    if (param.lexeme == "GET")
    {
      routeView.allowedMask = routeView.allowedMask | RouteView::MethodMask::kGet;
    }
    else if (param.lexeme == "POST")
    {
      routeView.allowedMask = routeView.allowedMask | RouteView::MethodMask::kPost;
    }
    else if (param.lexeme == "DELETE")
    {
      routeView.allowedMask = routeView.allowedMask | RouteView::MethodMask::kDelete;
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

void Builder::ExtractErrorPage(const Node& dir, RouteView& routeView)
{
  for (std::size_t i = 0; i < dir.params.size() - 1; ++i)
  {
    std::uint16_t code{};
    std::from_chars(dir.params[i].lexeme.data(), dir.params[i].lexeme.data() + dir.params[i].lexeme.size(), code);
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
      case Identifier::kIndex:
        ExtractIndex(dir, routeView);
        break;
      case Identifier::kClientMaxBodySize:
        ExtractClientMaxBodySize(dir, routeView);
        break;
      case Identifier::kErrorPage:
        ExtractErrorPage(dir, routeView);
        break;
      case Identifier::kAutoindex:
        ExtractAutoindex(dir, routeView);
        break;
      default:
        assert(false && "invalid directive in http block");
        __builtin_unreachable();
    }
  }
  ExtractServerData(http, routeView);
}

void Builder::ExtractServerDirectives(Node& server, ServerView& serverView, RouteView& routeView)
{
  for (Node& dir : server.directives)
  {
    switch (dir.name)
    {
      case Identifier::kListen:
        ExtractListen(dir, serverView);
        break;
      case Identifier::kServerName:
        ExtractServerName(dir, serverView);
        break;
      case Identifier::kAllowedMethods:
        ExtractAllowedMethods(dir, routeView);
        break;
      case Identifier::kReturn:
        ExtractReturn(dir, routeView);
        break;
      case Identifier::kRoot:
        ExtractRoot(dir, routeView);
        break;
      case Identifier::kIndex:
        ExtractIndex(dir, routeView);
        break;
      case Identifier::kAutoindex:
        ExtractAutoindex(dir, routeView);
        break;
      case Identifier::kClientMaxBodySize:
        ExtractClientMaxBodySize(dir, routeView);
        break;
      case Identifier::kErrorPage:
        ExtractErrorPage(dir, routeView);
        break;
      case Identifier::kCgiExtension:
        ExtractCgiExtension(dir, routeView);
        break;
      default:
        assert(false && "invalid directive in server block");
        __builtin_unreachable();
    }
  }
}

void Builder::ExtractServerData(Node& http, RouteView& routeView)
{
  std::size_t currentServerIdx = 0;
  std::size_t lastServerIdx = http.nestedBlocks.size() - 1;
  for (Node& server : http.nestedBlocks)
  {
    ServerView serverView;
    if (currentServerIdx == lastServerIdx)
    {
      ExtractServerDirectives(server, serverView, routeView);
      SetServerViewDefaults(server, serverView);
      servers_.emplace_back(serverView);
      ExtractLocationData(server, routeView, currentServerIdx++);
    }
    else
    {
      RouteView routeViewCopy(routeView);
      ExtractServerDirectives(server, serverView, routeViewCopy);
      SetServerViewDefaults(server, serverView);
      servers_.emplace_back(serverView);
      ExtractLocationData(server, routeViewCopy, currentServerIdx++);
    }
  }
}

void Builder::ExtractLocationDirectives(const Node& location, RouteView& routeView)
{
  for (const Node& dir : location.directives)
  {
    switch (dir.name)
    {
      case Identifier::kAllowedMethods:
        ExtractAllowedMethods(dir, routeView);
        break;
      case Identifier::kReturn:
        ExtractReturn(dir, routeView);
        break;
      case Identifier::kRoot:
        ExtractRoot(dir, routeView);
        break;
      case Identifier::kAlias:
        ExtractAlias(dir, routeView);
        break;
      case Identifier::kIndex:
        ExtractIndex(dir, routeView);
        break;
      case Identifier::kAutoindex:
        ExtractAutoindex(dir, routeView);
        break;
      case Identifier::kClientMaxBodySize:
        ExtractClientMaxBodySize(dir, routeView);
        break;
      case Identifier::kErrorPage:
        ExtractErrorPage(dir, routeView);
        break;
      case Identifier::kCgi:
        ExtractCgi(dir, routeView);
        break;
      case Identifier::kCgiExtension:
        ExtractCgiExtension(dir, routeView);
        break;
      default:
        assert(false && "invalid directive in location block");
        __builtin_unreachable();
    }
  }
}

void Builder::ExtractLocationData(const Node& server, RouteView& routeView, std::size_t currentServerIdx)
{
  if (server.nestedBlocks.empty())
  {
    servers_[currentServerIdx].routes.emplace_back(routeView);
    return;
  }
  std::size_t currentRouteIdx = 0;
  std::size_t lastRouteIdx = server.nestedBlocks.size() - 1;
  for (const Node& location : server.nestedBlocks)
  {
    if (currentRouteIdx == lastRouteIdx)
    {
      ExtractLocationPrefix(location, routeView);
      ExtractLocationDirectives(location, routeView);
      servers_[currentServerIdx].routes.emplace_back(routeView);
    }
    else
    {
      RouteView routeViewCopy(routeView);
      ExtractLocationPrefix(location, routeViewCopy);
      ExtractLocationDirectives(location, routeViewCopy);
      servers_[currentServerIdx].routes.emplace_back(routeViewCopy);
    }
    ++currentRouteIdx;
  }
}

void Builder::SetServerViewDefaults(Node& server, ServerView& serverView)
{
  if (serverView.hostNames.empty())
  {
    serverView.hostNames.emplace_back("");
  }
  if (serverView.ipPortList.empty())
  {
    if (ValidateIpPortDuplicate(server, defaultIp_, defaultPort_, serverView))
    {
      serverView.ipPortList.emplace_back(ServerView::IpPort{defaultIp_, defaultPort_});
    }
  }
}
