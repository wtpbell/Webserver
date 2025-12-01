#include <iostream>
#include <sstream>

#include "Logger.hpp"
#include "catch_amalgamated.hpp"

using namespace Catch::Matchers;

struct LogTest
{
    LogLevel level;
    std::string level_msg;

    LogTest(LogLevel level_, std::string level_msg_) : level(level_), level_msg(level_msg_) {}
};

/// @brief Test to check if the Logger will log the correct LogLevel tag and the basic message.
TEST_CASE("LogLevels", "[Logger]")
{
  std::ostringstream os;
  std::ostream org_cout{Logger::OverrideCoutBuf(os)};
  std::ostream org_cerr{Logger::OverrideCerrBuf(os)};

  Logger::SetLogFilter(LogLevel::ALL);

  auto log_test = GENERATE(LogTest(LogLevel::LDEBUG, "DEBUG"), LogTest(LogLevel::INFO, "INFO"),
                           LogTest(LogLevel::WARNING, "WARNING"), LogTest(LogLevel::ERROR, "ERROR"),
                           LogTest(LogLevel::CRITICAL, "CRITICAL"));

  SECTION(log_test.level_msg)
  {
    Logger::Log(log_test.level, "this is log message!");
    CAPTURE(log_test.level, log_test.level_msg);
    REQUIRE_THAT(os.str(), ContainsSubstring(log_test.level_msg, Catch::CaseSensitive::Yes));
    REQUIRE_THAT(os.str(), ContainsSubstring("this is log message!", Catch::CaseSensitive::Yes));
  }

  os.str("");
  Logger::OverrideCoutBuf(org_cout);
  Logger::OverrideCerrBuf(org_cerr);
}

///@brief Test the Logger that it will output to the correct std output based on the LogLevel
TEST_CASE("std output or std error based on LogLevel", "[Logger]")
{
  std::ostringstream os_cout;
  std::ostringstream os_cerr;

  std::ostream org_cout{Logger::OverrideCoutBuf(os_cout)};
  std::ostream org_cerr{Logger::OverrideCerrBuf(os_cerr)};

  Logger::SetLogFilter(LogLevel::ALL);

  SECTION("Output to cout")
  {
    auto log_test = GENERATE(LogTest(LogLevel::INFO, "INFO"), LogTest(LogLevel::WARNING, "WARNING"));
    Logger::Log(log_test.level, "this is log message!");
    INFO("Should print to cout");
    CAPTURE(log_test.level, log_test.level_msg);
    CHECK(os_cerr.str().length() == 0);
    REQUIRE(os_cout.str().length() > 0);
  }

  SECTION("Output to cerr")
  {
    auto log_test = GENERATE(LogTest(LogLevel::LDEBUG, "DEBUG"), LogTest(LogLevel::ERROR, "ERROR"),
                             LogTest(LogLevel::CRITICAL, "CRITICAL"));
    Logger::Log(log_test.level, "this is log message!");
    INFO("Should print to cerr");
    CAPTURE(log_test.level, log_test.level_msg);
    CHECK(os_cout.str().length() == 0);
    REQUIRE(os_cerr.str().length() > 0);
  }

  os_cout.str("");
  os_cerr.str("");
  Logger::OverrideCoutBuf(org_cout);
  Logger::OverrideCerrBuf(org_cerr);
}

///@brief test case for testing the logger to print out the correct amount of arguments per level
TEST_CASE("Log arguments", "[Logger]")
{
  using is_case_sensitive = Catch::CaseSensitive;

  std::ostringstream os;
  std::ostream org_cout{Logger::OverrideCoutBuf(os)};
  std::ostream org_cerr{Logger::OverrideCerrBuf(os)};

  Logger::SetLogFilter(LogLevel::ALL);

  auto log_test = GENERATE(LogTest(LogLevel::LDEBUG, "DEBUG"), LogTest(LogLevel::INFO, "INFO"),
                           LogTest(LogLevel::WARNING, "WARNING"), LogTest(LogLevel::ERROR, "ERROR"),
                           LogTest(LogLevel::CRITICAL, "CRITICAL"));

  SECTION("Amount of arguments matches the amount of format specifiers")
  {
    Logger::Log(log_test.level, "Hello, {}!", "world");
    REQUIRE_THAT(os.str(), EndsWith("Hello, world!\n", is_case_sensitive::Yes));
    os.str("");

    Logger::Log(log_test.level, "{}", "test this amazing string!! woot woot!");
    REQUIRE_THAT(os.str(), EndsWith("test this amazing string!! woot woot!\n", is_case_sensitive::Yes));
    os.str("");

    Logger::Log(log_test.level, "{}{}", "Hello,", " world!");
    REQUIRE_THAT(os.str(), EndsWith("Hello, world!\n", is_case_sensitive::Yes));
    os.str("");

    Logger::Log(log_test.level, "{}     maybe,   what is this {} random{}{} {}", "test", 42, 4, 2, "number");
    REQUIRE_THAT(os.str(), EndsWith("test     maybe,   what is this 42 random42 number\n", is_case_sensitive::Yes));
    os.str("");
  }

  SECTION("Amount of arguments that doesn't match the amount of format specifiers")
  {
    Logger::Log(log_test.level, "Hello, {}!", "world", 42);
    REQUIRE_THAT(os.str(), EndsWith("Hello, world!\n", is_case_sensitive::Yes));
    os.str("");

    Logger::Log(log_test.level, "What {} does {} mean to {}?", "|");
    REQUIRE_THAT(os.str(), EndsWith("What | does {} mean to {}?\n", is_case_sensitive::Yes));
    os.str("");

    Logger::Log(log_test.level, "{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}", 'a', 'z', 42, "hello", ", ", "world");
    REQUIRE_THAT(os.str(), EndsWith("az42hello, world{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}\n", is_case_sensitive::Yes));
    os.str("");
  }

  SECTION("No arguments with one or more format specifiers")
  {
    Logger::Log(log_test.level, "{}");
    REQUIRE_THAT(os.str(), EndsWith("{}\n", is_case_sensitive::Yes));
    os.str("");

    Logger::Log(log_test.level, "some {} text {}{} with format{} specifiers{} in {}between");
    REQUIRE_THAT(os.str(),
                 EndsWith("some {} text {}{} with format{} specifiers{} in {}between\n", is_case_sensitive::Yes));
    os.str("");

    Logger::Log(log_test.level, "{}{}{}{}{}{}{}{}{}{}{}{}{}");
    REQUIRE_THAT(os.str(), EndsWith("{}{}{}{}{}{}{}{}{}{}{}{}{}\n", is_case_sensitive::Yes));
    os.str("");
  }

  os.str("");
  Logger::OverrideCoutBuf(org_cout);
  Logger::OverrideCerrBuf(org_cerr);
}

