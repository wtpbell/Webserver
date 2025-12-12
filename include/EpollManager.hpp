/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   EpollManager.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/14 15:42:09 by bewong        #+#    #+#                 */
/*   Updated: 2025/12/11 23:10:03 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef EPOLLMANAGER_HPP
#define EPOLLMANAGER_HPP

#include <fcntl.h>

#include <atomic>
#include <functional>
#include <string>
#include <unordered_map>

extern std::atomic<bool> g_shutdown;

class EpollManager
{
  public:
    using EventCallback = std::function<void(EpollManager&, const struct epoll_event&)>;

    EpollManager(void);
    EpollManager(const EpollManager& other) = delete;
    EpollManager(EpollManager&& other) noexcept = delete;
    EpollManager& operator=(const EpollManager& other) = delete;
    EpollManager& operator=(EpollManager&& other) noexcept = delete;
    ~EpollManager(void);

    void AddFd(int fd, uint32_t events, EventCallback cb);
    void ModifyFd(int fd, uint32_t events);
    void ModifyFd(int fd, uint32_t events, EventCallback cb);
    void RemoveFd(int fd);
    void EventLoop(void);

#ifdef UNIT_TEST
    int GetEpFd() const
    {
      return ep_fd_;
    }
    auto& GetCallbacks()
    {
      return callbacks_;
    }
#endif

  private:
    int ep_fd_;
    std::unordered_map<int, EventCallback> callbacks_;
};
#endif  // EPOLLMANAGER_HPP_
