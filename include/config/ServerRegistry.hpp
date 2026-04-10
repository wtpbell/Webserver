/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerRegistry.hpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: jstuhrin <jstuhrin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/03/24 15:04:18 by jstuhrin      #+#    #+#                 */
/*   Updated: 2026/04/02 12:29:37 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERREGISTRY_HPP
#define SERVERREGISTRY_HPP

#include <cstdint>
#include <map>
#include <string>
#include <type_traits>
#include <vector>

#include "RouteView.hpp"
#include "ServerView.hpp"

class ServerRegistry
{
  public:
    ServerRegistry(std::vector<ServerView> serverViews,
                   std::map<ServerView::IpPort, std::vector<ServerView*>> serverViewMap,
                   std::map<ServerView::IpPort, std::map<std::string, std::map<std::string, RouteView*>>> RouteViewMap,
                   std::map<ServerView::IpPort, std::map<std::string, RouteView*>> defaultServerRouteViewMap);
    ServerRegistry(const ServerRegistry& other) = delete;
    ServerRegistry(ServerRegistry&& other) = default;
    ServerRegistry& operator=(const ServerRegistry& other) = delete;
    ServerRegistry& operator=(ServerRegistry&& other) = default;
    ~ServerRegistry() = default;

    std::size_t GetServerViewCount() const;
    std::size_t GetServerCount() const;
    const ServerView& GetServerView(std::size_t i) const;
    const std::map<ServerView::IpPort, std::vector<ServerView*>>& GetServerViewMap() const;
    const RouteView* GetRouteView(const std::string& ip, const std::string& port, const std::string& hostName,
                                  const std::string& targetPath) const;

  private:
    std::size_t GetLenMatch(const std::string& locationPrefix, const std::string& locationPrefixRouteView) const;
    const RouteView* GetDefaultServerRouteView(const std::string& ip, const std::string& port,
                                               const std::string& targetPath) const;
    std::vector<ServerView> serverViews_;
    std::map<ServerView::IpPort, std::vector<ServerView*>> serverViewMap_;
    std::map<ServerView::IpPort, std::map<std::string, std::map<std::string, RouteView*>>> routeViewMap_;
    std::map<ServerView::IpPort, std::map<std::string, RouteView*>> defaultServerRouteViewMap_;

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

static_assert(std::is_nothrow_move_constructible_v<ServerRegistry>);

#endif
