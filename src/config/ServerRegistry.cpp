/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   ServerRegistry.cpp                                  :+:    :+:           */
/*                                                      +:+                   */
/*   By: jstuhrin <jstuhrin@student.codam.nl>          +#+                    */
/*                                                    +#+                     */
/*   Created: 2026/03/24 15:04:18 by jstuhrin       #+#    #+#                */
/*   Updated: 2026/03/24 15:04:23 by jstuhrin       ########   codam.nl       */
/*                                                                            */
/* ************************************************************************** */

#include <vector>
#include <map>
#include <cstdint>
#include <string>
#include <cassert>

#include "config/RouteView.hpp"
#include "config/ServerView.hpp"
#include "config/ServerRegistry.hpp"

ServerRegistry::ServerRegistry(std::vector<ServerView> serverViews,
  std::map<ServerView::IpPort, std::vector<ServerView*>> serverViewMap,
  std::map<ServerView::IpPort, std::map<std::string, std::map<std::string, RouteView*>>> RouteViewMap,
  std::map<ServerView::IpPort, std::map<std::string, RouteView*>> defaultServerRouteViewMap)
  : serverViews_(std::move(serverViews))
  , serverViewMap_(std::move(serverViewMap))
  , routeViewMap_(std::move(RouteViewMap))
  , defaultServerRouteViewMap_(std::move(defaultServerRouteViewMap))
{}

//////////////////// PUBLIC ////////////////////

std::size_t ServerRegistry::GetServerViewCount() const
{
  return serverViews_.size();
}

std::size_t ServerRegistry::GetServerCount() const
{
  return serverViewMap_.size();
}

const ServerView& ServerRegistry::GetServerView(std::size_t i) const
{
  assert(i < serverViews_.size());
  return serverViews_[i];
}

const std::map<ServerView::IpPort, std::vector<ServerView*>>& ServerRegistry::GetServerViewMap() const
{
  return serverViewMap_;
}

const RouteView* ServerRegistry::GetRouteView(const std::string& ip, const std::string& port, const std::string& hostName, const std::string& targetPath) const
{
  auto ipPortIt = routeViewMap_.find(ServerView::IpPort{ip, port});
  if (ipPortIt == routeViewMap_.end())
  {
    return nullptr;
  }
  auto hostIt = ipPortIt->second.find(hostName);
  if (hostIt == ipPortIt->second.end())
  {
    return GetDefaultServerRouteView(ip, port, targetPath);
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

//////////////////// PRIVATE ////////////////////

std::size_t ServerRegistry::GetLenMatch(const std::string& targetPath, const std::string& locationPrefix) const
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

const RouteView* ServerRegistry::GetDefaultServerRouteView(const std::string& ip, const std::string& port, const std::string& targetPath) const
{
  auto ipPortIt = defaultServerRouteViewMap_.find(ServerView::IpPort{ip, port});
  std::size_t longestMatch = 0;
  RouteView* routeView = nullptr;
  for (auto it = ipPortIt->second.begin(); it != ipPortIt->second.end(); ++it)
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
