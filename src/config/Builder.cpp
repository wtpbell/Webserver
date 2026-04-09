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
#include <set>
#include <array>

#include <iostream>

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
  assert(ipPortNodes_.size() == hasDefaultServerFlagVector_.size());
  assert(ipPortNodes_.size() == serverViews_.size());
  if (error_ == false)
  {
    ValidateIpPortHostName();
  }
  if (error_ == false)
  {
    PopulateServerViewMap();
    PopulateRouteViewMap();
    PopulateDefaultServerRouteViewMap();
  }
}

//////////////////// PUBLIC ////////////////////

bool Builder::GetError() const
{
  return error_;
}

ServerRegistry Builder::BuildServerRegistry()
{
  return ServerRegistry(std::move(serverViews_),
    std::move(serverViewMap_),
    std::move(routeViewMap_),
    std::move(defaultServerRouteViewMap_));
}

//////////////////// PRIVATE ////////////////////

void Builder::SetErrorMessage(std::size_t line, std::size_t col, std::string_view lexeme, std::string_view message, std::size_t tokenIndex)
{
  std::stringstream ss;
  ss << line << ":" << col << ": " << kRed_ << "error:" << kReset_ << " unexpected token: `" << kRed_ << lexeme << kReset_ << "` " << message << "\n";
  lexer_.SetTokenErrorMessage(tokenIndex, ss.str());
  lexer_.SetTokenErrorTrue(tokenIndex);
}

void Builder::Error(Node& node, std::string_view message)
{
  switch (node.name)
  {
    case Identifier::kListen:
      SetErrorMessage(lexer_.GetLine(node.params[0].idxTokenListStart), lexer_.GetCol(node.params[0].idxTokenListStart), node.params[0].lexeme, message, node.params[0].idxTokenListStart);
      break;
    case Identifier::kServer:
      SetErrorMessage(lexer_.GetLine(node.idxTokenListEnd), lexer_.GetCol(node.idxTokenListEnd), lexer_.GetLexeme(node.idxTokenListEnd), message, node.idxTokenListEnd);
      break;
    case Identifier::kParam:
    case Identifier::kDefaultServer:
      SetErrorMessage(lexer_.GetLine(node.idxTokenListStart), lexer_.GetCol(node.idxTokenListStart), lexer_.GetLexeme(node.idxTokenListStart), message, node.idxTokenListStart);
      break;
    default:
      assert(false && "invalid Node type in Builder::Error()");
      __builtin_unreachable();
  }
  node.error = true;
  error_ = true;
}

