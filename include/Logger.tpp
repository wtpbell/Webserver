/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Logger.tpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/14 10:43:47 by jboon         #+#    #+#                 */
/*   Updated: 2025/11/28 10:44:06 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <ostream>
#include <sstream>
#include <utility>

template <typename T, typename... Args>
void Logger::LogFormat(std::ostream& out, std::string_view format, T&& arg, Args&&... args)
{
  std::size_t pos = format.find("{}");
  if (pos == std::string::npos)
    return (LogFormat(out, format));

  out << format.substr(0, pos) << std::forward<T>(arg);
  LogFormat(out, format.substr(pos + 2), std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::Log(LogLevel level, std::string_view format, Args&&... args)
{
  std::stringstream ss;
  char stime[32];

  if (IsFiltered(level))
    return;

  LogFormat(ss, "[{}] {}: ", GetTime(stime, 32), LevelToString(level));
  if (level == LogLevel::LDEBUG)
    LogFormat(ss, LOG_DEBUG "{} ({}): " LOG_CLEAR, __FILE__, __LINE__);  // TODO: HAHA!
  LogFormat(ss, format, std::forward<Args>(args)...);
  ss << "\n";
  GetOutputStream(level) << ss.str() << std::flush;
}
