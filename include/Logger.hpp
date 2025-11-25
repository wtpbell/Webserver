/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Logger.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/14 10:16:48 by jboon         #+#    #+#                 */
/*   Updated: 2025/11/20 15:24:14 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <iostream>
#include <string>

#define LOG_BOLD "\033[1m"
#define LOG_CLEAR "\033[0m"
#define LOG_DEBUG "\033[38;5;139m"
#define LOG_INFO "\033[38;5;114m"
#define LOG_WARNING "\033[38;5;221m"
#define LOG_ERROR "\033[38;5;196m"
#define LOG_CRITICAL "\033[38;5;203m"

enum class LogLevel
{
  NONE = 0x0,
  LDEBUG = 0x1,
  INFO = 0x2,
  WARNING = 0x4,
  ERROR = 0x8,
  CRITICAL = 0x10,
  STDOUT = (INFO | WARNING),
  STDERR = (LDEBUG | ERROR | CRITICAL)
};

class Logger
{
  public:
    template <typename... Args>
    static void Log(LogLevel level, std::string_view format, Args&&... args);
    static std::string_view LevelToString(LogLevel level);

    Logger(void) = delete;

  private:
    static std::ostream cout_;
    static std::ostream cerr_;

    static char* GetCurrentTime(char* stime, std::size_t n);
    static std::ostream& GetOutputStream(LogLevel level);
    static void LogFormat(std::ostream& out, std::string_view format);
    template <typename T, typename... Args>
    static void LogFormat(std::ostream& out, std::string_view format, T&& arg, Args&&... args);
};

LogLevel operator|(LogLevel lhs, LogLevel rhs);
LogLevel operator&(LogLevel lhs, LogLevel rhs);

#include "Logger.tpp"

#endif
