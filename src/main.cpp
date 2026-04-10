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

#include "EpollManager.hpp"
#include "Logger.hpp"
#include "Server.hpp"
#include "config/ServerRegistry.hpp"
#include "config/ServerView.hpp"
#include "exception/FileDescriptorException.hpp"
#include "io/Socket.hpp"
#include "webserv.hpp"

static void ConstructServers(std::vector<Server>& servers, const ServerRegistry& serverRegistry)
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
        servers.emplace_back(serverData.first, Socket::Type::kIPv6, serverRegistry);
      }
      else
      {
        servers.emplace_back(serverData.first, Socket::Type::kIPv4, serverRegistry);
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
    std::cerr << "Please provide path to config file. Valid input: `./webserv file.conf [--option]`"
              << "\n";
    return EXIT_FAILURE;
  }

  std::optional<ServerRegistry> serverRegistry = LoadConfigs(argc, argv);
  if (serverRegistry == std::nullopt)
  {
    return (EXIT_FAILURE);
  }

  Logger::Log(LogLevel::INFO, "Server registry successfully constructed!");
  try
  {
    Logger::Log(LogLevel::INFO, "Setting up signals...");
    setupSignals();

    Logger::Log(LogLevel::INFO, "Constructing the servers...");
    std::vector<Server> servers;
    ConstructServers(servers, *serverRegistry);

    Logger::Log(LogLevel::INFO, "Start listing on the sockets and adding them to the poll manager...");
    EpollManager manager;
    for (auto& server : servers)
      server.RegisterFD(manager);

    Logger::Log(LogLevel::INFO, "(⌒ω⌒)ﾉ Webserv is now running!");
    manager.EventLoop();
  }
  catch (const std::exception& e)
  {
    Logger::Log(LogLevel::CRITICAL, "ヽ(°〇°)ﾉ Webserv crashed: {}!", e.what());
    return (1);
  }
  Logger::Log(LogLevel::INFO, "(´ ▽ ` )ﾉ Goodbye!");
  return (EXIT_SUCCESS);
}
