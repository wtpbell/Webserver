/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/13 17:24:28 by jboon         #+#    #+#                 */
/*   Updated: 2025/12/02 15:59:34 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <memory>

#include "Logger.hpp"
#include "SharedFD.hpp"
#include "webserv.hpp"

int main(void)
{
  setupSignals();

  Logger::Log(LogLevel::LDEBUG, "{}", "Webserv goes brbrbrbr");
  Logger::Log(LogLevel::INFO, "{}", "Webserv goes brbrbrbr");
  Logger::Log(LogLevel::WARNING, "{}", "Webserv goes brbrbrbr");
  Logger::Log(LogLevel::ERROR, "{}", "Webserv goes brbrbrbr");
  Logger::Log(LogLevel::CRITICAL, "{}", "Webserv goes brbrbrbr");

  {
    SharedFD m1;
    SharedFD tmp;
    {
      SharedFD m = SharedFD::Open("src/main.cpp", O_RDONLY, 0);
      SharedFD s = SharedFD::Open("src/SharedFD.cpp", O_RDONLY, 0);

      m1 = m;

      std::cout << "m  " << m.GetFD() << " count: " << m.SharedCount() << std::endl;
      std::cout << "m1 " << static_cast<int>(m1) << " count: " << m1.SharedCount() << std::endl;
      std::cout << "s  " << s.GetFD() << " count: " << s.SharedCount() << std::endl;
    }
    std::cout << "m1 " << m1.GetFD() << " count: " << m1.SharedCount() << std::endl;
  }

  return (0);
}
