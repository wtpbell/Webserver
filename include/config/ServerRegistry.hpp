/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerRegistry.hpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: jstuhrin <jstuhrin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/03/24 15:04:18 by jstuhrin      #+#    #+#                 */
/*   Updated: 2026/04/24 15:52:17 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERREGISTRY_HPP
#define SERVERREGISTRY_HPP

#include <vector>
#include <map>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
#include <cassert>

#include "RouteView.hpp"
#include "ServerView.hpp"

class ServerRegistry
{
  public:
    struct SizeComparator
    {
      bool operator()(const std::string_view a, const std::string_view b) const
      {
        if (a.size() != b.size())
        {
          return a.size() > b.size();
        }
        return a > b;
      }
    };

    ServerRegistry(std::vector<ServerView> serverViews,
                   std::map<ServerView::IpPort, std::vector<ServerView*>> serverViewMap,
                   std::map<ServerView::IpPort, std::map<std::string_view, std::map<std::string_view, RouteView*, SizeComparator>>> RouteViewMap,
                   std::map<ServerView::IpPort, std::map<std::string_view, RouteView*, SizeComparator>> defaultServerRouteViewMap);
    ServerRegistry(const ServerRegistry& other) = delete;
    ServerRegistry(ServerRegistry&& other) = default;
    ServerRegistry& operator=(const ServerRegistry& other) = delete;
    ServerRegistry& operator=(ServerRegistry&& other) = delete;
    ~ServerRegistry() = default;

    std::size_t GetServerCount() const;
    const std::map<ServerView::IpPort, std::vector<ServerView*>>& GetServerViewMap() const;
    const RouteView* GetRouteView(const std::string& ip, const std::string& port, const std::string_view hostName,
                                  const std::string_view targetPath) const;

  private:
    bool IsMatch(const std::string_view locationPrefix, const std::string_view locationPrefixRouteView) const;
    const RouteView* GetDefaultServerRouteView(const std::string& ip, const std::string& port,
                                               const std::string_view targetPath) const;

    std::vector<ServerView> serverViews_;
    std::map<ServerView::IpPort, std::vector<ServerView*>> serverViewMap_;
    std::map<ServerView::IpPort, std::map<std::string_view, std::map<std::string_view, RouteView*, SizeComparator>>> routeViewMap_;
    std::map<ServerView::IpPort, std::map<std::string_view, RouteView*, SizeComparator>> defaultServerRouteViewMap_;

#ifdef UNIT_TEST
  public:
    std::size_t GetServerViewCount() const
    {
      return serverViews_.size();
    }
    const ServerView& GetServerView(std::size_t i) const
    {
      assert(i < serverViews_.size());
      return serverViews_[i];
    }
    const ServerView* GetServersData() const
    {
      return serverViews_.data();
    }
    const std::map<std::string_view, std::map<std::string_view, RouteView*, SizeComparator>>* GetAddressValue(const ServerView::IpPort& key) const
    {
      return &routeViewMap_.at(key);
    }
#endif
};

static_assert(std::is_nothrow_move_constructible_v<ServerRegistry>);

#endif
