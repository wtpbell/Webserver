/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionManager.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bewong <bewong@student.codam.nl>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 11:41:07 by bewong            #+#    #+#             */
/*   Updated: 2026/02/25 11:41:07 by bewong           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "http/SessionManager.hpp"

#include <ctime>
#include <random>
#include <string>
#include <string_view>
#include <unordered_map>

#include "http/HTTPRequest.hpp"
#include "http/HTTPResponse.hpp"
#include "http/HTTPValidator.hpp"

void SessionManager::UseOrCreateSession(HTTPRequest& req, HTTPResponse& res)
{
  UseOrCreateSessionAt(req, res, std::time(nullptr));
}

// if the client keeps sending the same session id, it will be refreshed
void SessionManager::UseOrCreateSessionAt(HTTPRequest& req, HTTPResponse& res, std::time_t now)
{
  CleanupExpired(now);

  std::string_view sid = req.GetCookieOr("session_id", "");

  if (HTTP::validate::IsValidSid(sid))
  {
    std::string sid_str(sid);
    auto it = sessions_.find(sid_str);
    if (it != sessions_.end())
    {
      it->second = now;
      return;
    }
  }

  // if the session time out and got erased, client send the same old session_id again, generate a new one
  std::string newSid;

  do
  {
    newSid = NewSidHex(16);
  } while (sessions_.find(newSid) != sessions_.end());

  sessions_[newSid] = now;
  res.SetHeader("Set-Cookie", BuildSetCookie(newSid));
}

// https://security.stackexchange.com/questions/24850/choosing-a-session-id-algorithm-for-a-client-server-relationship
// https://stackoverflow.com/questions/39288595/why-not-just-use-stdrandom-device
// thread_local ensures each thread has its own generator, no reseeding every call
std::string SessionManager::NewSidHex(std::size_t bytes)
{
  static thread_local std::mt19937_64 gen(std::random_device{}());
  static const char hex[] = "0123456789abcdef";
  std::uniform_int_distribution<int> dist(0, 255);

  std::string out;
  out.resize(bytes * 2);

  for (std::size_t i = 0; i < bytes; ++i)
  {
    unsigned char c = static_cast<unsigned char>(dist(gen));
    out[i * 2] = hex[c >> 4];
    out[i * 2 + 1] = hex[c & 0x0F];
  }

  return out;
}

std::string SessionManager::BuildSetCookie(std::string_view sid)
{
  return "session_id=" + std::string(sid) + "; Path=/; HttpOnly; SameSite=Lax";
}

// session keep arriving within kTTL, the session never expire
void SessionManager::CleanupExpired(std::time_t now)
{
  for (auto it = sessions_.begin(); it != sessions_.end();)
  {
    if (now - it->second > HTTP::kTTL)
      it = sessions_.erase(it);
    else
      ++it;
  }
}
