/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/13 17:24:28 by jboon         #+#    #+#                 */
/*   Updated: 2026/04/02 16:03:52 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <cassert>
#include <cstdlib>
#include <iostream>

#include "config/ServerRegistry.hpp"
#include "config/ServerView.hpp"
#include "config/loadConfigs.hpp"
#include "core/EpollManager.hpp"
#include "core/Server.hpp"
#include "exception/FileDescriptorException.hpp"
#include "io/Socket.hpp"
#include "utils/Logger.hpp"
#include "utils/signal.hpp"

static void ConstructServers(std::vector<Server>& servers, const ServerRegistry& serverRegistry,
                             EpollManager& epollManager)
{
  assert(serverRegistry.GetServerCount() > 0);
  servers.reserve(serverRegistry.GetServerCount());
  for (const auto& serverData : serverRegistry.GetServerViewMap())
  {
    assert(serverData.first.ip.empty() == false);
    assert(serverData.first.port.empty() == false);
    assert(serverData.second.size() > 0);
    try
    {
      if (serverData.first.ip.find_first_of(':') != std::string::npos)
      {
        servers.emplace_back(serverData.first, Socket::Type::kIPv6, serverRegistry, epollManager);
      }
      else
      {
        servers.emplace_back(serverData.first, Socket::Type::kIPv4, serverRegistry, epollManager);
      }
    }
    catch (const FileDescriptorException& ex)
    {
      Logger::Log(LogLevel::ERROR, "(҂◡_◡) ᕤ Failure to construct the server <{}:{}>: {}", serverData.first.ip,
                  serverData.first.port, ex.what());
    }
  }
}

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cerr << "Please provide path to config file. Valid input: `./webserv file.conf [--option]`\n";
    return EXIT_FAILURE;
  }

  Logger::Log(LogLevel::INFO, "Loading in the configuration file...");
  std::optional<ServerRegistry> serverRegistry = LoadConfigs(argc, argv);
  if (serverRegistry == std::nullopt)
  {
    return EXIT_FAILURE;
  }
  Logger::Log(LogLevel::INFO, "Server registry successfully constructed!");

  Logger::Log(LogLevel::INFO, "Setting up signals...");
  setupSignals();

  EpollManager manager;
  if (manager.Init() != EpollManager::Result::kOk)
  {
    Logger::Log(LogLevel::CRITICAL, "ヽ(°〇°)ﾉ Failed to initialize epoll manager.");
    return EXIT_FAILURE;
  }

  Logger::Log(LogLevel::INFO, "Constructing the servers...");
  std::vector<Server> servers;
  ConstructServers(servers, *serverRegistry, manager);
  if (servers.empty())
  {
    Logger::Log(LogLevel::ERROR, "ヽ(°〇°)ﾉ No valid servers could be constructed.");
    return EXIT_FAILURE;
  }

  Logger::Log(LogLevel::INFO, "(⌒ω⌒)ﾉ Webserv is now running!");
  EpollManager::Result loopResult = manager.EventLoop();
  if (loopResult != EpollManager::Result::kOk)
  {
    Logger::Log(LogLevel::CRITICAL, "ヽ(°〇°)ﾉ Webserv event loop stopped: {}", EpollManager::ToString(loopResult));
    return EXIT_FAILURE;
  }

  Logger::Log(LogLevel::INFO, "(´ ▽ ` )ﾉ Goodbye!");
  return EXIT_SUCCESS;
}
