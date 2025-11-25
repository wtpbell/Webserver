/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/13 17:24:28 by jboon         #+#    #+#                 */
/*   Updated: 2025/11/21 18:36:52 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>

#include "Logger.hpp"
#include "webserv.hpp"

int main(void)
{
  setupSignals();

  Logger::Log(LogLevel::LDEBUG, "{}", "Webserv goes brbrbrbr");
  Logger::Log(LogLevel::INFO, "{}", "Webserv goes brbrbrbr");
  Logger::Log(LogLevel::WARNING, "{}", "Webserv goes brbrbrbr");
  Logger::Log(LogLevel::ERROR, "{}", "Webserv goes brbrbrbr");
  Logger::Log(LogLevel::CRITICAL, "{}", "Webserv goes brbrbrbr");
  return (0);
}
