/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/13 17:24:28 by jboon         #+#    #+#                 */
/*   Updated: 2026/04/01 20:54:10 by jboon         ########   odam.nl         */
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
#include "io/Socket.hpp"
#include "webserv.hpp"

static void ConstructServers(std::vector<Server>& servers, const ServerRegistry& serverRegistry)
{
  assert(serverRegistry.GetServerViewCount() > 0);

  servers.reserve(serverRegistry.GetServerViewCount());
  for (std::size_t i = 0; i < serverRegistry.GetServerViewCount(); ++i)
  {
    assert(serverRegistry.GetServerView(i).ipPortList.size() > 0);

    const ServerView& serverView = serverRegistry.GetServerView(i);
    for (const auto& ipPortPair : serverView.ipPortList)
    {
      assert(ipPortPair.ip.empty() == false && ipPortPair.port.empty() == false);
      if (ipPortPair.ip.find_first_of(':') != std::string::npos)
      {
        servers.emplace_back(ipPortPair, Socket::Type::kIPv6, serverRegistry);
      }
      else if (ipPortPair.ip.find_first_of('.') != std::string::npos)
      {
        servers.emplace_back(ipPortPair, Socket::Type::kIPv4, serverRegistry);
      }
      else
      {
        Logger::Log(LogLevel::WARNING, "Unknown configurations for constructing a server.");
      }
    }
  }
}

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cerr << "Please provide path to config file. Valid input: ./webserv file [--option]"
              << "\n";
    return EXIT_FAILURE;
  }

  std::optional<ServerRegistry> serverRegistry = LoadConfigs(argc, argv);
  if (serverRegistry == std::nullopt)
  {
    std::cerr << "Failed to construct the server registry! Exiting..."
              << "\n";
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

    Logger::Log(LogLevel::INFO, "Listening on the sockets and adding them to the poll manager...");
    EpollManager manager;
    for (auto& server : servers)
      server.ListenAndRegister(manager);

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
