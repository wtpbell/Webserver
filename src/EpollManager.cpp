/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   EpollManager.cpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/06 14:03:27 by bewong        #+#    #+#                 */
/*   Updated: 2026/02/06 17:35:03 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "EpollManager.hpp"

#include <sys/epoll.h>

#include <cassert>
#include <csignal>

#include "webserv.hpp"

EpollManager::EpollManager() : SharedFD() {}

EpollManager::Result EpollManager::Init()
{
  if (fd_ != -1)
    return Result::kInvalidState;

  int fd = epoll_create1(O_CLOEXEC);
  if (fd == -1)
    return Result::kSyscallError;

  Initialize(fd);
  return Result::kOk;
}

/// @brief Add the fd (or modify if already present) to the poll
EpollManager::Result EpollManager::AddFd(int fd, uint32_t events, EventCallback cb)
{
  struct epoll_event ev{};
  ev.data.fd = fd;
  ev.events = events;

  if (epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &ev) < 0)
  {
    if (errno == EEXIST)
      return ModifyFd(fd, events, cb);
    return Result::kSyscallError;
  }

  callbacks_[fd] = cb;
  return Result::kOk;
}

EpollManager::Result EpollManager::ModifyFd(int fd, uint32_t events)
{
  struct epoll_event ev{};
  ev.data.fd = fd;
  ev.events = events;

  if (epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &ev) < 0)
  {
    if (errno == ENOENT)  // fd not registered
      return Result::kNotFound;
    return Result::kSyscallError;
  }
  return Result::kOk;
}

EpollManager::Result EpollManager::ModifyFd(int fd, uint32_t events, EventCallback cb)
{
  Result result = ModifyFd(fd, events);
  if (result != Result::kOk)
    return result;

  callbacks_[fd] = cb;
  return Result::kOk;
}

EpollManager::Result EpollManager::RemoveFd(int fd)
{
  if (epoll_ctl(fd_, EPOLL_CTL_DEL, fd, nullptr) < 0)
  {
    if (errno == ENOENT)
      return Result::kNotFound;
    return Result::kSyscallError;
  }

  callbacks_.erase(fd);
  return Result::kOk;
}

EpollManager::Result EpollManager::EventLoop()
{
  if (fd_ == -1)
    return Result::kInvalidState;

  constexpr int MAX_EVENTS = 64;
  struct epoll_event events[MAX_EVENTS];

  sigset_t waitMask;
  if (sigemptyset(&waitMask) < 0)
    return Result::kSyscallError;

  while (!g_shutdown.load())
  {
    int n = epoll_pwait(fd_, events, MAX_EVENTS, -1, &waitMask);
    if (n == -1)
    {
      if (errno == EINTR)
        continue;  // EINTR from signal, loop again to check shutdown
      return Result::kSyscallError;
    }
    for (int i = 0; i < n; i++)
    {
      int fd = events[i].data.fd;
      callbacks_.find(fd)->second(*this, events[i]);
    }
  }
  return Result::kOk;
}

const char* EpollManager::ToString(Result result)
{
  switch (result)
  {
    case Result::kOk:
      return "Ok";
    case Result::kNotFound:
      return "NotFound";
    case Result::kInvalidState:
      return "InvalidState";
    case Result::kSyscallError:
      return "SyscallError";
  }
  assert(false && "invalid result enum in EpollManager::ToString");
  __builtin_unreachable();
}
