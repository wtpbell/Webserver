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

ServerRegistry::ServerRegistry(std::vector<ServerView> servers,
  std::map<ServerView::IpPort, std::map<std::string, std::map<std::string, RouteView*>>> RouteViewMap)
  : serverViews_(std::move(servers))
  , RouteViewMap_(std::move(RouteViewMap))
{}

//////////////////// PUBLIC ////////////////////

std::size_t ServerRegistry::GetServerViewCount() const
{
  return serverViews_.size();
}

const ServerView& ServerRegistry::GetServerView(std::size_t i) const
{
  assert(i < serverViews_.size());
  return serverViews_[i];
}

const RouteView* ServerRegistry::GetRouteView(const std::string& ip, const std::string& port, const std::string& hostName, const std::string& targetPath) const
{
  auto ipPortIt = RouteViewMap_.find(ServerView::IpPort{ip, port});
  if (ipPortIt == RouteViewMap_.end())
  {
    return nullptr;
  }
  auto hostIt = ipPortIt->second.find(hostName);
  if (hostIt == ipPortIt->second.end())
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