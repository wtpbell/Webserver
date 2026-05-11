/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   EpollManager.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/14 15:42:09 by bewong        #+#    #+#                 */
/*   Updated: 2026/04/24 15:51:36 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef EPOLLMANAGER_HPP
#define EPOLLMANAGER_HPP

#include <fcntl.h>

#include <functional>
#include <unordered_map>

#include "io/SharedFD.hpp"

class EpollManager : private SharedFD
{
  public:
    using EventCallback = std::function<void(EpollManager&, const struct epoll_event&)>;

    enum class Result
    {
      kOk,
      kNotFound,
      kInvalidState,
      kSyscallError
    };

    EpollManager();
    EpollManager(const EpollManager& other) = delete;
    EpollManager(EpollManager&& other) noexcept = delete;
    EpollManager& operator=(const EpollManager& other) = delete;
    EpollManager& operator=(EpollManager&& other) noexcept = delete;
    ~EpollManager() = default;

    Result Init();

    Result AddFd(int fd, uint32_t events, EventCallback cb);
    Result ModifyFd(int fd, uint32_t events);
    Result ModifyFd(int fd, uint32_t events, EventCallback cb);
    Result RemoveFd(int fd);
    Result EventLoop();
    static const char* ToString(Result result);

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
