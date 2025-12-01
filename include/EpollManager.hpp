/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   EpollManager.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/14 15:42:09 by bewong        #+#    #+#                 */
/*   Updated: 2025/12/01 10:58:55 by jboon         ########   odam.nl         */
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
    using EventCallback = std::function<void(EpollManager& manager, const struct epoll_event&)>;

    EpollManager(void);
    EpollManager(const EpollManager& other) = delete;
    EpollManager(EpollManager&& other) noexcept = delete;
    EpollManager& operator=(const EpollManager& other) = delete;
    EpollManager& operator=(EpollManager&& other) noexcept = delete;
    ~EpollManager(void);

    void addFd(int fd, uint32_t events, EventCallback cb);
    void modifyFd(int fd, uint32_t events);
    void modifyFd(int fd, uint32_t events, EventCallback cb);
    void removeFd(int fd);
    void eventLoop(void);
#ifdef UNIT_TEST
    int getEpFd() const
    {
      return epFd_;
    }
    auto& getCallbacks()
    {
      return callbacks_;
    }
#endif

  private:
    int epFd_;
    std::unordered_map<int, EventCallback> callbacks_;
};
#endif  // EPOLLMANAGER_HPP_
