/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/13 17:24:28 by jboon         #+#    #+#                 */
/*   Updated: 2025/12/12 12:15:25 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <cstdlib>

#include "Logger.hpp"
#include "Server.hpp"
#include "webserv.hpp"

int main(int argc, char* argv[])
{
  if (LoadConfigs(argc, argv) != EXIT_SUCCESS && false)
    return (EXIT_FAILURE);

  try
  {
    setupSignals();

    EpollManager manager;
    Server server{"8080"};

    server.ListenAndRegister(manager, 5);

    Logger::Log(LogLevel::INFO, "(⌒ω⌒)ﾉ Webserv running!");
    manager.EventLoop();
  }
  catch (const std::exception& e)
  {
    Logger::Log(LogLevel::CRITICAL, "ヽ(°〇°)ﾉ Webserv crashed {}!", e.what());
    return (1);
  }
  Logger::Log(LogLevel::INFO, "(´ ▽ ` )ﾉ Goodbye!");
  return (EXIT_SUCCESS);
}
