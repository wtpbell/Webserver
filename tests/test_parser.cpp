#include <iostream>
#include <sstream>
#include <string>

#include "catch_amalgamated.hpp"
#include "config/Lexer.hpp"
#include "config/Parser.hpp"

TEST_CASE("parser.Parse() parses correct tree without printing errors", "[parser]")
{
  std::string raw =
      "directive 1234param;events{directive param;}http{directive param;server{directive param;location "
      "param{directive param 1234param;}}}";

  Lexer lexer(raw);
  lexer.Lex();

  Parser parser(lexer);

  std::stringstream buffer;

  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());

  parser.Parse();
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();

  REQUIRE(output.empty());
}

TEST_CASE("parser.Parse() parses incorrect tree printing correct errors", "[parser]")
{
  std::string raw =
      "directive 1234param;;events param {directive param;}http{directive param;   http  {directive http "
      "param;location param{directive param 1234param;}}}}";

  Lexer lexer(raw);
  lexer.Lex();

  Parser parser(lexer);

  std::stringstream buffer;

  auto* oldBuf = std::cerr.rdbuf(buffer.rdbuf());

  parser.Parse();
  std::cerr.rdbuf(oldBuf);
  std::string output = buffer.str();

  std::string expected =
      "1:21: \033[31merror:\033[0m unexpected token: `\033[31m;\033[0m` expected: DIRECTIVE or BLOCKDIRECTIVE\n"
      "1:29: \033[31merror:\033[0m unexpected token: `\033[31mparam\033[0m` expected: `{`\n"
      "1:77: \033[31merror:\033[0m incorrect context: `\033[31mhttp\033[0m` not allowed in context http\n"
      "1:77: \033[31merror:\033[0m duplicate `http`: `\033[31mhttp\033[0m`\n"
      "1:94: \033[31merror:\033[0m unexpected token: `\033[31mhttp\033[0m` expected: PARAMETER\n"
      "1:105: \033[31merror:\033[0m incorrect context: `\033[31mlocation\033[0m` not allowed in context http\n"
      "1:149: \033[31merror:\033[0m unexpected token: `\033[31m}\033[0m` expected: DIRECTIVE or BLOCKDIRECTIVE\n";

  REQUIRE(output == expected);
}

TEST_CASE("parser.PrintDetailedAST() correctly prints tree for valid input", "[parser]")
{
  std::string raw =
      "directive 1234param;events{directive param;}http{directive param;server{directive param;location "
      "param{directive param 1234param;}}}";

  Lexer lexer(raw);
  lexer.Lex();

  Parser parser(lexer);

  std::stringstream buffer;

  auto* oldBuf = std::cout.rdbuf(buffer.rdbuf());

  parser.Parse();
  parser.PrintDetailedAST();
  std::cout.rdbuf(oldBuf);
  std::string output = buffer.str();

  std::string expected =
      "\n"
      "###############\n"
      "##### AST #####\n"
      "###############\n"
      "\n"
      "CONFIGS:\n"
      "\n"
      "    DIRECTIVE:   Name: directive   Context: main\n"
      "        PARAM:   1234param\n"
      "\n"
      "    BLOCKDIRECTIVE:   Name: events   Context: main\n"
      "\n"
      "        DIRECTIVE:   Name: directive   Context: events\n"
      "            PARAM:   param\n"
      "\n"
      "    BLOCKDIRECTIVE:   Name: http   Context: main\n"
      "\n"
      "        DIRECTIVE:   Name: directive   Context: http\n"
      "            PARAM:   param\n"
      "\n"
      "        BLOCKDIRECTIVE:   Name: server   Context: http\n"
      "\n"
      "            DIRECTIVE:   Name: directive   Context: server\n"
      "                PARAM:   param\n"
      "\n"
      "            BLOCKDIRECTIVE:   Name: location   Context: server\n"
      "                PARAM:   param\n"
      "\n"
      "                DIRECTIVE:   Name: directive   Context: location\n"
      "                    PARAM:   param\n"
      "                    PARAM:   1234param\n"
      "\n"
      "###################\n"
      "##### END AST #####\n"
      "###################\n"
      "\n";

  REQUIRE(output == expected);
}
