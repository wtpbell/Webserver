/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   Builder.hpp                                         :+:    :+:           */
/*                                                      +:+                   */
/*   By: jstuhrin <jstuhrin@student.codam.nl>          +#+                    */
/*                                                    +#+                     */
/*   Created: 2026/02/12 10:14:00 by jstuhrin       #+#    #+#                */
/*   Updated: 2026/02/12 10:14:00 by jstuhrin       ########   odam.nl        */
/*                                                                            */
/* ************************************************************************** */

#ifndef BUILDER_HPP
#define BUILDER_HPP

#include <cstdint>
#include <string>
#include <map>
#include <vector>
#include <set>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "ValidatorIpPort.hpp"
#include "ServerRegistry.hpp"
#include "RouteView.hpp"
#include "ServerView.hpp"

class Builder
{
  public:
    Builder(Lexer& lexer, Parser& parser, const ValidatorIpPort& validatorIpPort_);
    Builder(const Builder& other) = delete;
    Builder(Builder&& other) = delete;
    Builder& operator=(const Builder& other) = delete;
    Builder& operator=(Builder&& other) = delete;
    ~Builder() = default;

    bool GetError() const;
    ServerRegistry BuildServerRegistry();

  private:
    std::size_t GetNumListenDirectives(const Node& serverBlock);
    void ValidateIpPortHostName();
    void PopulateServerViewMap();
    void PopulateRouteViewMap();
    void PopulateDefaultServerRouteViewMap();
    void SetErrorMessage(std::size_t line, std::size_t col, std::string_view lexeme, std::string_view message, std::size_t tokenIndex);
    void Error(Node& dir, std::string_view message);
    void ValidateAndExtractListen(Node& serverBlock, ServerView& serverView);
    void ValidateAndExtractServerNames(Node& serverBlock, ServerView& serverView);
    void ExtractCgi(const Node& httpBlock, RouteView& routeView);
    void ExtractCgiExtension(const Node& httpBlock, RouteView& routeView);
    void ExtractIndex(const Node& node, RouteView& routeView);
    void ExtractAutoindex(const Node& node, RouteView& routeView);
    void ExtractClientMaxBodySize(const Node& node, RouteView& routeView);
    void ExtractAllowedMethods(const Node& node, RouteView& routeView);
    void ExtractReturn(const Node& node, RouteView& routeView);
    void ExtractRoot(const Node& node, RouteView& routeView);
    void ExtractAlias(const Node& node, RouteView& routeView);
    void ExtractErrorPage(const Node& node, RouteView& routeView);
    void ValidateAndExtractLocationPrefix(Node& location, RouteView& routeView, std::set<std::string>& locationPrefixSet);
    void ExtractHttpData();
    void ExtractServerBlockDirectivesRouteView(const Node& serverBlock, RouteView& routeView);
    void ExtractServerBlockDirectivesServerView(Node& serverBlock, ServerView& serverView);
    void ExtractServerBlockData(Node& http, RouteView& routeView);
    void ExtractLocationDirectives(const Node& location, RouteView& routeView);
    void ExtractLocationData(Node& serverBlock, ServerView& serverView, RouteView& routeView, std::set<std::string>& locationPrefixSet);
    void SetServerViewDefaults(Node& serverBlock, ServerView& serverView);

    Lexer& lexer_;
    Parser& parser_;
    const ValidatorIpPort& validatorIpPort_;
    bool error_;
    const std::string defaultPort_;
    const std::string defaultIp_;
    const std::uint16_t defaultReturnCode_;

    std::vector<ServerView> serverViews_;
    std::map<ServerView::IpPort, std::vector<ServerView*>> serverViewMap_;
    std::map<ServerView::IpPort, std::map<std::string, std::map<std::string, RouteView*>>> routeViewMap_;
    std::map<ServerView::IpPort, std::map<std::string, RouteView*>> defaultServerRouteViewMap_;

    std::vector<Node*> serverNameNodes_;
    std::vector<Node*> ipPortNodes_;
    std::vector<bool> hasDefaultServerFlagVector_;
    std::set<ServerView::IpPort> hasDefaultServerFlagSet_;

    static constexpr std::string_view kRed_ = "\033[31m";
    static constexpr std::string_view kReset_ = "\033[0m";

#ifdef UNIT_TEST
  public:
    const ServerView* GetServersData() const
    {
      return serverViews_.data();
    }
    const std::map<std::string, std::map<std::string, RouteView*>>* GetAddressValue(const ServerView::IpPort& key) const
    {
      return &routeViewMap_.at(key);
    }
#endif
};

#endif
