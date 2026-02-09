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

#include <csignal>
#include <system_error>

#include "exception/EPollManagerException.hpp"
#include "webserv.hpp"

EpollManager::EpollManager(void) : SharedFD()
{
  int fd = epoll_create1(O_CLOEXEC);
  if (fd == -1)
    throw EPollManagerException("epoll_create1", errno);

  Initialize(fd);
}

/// @brief Add the fd (or modify if already present) to the poll
void EpollManager::AddFd(int fd, uint32_t events, EventCallback cb)
{
  struct epoll_event ev{};

  ev.data.fd = fd;
  ev.events = events;
  if (epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &ev) < 0)
  {
    if (errno == EEXIST)
      ModifyFd(fd, events, cb);
    else
      throw EPollManagerException("AddFd", errno);
  }
  callbacks_[fd] = cb;
}

void EpollManager::ModifyFd(int fd, uint32_t events)
{
  struct epoll_event ev{};

  ev.data.fd = fd;
  ev.events = events;
  if (epoll_ctl(fd_, EPOLL_CTL_MOD, fd, &ev) < 0)
  {
    if (errno == ENOENT)  // fd not registered
      throw NotRegisteredInEPollException("ModifyFd", errno);
    else
      throw EPollManagerException("ModifyFd", errno);
  }
}

void EpollManager::ModifyFd(int fd, uint32_t events, EventCallback cb)
{
  ModifyFd(fd, events);
  callbacks_[fd] = std::move(cb);
}

void EpollManager::RemoveFd(int fd)
{
  if (epoll_ctl(fd_, EPOLL_CTL_DEL, fd, nullptr) < 0)
    throw EPollManagerException("RemoveFd", errno);
  callbacks_.erase(fd);
}

void EpollManager::EventLoop(void)
{
  constexpr int MAX_EVENTS = 64;
  struct epoll_event events[MAX_EVENTS];
  sigset_t waitMask;
  if (sigemptyset(&waitMask) < 0)
    throw std::system_error(errno, std::system_category(), "sigemptyset");
  while (!g_shutdown.load())
  {
    int n = epoll_pwait(fd_, events, MAX_EVENTS, -1, &waitMask);
    if (n == -1)
    {
      if (errno == EINTR)
        continue;  // EINTR from signal, loop again to check shutdown
      throw EPollManagerException("EventLoop", errno);
    }
    for (int i = 0; i < n; i++)
    {
      int fd = events[i].data.fd;
      callbacks_.find(fd)->second(*this, events[i]);
    }
  }
}
