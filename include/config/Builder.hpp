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
#include <filesystem>
#include <optional>
#include <map>
#include <vector>
#include <utility>

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
    void PopulateRouteViewMap();
    void SetErrorMessage(std::size_t line, std::size_t col, std::string_view lexeme, std::string_view message, std::size_t tokenIndex);
    void Error(Node& dir, std::string_view message);
    bool ValidateIpPortDuplicate(Node& dir, const std::string& ip, const std::uint16_t port, const ServerView& serverView);
    void ExtractListen(Node& serverBlock, ServerView& serverView);
    void ExtractServerName(const Node& serverBlock, ServerView& serverView);
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
    void ExtractLocationPrefix(const Node& location, RouteView& routeView);
    void ExtractHttpData();
    void ExtractServerDirectives(Node& server, ServerView& serverView, RouteView& routeView);
    void ExtractServerData(Node& http, RouteView& routeView);
    void ExtractLocationDirectives(const Node& location, RouteView& routeView);
    void ExtractLocationData(const Node& server, RouteView& routeView, std::size_t i);
    void SetServerViewDefaults(Node& server, ServerView& serverView);

    Lexer& lexer_;
    Parser& parser_;
    const ValidatorIpPort& validatorIpPort_;
    bool error_;
    const std::uint16_t defaultPort_;
    const std::string defaultIp_;
    const std::uint16_t defaultReturnCode_;
    std::vector<ServerView> servers_;
    std::map<std::pair<std::string, std::uint16_t>, std::map<std::string, std::map<std::string, RouteView*>>> RouteViewMap_;

    static constexpr std::string_view kRed_ = "\033[31m";
    static constexpr std::string_view kReset_ = "\033[0m";

#ifdef UNIT_TEST
  public:
    const ServerView* GetServersData() const
    {
      return servers_.data();
    }
    const std::map<std::string, std::map<std::string, RouteView*>>* GetAddressValue(const std::pair<std::string, std::uint16_t>& key) const
    {
      return &RouteViewMap_.at(key);
    }
#endif
};

#endif
