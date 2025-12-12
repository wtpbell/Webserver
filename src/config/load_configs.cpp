/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   load_configs.cpp                                    :+:    :+:           */
/*                                                      +:+                   */
/*   By: jstuhrin <jstuhrin@student.codam.nl>          +#+                    */
/*                                                    +#+                     */
/*   Created: 2025/12/02 10:47:44 by jstuhrin       #+#    #+#                */
/*   Updated: 2025/12/02 10:47:48 by jstuhrin       ########   odam.nl        */
/*                                                                            */
/* ************************************************************************** */

#include <fstream>
#include <iostream>
#include <sstream>

#include "config/Lexer.hpp"
#include "config/Parser.hpp"
#include "webserv.hpp"

static std::string readFile(const std::string& filename)
{
  std::ifstream file(filename);
  if (!file)
  {
    throw std::runtime_error("Error: cannot open file: " + filename);
  }
  std::ostringstream stream;
  stream << file.rdbuf();
  std::string buffer = stream.str();
  if (buffer.empty())
  {
    throw std::runtime_error("Error: config file empty");
  }
  return (buffer);
}

int LoadConfigs(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cerr << "Please provide path to config file. Valid input: ./webserv <config_file>" << std::endl;
    return (EXIT_FAILURE);
  }
  if (argc > 2)
  {
    std::cerr << "Please provide exactly one paramater. Valid input: ./webserv <config_file>" << std::endl;
    return (EXIT_FAILURE);
  }

  Lexer lexer(readFile(argv[1]));
  lexer.Lex();
  // lexer.printTokenList();
  Parser parser(lexer);
  parser.Parse();
  parser.PrintDetailedAST();
  // lexer.printTokenList();
  lexer.PrintConfigsDebug();

  return (EXIT_SUCCESS);
}
