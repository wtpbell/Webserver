/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGIProcess.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/20 15:08:10 by jboon         #+#    #+#                 */
/*   Updated: 2026/01/20 15:08:58 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIPROCESS_H_
#define CGIPROCESS_H_

#include <bits/types/error_t.h>
#include <unistd.h>

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
        kSendRequest,
        kAwaitResponse,
        kWaitPid,
        kComplete,
        kError,
        kIsChild,
      };

      bool IsParent(void) const noexcept;
      bool IsError(void) const noexcept;
      bool IsCompleted(void) const noexcept;
      void CloseSocket(void) noexcept;
      CGIState SendRequest(void) noexcept;
      CGIState AwaitResponse(void) noexcept;
      CGIState WaitPid(bool wait_for_child = false) noexcept;
      bool KillChild(void) noexcept;

      const Socket& GetSocket(void) const noexcept;
      const CGIRequest& GetRequest(void) const noexcept;
      const CGIResponse& GetResponse(void) const noexcept;
      CGIState GetState(void) const noexcept;
      int GetWaitStatus(void) const noexcept;

    private:
      CGIState state_{CGIState::kNone};
      pid_t child_pid_{-1};
      int wait_status_{-1};
      Socket socket_;
      std::string body_;
      CGIRequest request_;
      CGIResponse response_;
  };
}  // namespace cgi

#endif  // CGIPROCESS_H_
