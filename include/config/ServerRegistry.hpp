/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   ServerRegistry.hpp                                  :+:    :+:           */
/*                                                      +:+                   */
/*   By: jstuhrin <jstuhrin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/03/24 15:04:18 by jstuhrin       #+#    #+#                */
/*   Updated: 2026/03/24 15:04:23 by jstuhrin       ########   codam.nl       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERREGISTRY_HPP
#define SERVERREGISTRY_HPP

#include <vector>
#include <map>
#include <cstdint>
#include <string>
#include <type_traits>

#include "RouteView.hpp"
#include "ServerView.hpp"

class ServerRegistry
{
  public:
    ServerRegistry(std::vector<ServerView> servers,
      std::map<std::pair<std::string, std::uint16_t>, std::map<std::string, std::map<std::string, RouteView*>>> RouteViewMap);
    ServerRegistry(const ServerRegistry& other) = delete;
    ServerRegistry(ServerRegistry&& other) = default;
    ServerRegistry& operator=(const ServerRegistry& other) = delete;
    ServerRegistry& operator=(ServerRegistry&& other) = delete;
    ~ServerRegistry() = default;

    std::size_t GetServerCount() const;
    const ServerView& GetServerView(std::size_t i) const;
    const RouteView* GetRouteView(const std::string& ip, const std::uint16_t port, const std::string& hostName, const std::string& targetPath) const;

  private:
    std::size_t GetLenMatch(const std::string& locationPrefix, const std::string& locationPrefixRouteView) const;

    std::vector<ServerView> servers_;
    std::map<std::pair<std::string, std::uint16_t>, std::map<std::string, std::map<std::string, RouteView*>>> RouteViewMap_;

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

static_assert(std::is_nothrow_move_constructible_v<ServerRegistry>);

#endif