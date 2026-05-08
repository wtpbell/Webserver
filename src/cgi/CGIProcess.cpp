/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGIProcess.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/20 15:28:01 by jboon         #+#    #+#                 */
/*   Updated: 2026/05/03 22:15:22 by jboon         ########   odam.nl         */
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
#include <optional>
#include <utility>

#include "Logger.hpp"
#include "cgi/CGIParser.hpp"
#include "cgi/CGIRequest.hpp"
#include "cgi/CGIResponse.hpp"
#include "io/Socket.hpp"

namespace cgi
{
  CGIProcess::CGIProcess(pid_t child_pid, CGIRequest&& request, std::pair<Socket, Socket>&& sockets) noexcept
      : childPid_(child_pid), request_(std::move(request))
  {
    if (childPid_ == -1)
    {
      state_ = CGIState::kError;
    }
    else if (childPid_ == 0)
    {
      socket_ = std::move(sockets.second);
      state_ = CGIState::kIsChild;
    }
    else
    {
      socket_ = std::move(sockets.first);
      state_ = CGIState::kRunning;
    }
  }

  CGIProcess::CGIProcess(CGIProcess&& other) noexcept
      : state_(std::exchange(other.state_, CGIState::kNone)),
        childPid_(std::exchange(other.childPid_, -1)),
        waitStatus_(std::exchange(other.waitStatus_, -1)),
        socket_(std::move(other.socket_)),
        body_(std::move(other.body_)),
        request_(std::move(other.request_))
  {
  }

  CGIProcess::~CGIProcess(void)
  {
    const int oldPid = childPid_;
    if (childPid_ > 0)
    {
      KillChild();
      WaitPid(true);
    }

    if (oldPid > 0 && waitStatus_ != 0)
    {
      Logger::Log(LogLevel::WARNING, "{} failed to exit normally: {}", request_.GetExecutable(), GetWaitStatus());
    }
  }

  CGIProcess& CGIProcess::operator=(CGIProcess&& rhs) noexcept
  {
    if (this != &rhs)
    {
      state_ = std::exchange(rhs.state_, CGIState::kNone);
      childPid_ = std::exchange(rhs.childPid_, -1);
      waitStatus_ = std::exchange(rhs.waitStatus_, -1);
      socket_ = std::move(rhs.socket_);
      body_ = std::move(rhs.body_);
      request_ = std::move(rhs.request_);
    }
    return *this;
  }

  bool CGIProcess::IsParent(void) const noexcept
  {
    return childPid_ > 0;
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

  CGIProcess::ReturnState CGIProcess::WaitPid(bool wait_for_child) noexcept
  {
    if (childPid_ == -1)
    {
      return ReturnState::kDone;
    }

    errno = 0;
    int options = wait_for_child ? 0 : WNOHANG;
    pid_t waited_on = waitpid(childPid_, &waitStatus_, options);
    if (waited_on == -1 && errno != EINTR)
    {
      childPid_ = -1;
      state_ = CGIState::kError;
      return ReturnState::kFail;
    }

    if (waited_on == childPid_)
    {
      childPid_ = -1;
      state_ = CGIState::kComplete;
      return ReturnState::kDone;
    }

    state_ = CGIState::kWaitPid;
    return ReturnState::kOk;
  }

  bool CGIProcess::KillChild(void) noexcept
  {
    if (childPid_ == -1)
      return false;
    return kill(-childPid_, SIGKILL) == 0;
  }

  const Socket& CGIProcess::GetSocket(void) const noexcept
  {
    return socket_;
  }

  const CGIRequest& CGIProcess::GetRequest(void) const noexcept
  {
    return request_;
  }

  CGIProcess::CGIState CGIProcess::GetState(void) const noexcept
  {
    return state_;
  }

  int CGIProcess::GetWaitStatus(void) const noexcept
  {
    if (WIFEXITED(waitStatus_))
      return WEXITSTATUS(waitStatus_);
    if (WIFSIGNALED(waitStatus_))
      return 128 + WTERMSIG(waitStatus_);
    return waitStatus_;
  }

  std::optional<CGIResponse> CGIProcess::ParseCGIResponse(void) const noexcept
  {
    std::optional<CGIResponse> cgiResponse = CGIParser::Parse(body_);
    if (cgiResponse.has_value() && request_.IsClosedConnection())
    {
      cgiResponse->EmplaceHeader("Connection", "close");
    }
    return cgiResponse;
  }

  CGIProcess::ReturnState CGIProcess::SendRequest(void) noexcept
  {
    std::size_t& leftover = request_.GetLeftover();
    if (leftover > 0)
    {
      ssize_t bytes = socket_.Send(request_.GetBody(), leftover, MSG_DONTWAIT);
      if (bytes == -1)
      {
        state_ = CGIState::kError;
        return ReturnState::kFail;
      }
    }

    if (leftover == 0)
    {
      socket_.Shutdown(Socket::ShutdownState::kShutWr);
      return ReturnState::kDone;
    }
    return ReturnState::kOk;
  }

  CGIProcess::ReturnState CGIProcess::AwaitResponse(void) noexcept
  {
    ssize_t bytes = socket_.Recv(body_, MSG_DONTWAIT);
    if (bytes == -1)
    {
      state_ = CGIState::kError;
      return ReturnState::kFail;
    }

    if (bytes > 0)
    {
      return ReturnState::kOk;
    }

    socket_.Shutdown(Socket::ShutdownState::kShutRd);
    state_ = CGIState::kWaitPid;
    return ReturnState::kDone;
  }
}  // namespace cgi
