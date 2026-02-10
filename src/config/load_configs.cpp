/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   load_configs.cpp                                    :+:    :+:           */
/*                                                      +:+                   */
/*   By: jstuhrin <jstuhrin@student.ccodam.nl>          +#+                    */
/*                                                    +#+                     */
/*   Created: 2025/12/02 10:47:44 by jstuhrin       #+#    #+#                */
/*   Updated: 2025/12/02 10:47:48 by jstuhrin       ########   codam.nl        */
/*                                                                            */
/* ************************************************************************** */

#include <fstream>
#include <iostream>
#include <sstream>

#include "config/Lexer.hpp"
#include "config/Parser.hpp"
#include "config/Validator.hpp"
#include "webserv.hpp"

static std::string readFile(const std::string& filename, bool* readFileError)
{
  std::ifstream file(filename);
  if (!file)
  {
    std::cerr << "Error: cannot open file: " << filename << "\n";
    *readFileError = true;
    return ("");
  }
  std::ostringstream stream;
  stream << file.rdbuf();
  std::string buffer = stream.str();
  return (buffer);
}

int LoadConfigs(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cerr << "Please provide path to config file. Valid input: ./webserv <config_file>" << std::endl;
    return (EXIT_FAILURE);
  }
  bool readFileError = false;
  std::string buffer = readFile(argv[1], &readFileError);
  if (readFileError)
  {
    return (EXIT_FAILURE);
  }

  Lexer lexer;
  Parser parser(lexer);
  Validator validator(lexer, parser);

  lexer.Lex(buffer);
  parser.Parse();
  validator.ValidateAst();
  lexer.PrintErrorMessages();

  //lexer.PrintTokenList();
  //lexer.PrintErrorIdxs();
  //parser.PrintDetailedAST();
  lexer.PrintConfigsDebug();

  if (lexer.GetError() || parser.GetError() || validator.GetError())
  {
    return (EXIT_FAILURE);
  }
  return (EXIT_SUCCESS);
}
