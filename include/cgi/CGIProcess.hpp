/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGIProcess.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/20 15:08:10 by jboon         #+#    #+#                 */
/*   Updated: 2026/05/03 22:11:06 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIPROCESS_H_
#define CGIPROCESS_H_

#include <unistd.h>

#include <optional>
#include <string>
#include <utility>

#include "cgi/CGIRequest.hpp"
#include "cgi/CGIResponse.hpp"
#include "io/Socket.hpp"

namespace cgi
{
  class CGIProcess
  {
    public:
      CGIProcess(pid_t child_pid, CGIRequest&& request, std::pair<Socket, Socket>&& sockets) noexcept;
      CGIProcess(void) = delete;
      CGIProcess(const CGIProcess& other) = delete;
      CGIProcess(CGIProcess&& other) noexcept;
      ~CGIProcess(void);

      CGIProcess& operator=(const CGIProcess& rhs) = delete;
      CGIProcess& operator=(CGIProcess&& rhs) noexcept;

      enum class CGIState
      {
        kNone,
        kRunning,
        kWaitPid,
        kComplete,
        kError,
        kIsChild,
      };

      enum class ReturnState
      {
        kOk,
        kDone,
        kFail
      };

      bool IsParent(void) const noexcept;
      bool IsError(void) const noexcept;
      bool IsCompleted(void) const noexcept;
      void CloseSocket(void) noexcept;
      ReturnState SendRequest(void) noexcept;
      ReturnState AwaitResponse(void) noexcept;
      ReturnState WaitPid(bool wait_for_child = false) noexcept;
      bool KillChild(void) noexcept;

      const Socket& GetSocket(void) const noexcept;
      const CGIRequest& GetRequest(void) const noexcept;
      std::optional<CGIResponse> ParseCGIResponse(void) const noexcept;
      CGIState GetState(void) const noexcept;
      int GetWaitStatus(void) const noexcept;

    private:
      CGIState state_{CGIState::kNone};
      pid_t childPid_{-1};
      int waitStatus_{-1};
      Socket socket_;
      std::string body_;
      CGIRequest request_;
  };
}  // namespace cgi

#endif  // CGIPROCESS_H_
