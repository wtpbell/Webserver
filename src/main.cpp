/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/13 17:24:28 by jboon         #+#    #+#                 */
/*   Updated: 2025/11/17 18:14:15 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>

#include "Logger.hpp"

int main(void)
{
  // std::cout << "Webserv goes brbrbrbr" << std::endl;
  Logger::Log(LogLevel::DEBUG, "{}", "Webserv goes brbrbrbr");
  Logger::Log(LogLevel::INFO, "{}", "Webserv goes brbrbrbr");
  Logger::Log(LogLevel::WARNING, "{}", "Webserv goes brbrbrbr");
  Logger::Log(LogLevel::ERROR, "{}", "Webserv goes brbrbrbr");
  Logger::Log(LogLevel::CRITICAL, "{}", "Webserv goes brbrbrbr");
  return (0);
}
