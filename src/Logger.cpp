/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Logger.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/14 11:15:07 by jboon         #+#    #+#                 */
/*   Updated: 2026/01/08 19:58:07 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

#include <chrono>
#include <cstring>
#include <iostream>

std::ostream Logger::cout_{std::cout.rdbuf()};
std::ostream Logger::cerr_{std::cerr.rdbuf()};
LogLevel Logger::filter_{LogLevel::ALL};

char* Logger::GetTime(char* stime, std::size_t n, TimeFormat fmt)
{
  using sclock = std::chrono::system_clock;

  auto now = sclock::now();
  const std::time_t time = sclock::to_time_t(now);
  std::tm tm{};
  const char* format = nullptr;

  if (fmt == TimeFormat::HTTP_GMT)
  {
    gmtime_r(&time, &tm);
    format = "%a, %d %b %Y %H:%M:%S GMT";
  }
  else
  {
    localtime_r(&time, &tm);
    format = "%Y-%m-%d %H:%M:%S";  // logger
  }
  if (std::strftime(stime, n, format, &tm) > 0)
    return stime;
  GetOutputStream(LogLevel::CRITICAL) << LevelToString(LogLevel::CRITICAL)
                                      << "strftime exceeded the maximum size of `stime`!" << std::endl;

  return std::strncpy(stime, "<undetermined>", n);
}

std::ostream& Logger::GetOutputStream(LogLevel level)
{
  if ((level & LogLevel::STDERR) != LogLevel::NONE)
    return (cerr_);
  return (cout_);
}

void Logger::LogFormat(std::ostream& out, std::string_view format)
{
  out << format;
}

std::string_view Logger::LevelToString(LogLevel level)
{
  switch (level)
  {
    case LogLevel::LDEBUG:
      return LOG_DEBUG LOG_BOLD "DEBUG   " LOG_CLEAR;
    case LogLevel::INFO:
      return LOG_INFO LOG_BOLD "INFO    " LOG_CLEAR;
    case LogLevel::WARNING:
      return LOG_WARNING LOG_BOLD "WARNING " LOG_CLEAR;
    case LogLevel::ERROR:
      return LOG_ERROR LOG_BOLD "ERROR   " LOG_CLEAR;
    case LogLevel::CRITICAL:
      return LOG_CRITICAL LOG_BOLD "CRITICAL" LOG_CLEAR;
    default:
      break;
  }
  return "UNKNOWN";
}

void Logger::SetLogFilter(LogLevel filter)
{
  filter_ = filter;
}

bool Logger::IsFiltered(LogLevel level)
{
  return ((level & filter_) == LogLevel::NONE || level == LogLevel::STDERR || level == LogLevel::STDOUT ||
          level == LogLevel::ALL);
}

LogLevel operator|(LogLevel lhs, LogLevel rhs)
{
  return (static_cast<LogLevel>(static_cast<int>(lhs) | static_cast<int>(rhs)));
}

LogLevel operator&(LogLevel lhs, LogLevel rhs)
{
  return (static_cast<LogLevel>(static_cast<int>(lhs) & static_cast<int>(rhs)));
}
