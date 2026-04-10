/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   SessionManager.hpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/04/02 09:57:12 by bewong        #+#    #+#                 */
/*   Updated: 2026/04/02 09:57:12 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SESSION_MANAGER_HPP
#define SESSION_MANAGER_HPP

#include <ctime>
#include <string>
#include <string_view>
#include <unordered_map>

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

class SessionManager
{
  public:
    SessionManager(void) = default;
    SessionManager(const SessionManager& other) = default;
    SessionManager(SessionManager&& other) noexcept = default;
    SessionManager& operator=(const SessionManager& other) = default;
    SessionManager& operator=(SessionManager&& other) noexcept = default;
    ~SessionManager(void) = default;

    void UseOrCreateSession(HTTPRequest& request, HTTPResponse& res);
    void UseOrCreateSessionAt(HTTPRequest& request, HTTPResponse& res, std::time_t now);

#ifdef UNIT_TEST

    std::size_t SessionCount() const
    {
      return sessions_.size();
    }
    bool HasSession(std::string_view sid) const
    {
      return sessions_.count(std::string(sid)) != 0;
    }
#endif

  private:
    void CleanupExpired(std::time_t now);
    std::string NewSidHex(std::size_t bytes = 16);
    std::string BuildSetCookie(std::string_view sid);

    std::unordered_map<std::string, std::time_t> sessions_;
};

#endif
