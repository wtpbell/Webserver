/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   EpollManager.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/14 15:42:09 by bewong        #+#    #+#                 */
/*   Updated: 2026/01/08 19:57:18 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef EPOLLMANAGER_HPP
#define EPOLLMANAGER_HPP

#include <fcntl.h>

#include <atomic>
#include <functional>
#include <unordered_map>

#include "io/SharedFD.hpp"

class EpollManager : private SharedFD
{
  public:
    using EventCallback = std::function<void(EpollManager&, const struct epoll_event&)>;

    EpollManager(void);
    EpollManager(const EpollManager& other) = delete;
    EpollManager(EpollManager&& other) noexcept = delete;
    EpollManager& operator=(const EpollManager& other) = delete;
    EpollManager& operator=(EpollManager&& other) noexcept = delete;
    ~EpollManager(void) = default;

    void AddFd(int fd, uint32_t events, EventCallback cb);
    void ModifyFd(int fd, uint32_t events);
    void ModifyFd(int fd, uint32_t events, EventCallback cb);
    void RemoveFd(int fd);
    void EventLoop(void);

#ifdef UNIT_TEST
    auto& GetCallbacks()
    {
      return callbacks_;
    }
#endif

  private:
    std::unordered_map<int, EventCallback> callbacks_;
};
#endif  // EPOLLMANAGER_HPP_
