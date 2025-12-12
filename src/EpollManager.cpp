/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   EpollManager.cpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/14 15:42:18 by bewong        #+#    #+#                 */
/*   Updated: 2025/12/11 23:10:53 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "EpollManager.hpp"

#include <sys/epoll.h>

#include <csignal>
#include <system_error>

#include "exception/EPollManagerException.hpp"

EpollManager::EpollManager(void)
{
  ep_fd_ = epoll_create1(O_CLOEXEC);  // will test it out later to see use O_CLOEXEC or 0
  if (ep_fd_ < 0)
    throw EPollManagerException("epoll_create1", errno);
}

EpollManager::~EpollManager(void)
{
  // Close all registered file descriptors
  for (const auto& [fd, callback] : callbacks_)
    close(fd);
  close(ep_fd_);
}

void EpollManager::AddFd(int fd, uint32_t events, EventCallback cb)
{
  struct epoll_event ev{};

  ev.data.fd = fd;
  ev.events = events;
  if (epoll_ctl(ep_fd_, EPOLL_CTL_ADD, fd, &ev) < 0)
  {
    if (errno == EEXIST)
      throw ExistInEPollException("epoll_ctl (ADD)", errno);
    else
      throw EPollManagerException("epoll_ctl (ADD)", errno);
  }
  callbacks_[fd] = cb;
}

void EpollManager::ModifyFd(int fd, uint32_t events)
{
  struct epoll_event ev{};

  ev.data.fd = fd;
  ev.events = events;
  if (epoll_ctl(ep_fd_, EPOLL_CTL_MOD, fd, &ev) < 0)
  {
    if (errno == ENOENT)  // fd not registered
      throw NotRegisteredInEPollException("epoll_ctl (MOD)", errno);
    else
      throw EPollManagerException("epoll_ctl (MOD)", errno);
  }
}

void EpollManager::ModifyFd(int fd, uint32_t events, EventCallback cb)
{
  ModifyFd(fd, events);
  callbacks_[fd] = std::move(cb);
}

void EpollManager::RemoveFd(int fd)
{
  if (epoll_ctl(ep_fd_, EPOLL_CTL_DEL, fd, nullptr) < 0)
    throw EPollManagerException("epoll_ctl (DEL)", errno);
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
    int n = epoll_pwait(ep_fd_, events, MAX_EVENTS, -1, &waitMask);
    if (n == -1)
    {
      if (errno == EINTR)
        continue;  // EINTR from signal, loop again to check shutdown
      throw EPollManagerException("epoll_pwait", errno);
    }
    for (int i = 0; i < n; i++)
    {
      int fd = events[i].data.fd;
      callbacks_.find(fd)->second(*this, events[i]);
    }
  }
}
