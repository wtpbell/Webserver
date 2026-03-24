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

#ifndef BUILDER_HPP
#define BUILDER_HPP

#include <cstdint>
#include <string>
#include <filesystem>
#include <optional>
#include <map>
#include <vector>
#include <utility>

#include "Parser.hpp"

struct RouteView
{
  enum class MethodMask : unsigned
  {
    None = 0,
    Get = 1u << 0,
    Post = 1u << 1,
    Delete = 1u << 2,
    All = Get | Post | Delete,
  };

  struct ReturnRule
  {
    std::uint16_t code;
    std::string target;
  };

  std::string locationPrefix = "/";

  std::filesystem::path root = "./www";
  std::optional<std::filesystem::path> alias;
  std::string index = "index.html";

  bool autoindex = false;
  std::size_t clientMaxBody = 1u << 20;
  MethodMask allowedMask = MethodMask::Get;

  std::optional<ReturnRule> returnRule;
  std::map<std::uint16_t, std::string> errorPages;

  bool cgi = false;
  std::optional<std::map<std::string, std::filesystem::path>> cgiExePaths;
};

struct ServerView
{
  std::vector<std::string> hostNames;
  std::vector<std::pair<std::string, std::uint16_t>> ipPortList;
  std::vector<RouteView> routes;
};

class Builder
{
  public:
    Builder(Lexer& lexer, Parser& parser, ValidatorIpPort& validatorIpPort_);
    Builder(const Builder& other) = delete;
    Builder(Builder&& other) = delete;
    Builder& operator=(const Builder& other) = delete;
    Builder& operator=(Builder&& other) = delete;
    ~Builder() = default;

    void Build();
    std::size_t GetServerCount() const;
    const ServerView& GetServerView(std::size_t i) const;
    const RouteView* GetRouteView(const std::string& hostname, const std::string& targetPath) const;
    const RouteView* GetRouteView(const std::string& ip, const uint16_t port, const std::string& hostName, const std::string& targetPath) const;
    bool GetError() const;

  private:
    std::size_t GetLenMatch(const std::string& locationPrefix, const std::string& locationPrefixRouteView) const;
    void PopulateRouteViewMap();
    void PopulateRouteViewMapIp();
    void Error(Node& dir, std::string_view errorType, std::string_view message);
    void ValidateIpPortDuplicate(Node& dir, const std::string& ip, const std::uint16_t port, const ServerView& serverView);
    void ExtractListen(Node& serverBlock, ServerView& serverView);
    void ExtractServer_name(const Node& serverBlock, ServerView& serverView);
    void ExtractCgi(const Node& httpBlock, RouteView& routeView);
    void ExtractCgi_extension(const Node& httpBlock, RouteView& routeView);
    void ExtractIndex(const Node& node, RouteView& routeView);
    void ExtractAutoindex(const Node& node, RouteView& routeView);
    void ExtractClientMaxBodySize(const Node& node, RouteView& routeView);
    void ExtractAllowed_methods(const Node& node, RouteView& routeView);
    void ExtractReturn(const Node& node, RouteView& routeView);
    void ExtractRoot(const Node& node, RouteView& routeView);
    void ExtractAlias(const Node& node, RouteView& routeView);
    void ExtractError_page(const Node& node, RouteView& routeView);
    void ExtractLocationPrefix(const Node& location, RouteView& routeView);
    void ExtractHttpData();
    void ExtractServerData(Node& http, const RouteView& routeView);
    void ExtractLocationData(const Node& server, RouteView& routeView, std::size_t i);
    void SetServerViewDefaults(ServerView& serverView);

    Lexer& lexer_;
    Parser& parser_;
    ValidatorIpPort& validatorIpPort_;
    bool error_;
    std::uint16_t defaultPort_;
    std::string defaultIp_;
    std::uint16_t defaultReturnCode_;
    std::vector<ServerView> servers_;
    std::map<std::string, std::map<std::string, RouteView*>> routeViewMap_;
    std::map<std::pair<std::string, std::uint16_t>, std::map<std::string, std::map<std::string, RouteView*>>> routeViewMapIp_;

    static constexpr std::string_view kRed_ = "\033[31m";
    static constexpr std::string_view kReset_ = "\033[0m";
};

// The following two functions can be removed after all branches are merged

constexpr RouteView::MethodMask operator|(RouteView::MethodMask a, RouteView::MethodMask b)
{
  return static_cast<RouteView::MethodMask>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

constexpr bool Has(RouteView::MethodMask set, RouteView::MethodMask bit)
{
  return (static_cast<unsigned>(set) & static_cast<unsigned>(bit)) != 0;
}

#endif