///@brief Testcase with unclosed format specifiers ({})
TEST_CASE("Incomplete format specifier", "[Logger]")
{
  std::ostringstream os;
  std::ostream org_cout{Logger::OverrideCoutBuf(os)};

  Logger::SetLogFilter(LogLevel::ALL);

  SECTION("Open or Closing specifier")
  {
    Logger::Log(LogLevel::INFO, "{", "My argument");
    REQUIRE_THAT(os.str(), EndsWith("{\n"));
    os.str("");

    Logger::Log(LogLevel::INFO, "{{", "My argument");
    REQUIRE_THAT(os.str(), EndsWith("{{\n"));
    os.str("");

    Logger::Log(LogLevel::INFO, "{{{{{{}", "My argument");
    REQUIRE_THAT(os.str(), EndsWith("{{{{{My argument\n"));
    os.str("");

    Logger::Log(LogLevel::INFO, "}", "My argument");
    REQUIRE_THAT(os.str(), EndsWith("}\n"));
    os.str("");

    Logger::Log(LogLevel::INFO, "{}}}}}}}}}}}", "My argument");
    REQUIRE_THAT(os.str(), EndsWith("My argument}}}}}}}}}}\n"));
    os.str("");
  }

  os.str("");
  Logger::OverrideCoutBuf(org_cout);
}

TEST_CASE("filter log messages", "[Logger]")
{
  std::ostringstream os;
  std::ostream org_cout{Logger::OverrideCoutBuf(os)};
  std::ostream org_cerr{Logger::OverrideCerrBuf(os)};

  auto log_all_except = [](LogLevel exclude)
  {
    LogLevel levels[] = {LogLevel::LDEBUG, LogLevel::INFO, LogLevel::WARNING, LogLevel::ERROR, LogLevel::CRITICAL};
    for (LogLevel level : levels)
    {
      if ((level & exclude) != LogLevel::NONE)
        continue;
      Logger::Log(level, "This a log message!");
    }
  };

  SECTION("Filter set to ALL")
  {
    auto level = GENERATE(LogLevel::LDEBUG, LogLevel::INFO, LogLevel::WARNING, LogLevel::ERROR, LogLevel::CRITICAL);
    Logger::SetLogFilter(LogLevel::ALL);
    Logger::Log(level, "This is a log message!");
    REQUIRE(os.str().length() > 0);
  }

  SECTION("Filter set to NONE")
  {
    Logger::SetLogFilter(LogLevel::NONE);
    log_all_except(LogLevel::NONE);
    REQUIRE(os.str().length() == 0);
  }

  SECTION("Filter set to STDOUT")
  {
    Logger::SetLogFilter(LogLevel::STDOUT);

    log_all_except(LogLevel::STDOUT);
    REQUIRE(os.str().length() == 0);

    log_all_except(LogLevel::STDERR);
    REQUIRE(os.str().length() > 0);
  }

  SECTION("Filter set to STDERR")
  {
    Logger::SetLogFilter(LogLevel::STDERR);

    log_all_except(LogLevel::STDERR);
    REQUIRE(os.str().length() == 0);

    log_all_except(LogLevel::STDOUT);
    REQUIRE(os.str().length() > 0);
  }

  SECTION("No messages from NONE, STDOUT, STDERR and ALL")
  {
    auto filter = GENERATE(LogLevel::NONE, LogLevel::LDEBUG, LogLevel::INFO, LogLevel::WARNING, LogLevel::ERROR,
                           LogLevel::CRITICAL, LogLevel::STDOUT, LogLevel::STDERR, LogLevel::ALL);

    Logger::SetLogFilter(filter);
    Logger::Log(LogLevel::NONE, "This message should be filtered out!");
    Logger::Log(LogLevel::STDOUT, "This message should be filtered out!");
    Logger::Log(LogLevel::STDERR, "This message should be filtered out!");
    Logger::Log(LogLevel::ALL, "This message should be filtered out!");
    CAPTURE(filter);
    REQUIRE(os.str().length() == 0);
  }

  SECTION("Filter set to single printable log level")
  {
    auto level = GENERATE(LogLevel::LDEBUG, LogLevel::INFO, LogLevel::WARNING, LogLevel::ERROR, LogLevel::CRITICAL);
    Logger::SetLogFilter(level);

    log_all_except(level);
    REQUIRE(os.str().length() == 0);

    Logger::Log(level, "This message will display!");
    REQUIRE(os.str().length() > 0);
  }

  os.str("");
  Logger::SetLogFilter(LogLevel::ALL);
  Logger::OverrideCoutBuf(org_cout);
  Logger::OverrideCerrBuf(org_cerr);
}
