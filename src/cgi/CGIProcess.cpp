/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGIProcess.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/20 15:28:01 by jboon         #+#    #+#                 */
/*   Updated: 2026/03/17 00:24:35 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "cgi/CGIProcess.hpp"

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <utility>

#include "cgi/CGIParser.hpp"
#include "cgi/CGIRequest.hpp"
#include "cgi/CGIResponse.hpp"
#include "io/Socket.hpp"

namespace cgi
{
  CGIProcess::CGIProcess(pid_t child_pid, CGIRequest&& request, std::pair<Socket, Socket>&& sockets) noexcept
      : child_pid_(child_pid), request_(std::move(request))
  {
    if (child_pid_ == -1)
    {
      state_ = CGIState::kError;
    }
    else if (child_pid_ == 0)
    {
      socket_ = std::move(sockets.second);
      state_ = CGIState::kIsChild;
    }
    else
    {
      socket_ = std::move(sockets.first);
      if (request.GetLeftover() > 0)
      {
        state_ = CGIState::kSendRequest;
      }
      else
      {
        state_ = CGIState::kAwaitResponse;
        socket_.Shutdown(Socket::ShutdownState::kShutWr);
      }
    }
  }

  CGIProcess::CGIProcess(CGIProcess&& other) noexcept
  {
    state_ = std::exchange(other.state_, CGIState::kNone);
    child_pid_ = std::exchange(other.child_pid_, -1);
    wait_status_ = std::exchange(other.wait_status_, -1);
    socket_ = std::move(other.socket_);
    body_ = std::move(other.body_);
    request_ = std::move(other.request_);
    response_ = std::move(other.response_);
  }

  CGIProcess::~CGIProcess(void)
  {
    if (child_pid_ > 0)
    {
      KillChild();
      WaitPid(true);
    }
  }

  CGIProcess& CGIProcess::operator=(CGIProcess&& rhs) noexcept
  {
    if (this == &rhs)
      return *this;

    state_ = std::exchange(rhs.state_, CGIState::kNone);
    child_pid_ = std::exchange(rhs.child_pid_, -1);
    wait_status_ = std::exchange(rhs.wait_status_, -1);
    socket_ = std::move(rhs.socket_);
    body_ = std::move(rhs.body_);
    request_ = std::move(rhs.request_);
    response_ = std::move(rhs.response_);
    return *this;
  }

  bool CGIProcess::IsParent(void) const noexcept
  {
    return child_pid_ > 0;
  }

  bool CGIProcess::IsError(void) const noexcept
  {
    return state_ == CGIState::kError;
  }

  bool CGIProcess::IsCompleted(void) const noexcept
  {
    return state_ == CGIProcess::CGIState::kComplete;
  }

  void CGIProcess::CloseSocket(void) noexcept
  {
    socket_.Reset();
  }

  CGIProcess::CGIState CGIProcess::WaitPid(bool wait_for_child) noexcept
  {
    if (child_pid_ == -1)
      return state_;

    errno = 0;
    int options = wait_for_child ? 0 : WNOHANG;
    pid_t waited_on = waitpid(child_pid_, &wait_status_, options);
    if (waited_on == -1 && errno != EINTR)
    {
      child_pid_ = -1;
      state_ = CGIState::kError;
    }
    else if (waited_on == child_pid_)
    {
      child_pid_ = -1;
      state_ = CGIState::kComplete;
    }
    else
    {
      state_ = CGIState::kWaitPid;
    }

    return state_;
  }

  bool CGIProcess::KillChild(void) noexcept
  {
    if (child_pid_ == -1)
      return false;
    return kill(-child_pid_, SIGKILL) == 0;
  }

  const Socket& CGIProcess::GetSocket(void) const noexcept
  {
    return socket_;
  }

  const CGIRequest& CGIProcess::GetRequest(void) const noexcept
  {
    return request_;
  }

  const CGIResponse& CGIProcess::GetResponse(void) const noexcept
  {
    return response_;
  }

  CGIProcess::CGIState CGIProcess::GetState(void) const noexcept
  {
    return state_;
  }

  int CGIProcess::GetWaitStatus(void) const noexcept
  {
    if (WIFEXITED(wait_status_))
      return WEXITSTATUS(wait_status_);
    if (WIFSIGNALED(wait_status_))
      return 128 + WTERMSIG(wait_status_);
    return wait_status_;
  }

  CGIProcess::CGIState CGIProcess::SendRequest(void) noexcept
  {
    std::size_t& leftover = request_.GetLeftover();
    ssize_t bytes = socket_.Send(request_.GetBody(), leftover, MSG_DONTWAIT);

    if (bytes < 1)
    {
#ifdef UNIT_TEST
      // TODO: The process itself relies on working with epoll but with the integration test runs without it (might
      // change later)
      if (errno == EWOULDBLOCK || errno == EAGAIN)
        return state_;
#endif
      state_ = CGIState::kError;
    }
    else if (leftover == 0)
    {
      state_ = CGIState::kAwaitResponse;
      socket_.Shutdown(Socket::ShutdownState::kShutWr);
    }
    return state_;
  }

  CGIProcess::CGIState CGIProcess::AwaitResponse(void) noexcept
  {
    ssize_t bytes = socket_.Recv(body_, MSG_DONTWAIT);

    if (bytes > 0)
      return state_;

    if (bytes == -1)
    {
#ifdef UNIT_TEST
      // TODO: Same as TODO above
      if (errno == EWOULDBLOCK || errno == EAGAIN)
        return state_;
#endif
      state_ = CGIState::kError;
    }
    else
    {
      std::optional<CGIResponse> response = CGIParser::Parse(body_);
      if (response.has_value())
      {
        response_ = std::move(response.value());
      }
      else
      {
        response_.SetStatus({500, "Internal Server Error"});
      }
      socket_.Shutdown(Socket::ShutdownState::kShutRd);
      state_ = CGIState::kWaitPid;
    }
    return state_;
  }
}  // namespace cgi