void Builder::ValidateAndExtractListen(Node& dir, ServerView& serverView)
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
  serverView.ipPort.ip = ip;
  serverView.ipPort.port = port;
  ipPortNodes_.emplace_back(&dir.params[0]);
  if (dir.params.size() == 2)
  {
    hasDefaultServerFlagVector_.push_back(true);
    auto result = hasDefaultServerFlagSet_.insert({ip, port});
    if (result.second == false)
    {
      Error(dir.params[1], "expected `;` - repeat use of `default_server`");
    }
  }
  else
  {
    hasDefaultServerFlagVector_.push_back(false);
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

void Builder::ValidateAndExtractLocationPrefix(Node& location, RouteView& routeView, std::set<std::string>& locationPrefixSet)
{
  const std::string& locationPrefix = location.params[0].lexeme;
  auto result = locationPrefixSet.insert(locationPrefix);
  if (result.second == false)
  {
    Error(location.params[0], "duplicate location prefix in server");
  }
  routeView.locationPrefix = locationPrefix;
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
  ExtractServerBlockData(http, routeView);
}

void Builder::ExtractServerBlockDirectivesRouteView(const Node& serverBlock, RouteView& routeView)
{
  for (const Node& dir : serverBlock.directives)
  {
    switch (dir.name)
    {
      case Identifier::kListen:
        break;
      case Identifier::kServerName:
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

std::size_t Builder::GetNumListenDirectives(const Node& serverBlock)
{
  std::size_t numListenDirectives = 0;
  for (const Node& dir : serverBlock.directives)
  {
    if (dir.name == Identifier::kListen)
    {
      ++numListenDirectives;
    }
  }
  return numListenDirectives;
}

void Builder::ValidateAndExtractServerNames(Node& serverBlock, ServerView& serverView)
{
  std::set<std::string> serverNamesSet;
  for (Node& dir : serverBlock.directives)
  {
    if (dir.name == Identifier::kServerName)
    {
      for (Node& param : dir.params)
      {
        auto result = serverNamesSet.insert(param.lexeme);
        if (result.second == false)
        {
          Error(param, "- duplicate hostname in server");
        }   
        serverView.hostNames.emplace_back(param.lexeme);
        serverNameNodes_.emplace_back(&param);
      }
    }
  }
}

void Builder::ExtractServerBlockDirectivesServerView(Node& serverBlock, ServerView& serverView)
{
  std::size_t numListenDirectives = GetNumListenDirectives(serverBlock);
  if (numListenDirectives == 0)
  {
    ValidateAndExtractServerNames(serverBlock,serverView);
    SetServerViewDefaults(serverBlock, serverView);
    serverViews_.emplace_back(serverView);
    return;
  }
  std::size_t currentListenDirective = 1;
  for (Node& dir : serverBlock.directives)
  {
    if (dir.name == Identifier::kListen)
    {
      if (currentListenDirective == numListenDirectives)
      {
        ValidateAndExtractServerNames(serverBlock, serverView);
        ValidateAndExtractListen(dir, serverView);
        SetServerViewDefaults(serverBlock, serverView);
        serverViews_.emplace_back(serverView);
        break;
      }
      else
      {
        ServerView serverViewCopy(serverView);
        ValidateAndExtractServerNames(serverBlock, serverViewCopy);
        ValidateAndExtractListen(dir, serverViewCopy);
        SetServerViewDefaults(serverBlock, serverViewCopy);
        serverViews_.emplace_back(serverViewCopy);
      }
      ++currentListenDirective;
    }
  }
}

void Builder::ExtractServerBlockData(Node& http, RouteView& routeView)
{
  std::size_t currentServerBlockIdx = 0;
  std::size_t lastServerBlockIdx = http.nestedBlocks.size() - 1;
  for (Node& serverBlock : http.nestedBlocks)
  {
    ServerView serverView;
    std::set<std::string> locationPrefixSet;
    if (currentServerBlockIdx == lastServerBlockIdx)
    {
      ExtractServerBlockDirectivesRouteView(serverBlock, routeView);
      ExtractLocationData(serverBlock, serverView, routeView, locationPrefixSet);
    }
    else
    {
      RouteView routeViewCopy(routeView);
      ExtractServerBlockDirectivesRouteView(serverBlock, routeViewCopy);
      ExtractLocationData(serverBlock, serverView, routeViewCopy, locationPrefixSet);
    }
    ExtractServerBlockDirectivesServerView(serverBlock, serverView);
    ++currentServerBlockIdx;
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

void Builder::ExtractLocationData(Node& serverBlock, ServerView& serverView, RouteView& routeView, std::set<std::string>& locationPrefixSet)
{
  if (serverBlock.nestedBlocks.empty())
  {
    serverView.routes.emplace_back(routeView);
    return;
  }
  std::size_t currentRouteIdx = 0;
  std::size_t lastRouteIdx = serverBlock.nestedBlocks.size() - 1;
  for (Node& location : serverBlock.nestedBlocks)
  {
    if (currentRouteIdx == lastRouteIdx)
    {
      ValidateAndExtractLocationPrefix(location, routeView, locationPrefixSet);
      ExtractLocationDirectives(location, routeView);
      serverView.routes.emplace_back(routeView);
    }
    else
    {
      RouteView routeViewCopy(routeView);
      ValidateAndExtractLocationPrefix(location, routeViewCopy, locationPrefixSet);
      ExtractLocationDirectives(location, routeViewCopy);
      serverView.routes.emplace_back(routeViewCopy);
    }
    ++currentRouteIdx;
  }
}

void Builder::SetServerViewDefaults(Node& serverBlock, ServerView& serverView)
{
  if (serverView.hostNames.empty())
  {
    serverView.hostNames.emplace_back("");
    serverNameNodes_.emplace_back(&serverBlock);
  }
  if (serverView.ipPort.ip.empty())
  {
    serverView.ipPort.ip = defaultIp_;
    serverView.ipPort.port = defaultPort_;
    ipPortNodes_.emplace_back(&serverBlock);
    hasDefaultServerFlagVector_.push_back(false);
  }
}

void Builder::ValidateIpPortHostName()
{
  std::size_t serverNameNodesIdx = 0;
  std::size_t ipPortNodesIdx = 0;
  std::set<std::array<std::string, 3>> ipPortHostNamesSet;
  for (const ServerView& serverView : serverViews_)
  {
    for (const std::string& hostName : serverView.hostNames)
    {
      auto result = ipPortHostNamesSet.insert({serverView.ipPort.ip, serverView.ipPort.port, hostName});
      if (result.second == false)
      {
        if (serverNameNodes_[serverNameNodesIdx]->name == Identifier::kServer)
        {
          Error(*serverNameNodes_[serverNameNodesIdx], "expected: `server_name` - duplicate hostname IP:port combination");
        }
        else
        {
          Error(*serverNameNodes_[serverNameNodesIdx], "- duplicate hostname IP:port combination");
        }
        if (ipPortNodes_[ipPortNodesIdx]->name == Identifier::kServer)
        {
          Error(*ipPortNodes_[ipPortNodesIdx], "expected: `listen` - duplicate hostname IP:port combination");
        }
        else
        {
          Error(*ipPortNodes_[ipPortNodesIdx], "- duplicate hostname IP:port combination");
        }
      }
      ++serverNameNodesIdx;
    }
    ++ipPortNodesIdx;
  }
}

void Builder::PopulateServerViewMap()
{
  for (ServerView& serverView : serverViews_)
  {
    serverViewMap_[serverView.ipPort].emplace_back(&serverView);
  }
}

void Builder::PopulateRouteViewMap()
{
  for (ServerView& serverView : serverViews_)
  {
    ServerView::IpPort& ipPort = serverView.ipPort;
    for (const std::string& hostName : serverView.hostNames)
    {
      std::map<std::string, RouteView*>& routes = routeViewMap_[ipPort][hostName];
      for (RouteView& routeView : serverView.routes)
      {
        routes.emplace(routeView.locationPrefix, &routeView);
      }
    }
  }
}

void Builder::PopulateDefaultServerRouteViewMap()
{
  std::size_t idx = 0;
  for (ServerView& serverView : serverViews_)
  {
    ServerView::IpPort& ipPort = serverView.ipPort;
    if (defaultServerRouteViewMap_.count(ipPort) == 0 || hasDefaultServerFlagVector_[idx] == true)
    {
      defaultServerRouteViewMap_.erase(ipPort);
      std::map<std::string, RouteView*>& routes = defaultServerRouteViewMap_[ipPort]; 
      for (RouteView& routeView : serverView.routes)
      {        
        routes.emplace(routeView.locationPrefix, &routeView);
      }
    }
    ++idx;
  }
}
