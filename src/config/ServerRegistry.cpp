/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerRegistry.cpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: jstuhrin <jstuhrin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/03/24 15:04:18 by jstuhrin      #+#    #+#                 */
/*   Updated: 2026/04/24 15:52:55 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "config/ServerRegistry.hpp"

#include <cassert>
#include <string>
#include <vector>

#include "config/RouteView.hpp"
#include "config/ServerView.hpp"

ServerRegistry::ServerRegistry(
    std::vector<ServerView> serverViews, std::map<ServerView::IpPort, std::vector<ServerView*>> serverViewMap,
    std::map<ServerView::IpPort, std::map<std::string_view, std::map<std::string_view, RouteView*, SizeComparator>>> RouteViewMap,
    std::map<ServerView::IpPort, std::map<std::string_view, RouteView*, SizeComparator>> defaultServerRouteViewMap)
    : serverViews_(std::move(serverViews)),
      serverViewMap_(std::move(serverViewMap)),
      routeViewMap_(std::move(RouteViewMap)),
      defaultServerRouteViewMap_(std::move(defaultServerRouteViewMap))
{
}

//////////////////// PUBLIC ////////////////////

std::size_t ServerRegistry::GetServerCount() const
{
  return serverViewMap_.size();
}

const std::map<ServerView::IpPort, std::vector<ServerView*>>& ServerRegistry::GetServerViewMap() const
{
  return serverViewMap_;
}

const RouteView* ServerRegistry::GetRouteView(const std::string& ip, const std::string& port,
                                              const std::string_view hostName, const std::string_view targetPath) const
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
  for (auto it = hostIt->second.begin(); it != hostIt->second.end(); ++it)
  {
    if(IsMatch(targetPath, it->first))
    {
      return it->second;
    }
  }
  return nullptr;
}

//////////////////// PRIVATE ////////////////////

bool ServerRegistry::IsMatch(const std::string_view targetPath, const std::string_view locationPrefix) const
{
  if (locationPrefix.size() > targetPath.size())
  {
    return false;
  }
  std::size_t len = 0;
  while (len < targetPath.size() && len < locationPrefix.size())
  {
    if (targetPath[len] != locationPrefix[len])
    {
      break;
    }
    ++len;
  }
  if (len < locationPrefix.size())
  {
    return false;
  }
  if (len == targetPath.size() || targetPath[len] == '/' ||
      (locationPrefix.back() == '/' && targetPath[locationPrefix.size() - 1] == '/'))
  {
    return true;
  }
  return false;
}

const RouteView* ServerRegistry::GetDefaultServerRouteView(const std::string& ip, const std::string& port,
                                                           const std::string_view targetPath) const
{
  auto ipPortIt = defaultServerRouteViewMap_.find(ServerView::IpPort{ip, port});
  for (auto it = ipPortIt->second.begin(); it != ipPortIt->second.end(); ++it)
  {
    if(IsMatch(targetPath, it->first))
    {
      return it->second;
    }
  }
  return nullptr;
}
