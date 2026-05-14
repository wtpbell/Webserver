/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   loadConfigs.cpp                                     :+:    :+:           */
/*                                                      +:+                   */
/*   By: jstuhrin <jstuhrin@student.codam.nl>          +#+                    */
/*                                                    +#+                     */
/*   Created: 2025/12/02 10:47:44 by jstuhrin       #+#    #+#                */
/*   Updated: 2025/12/02 10:47:48 by jstuhrin      ########   codam.nl        */
/*                                                                            */
/* ************************************************************************** */

#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <cstdint>

#include "config/Builder.hpp"
#include "config/Lexer.hpp"
#include "config/Parser.hpp"
#include "config/ServerRegistry.hpp"
#include "config/Validator.hpp"
#include "config/ValidatorIpPort.hpp"

namespace
{
  struct Options
  {
      bool printTokenList = false;
      bool printAST = false;
      bool printConfigsDebug = false;
  };

  std::optional<std::string> ReadFile(const std::string& filename)
  {
    std::error_code ec{};
    std::uintmax_t size = std::filesystem::file_size(filename, ec);
    if (ec)
    {
      std::cerr << "ERROR: " << ec.message() << "\n";
      return std::nullopt;
    }
    if (size > 32 * 1024 * 1024)
    {
      std::cerr << "Error: config file exceeds 32 MiB\n";
      return std::nullopt;
    }
    std::ifstream file(filename);
    if (!file)
    {
      std::cerr << "Error: cannot open file: " << filename << "\n";
      return std::nullopt;
    }
    std::ostringstream stream;
    stream << file.rdbuf();
    return stream.str();
  }

  bool ParseOptions(const int argc, char* argv[], Options& options)
  {
    bool valid = true;
    for (int i = 2; i < argc; ++i)
    {
      std::string flag{argv[i]};
      if (flag == "--print-tokenlist")
      {
        options.printTokenList = true;
      }
      else if (flag == "--print-ast")
      {
        options.printAST = true;
      }
      else if (flag == "--config-debug")
      {
        options.printConfigsDebug = true;
      }
      else
      {
        std::cerr << "Error: unknown parameter: " << flag << "\n";
        valid = false;
      }
    }
    return valid;
  }

  void Print(const Lexer& lexer, const Parser& parser, Options& options)
  {
    lexer.PrintErrorMessages();
    if (options.printTokenList)
    {
      lexer.PrintTokenList();
    }
    if (options.printAST)
    {
      parser.PrintDetailedAST();
    }
    if (options.printConfigsDebug)
    {
      lexer.PrintConfigsDebug();
    }
  }

  std::optional<ServerRegistry> ExtractConfigs(std::string buffer, Options& options)
  {
    Lexer lexer(std::move(buffer));
    Parser parser(lexer);
    ValidatorIpPort validatorIpPort;
    Validator validator(lexer, parser, validatorIpPort);
    if (lexer.GetError() || parser.GetError() || validator.GetError())
    {
      Print(lexer, parser, options);
      return std::nullopt;
    }
    Builder builder(lexer, parser, validatorIpPort);
    if (builder.GetError())
    {
      Print(lexer, parser, options);
      return std::nullopt;
    }
    Print(lexer, parser, options);
    return builder.BuildServerRegistry();
  }
}  // namespace

std::optional<ServerRegistry> LoadConfigs(const int argc, char* argv[])
{
  Options options;
  if (!ParseOptions(argc, argv, options))
  {
    return std::nullopt;
  }
  std::optional<std::string> buffer = ReadFile(argv[1]);
  if (!buffer.has_value())
  {
    return std::nullopt;
  }
  return ExtractConfigs(std::move(buffer).value(), options);
}
