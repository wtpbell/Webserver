/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   test_integration_cgi.cpp                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/02/19 10:35:00 by jboon         #+#    #+#                 */
/*   Updated: 2026/05/10 19:47:16 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <sys/epoll.h>

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

#include "cgi/CGI.hpp"
#include "cgi/CGIError.hpp"
#include "cgi/CGIProcess.hpp"
#include "cgi/CGIRequest.hpp"
#include "config/RouteView.hpp"
#include "core/EpollManager.hpp"
#include "http/HTTPParser.hpp"
#include "http/HTTPRequest.hpp"
#include "io/TimerFD.hpp"
#include "utils/Logger.hpp"
#include "utils/signal.hpp"

#define BUF_SIZE 1024

namespace FileSystem = std::filesystem;

namespace  // Exit
{
  enum ExitCode  // TODO: compare with cgi error
  {
    kSuccess = EXIT_SUCCESS,
    kFailure = EXIT_FAILURE,
    kFailedToCreateHTTPRequest = 3,  // safe exit code range is from 3-125
    kFailedToCreateTimerFd,
    kFailedToRegisterFd,
    kFailedEventLoop,
    kCGIScriptNotFound = 7,
    kCGIScriptMissingPermission,
    kCGIForkFailure,
    kCGIScriptFailureToExec,
    kCGISocketPairFailure,
    kScriptForbidden,
    kInvalidResponse,
    kCGIScriptTimeOut
  };

  ExitCode ToExitCode(cgi::CGIErrorCode cgiErrorCode)
  {
    return static_cast<ExitCode>(ExitCode::kCGIScriptNotFound + (static_cast<int>(cgiErrorCode) - 1));
  }

  std::atomic<int> g_exitCode;
}  // namespace

namespace  // HELPERS
{
  FileSystem::path GetRootPathTo(std::string_view dir)
  {
    return {FileSystem::current_path() / dir};
  }

  std::string GetRawRequest(void)
  {
    char buffer[BUF_SIZE];
    std::string message;

    while (std::cin.good() && std::cin.get(buffer, BUF_SIZE, '\0'))
    {
      message.append(buffer);
    }
    return message;
  }

  std::optional<HTTPRequest> CreateHTTPRequest(std::string_view message)
  {
    HTTPParser parser;

    auto result = parser.Parse(message);
    if (result == HTTPParser::ParseResult::kError || result == HTTPParser::ParseResult::kNeedMoreData)
      return std::nullopt;

    const HTTPRequest& request = parser.GetRequest();
    if (parser.NeedsBodyDecision())
    {
      if (request.HasHeader("transfer-encoding"))
        parser.SetChunked();
      else if (request.GetContentLength())
        parser.SetContentLength(request.GetContentLength().value());
      else
        parser.SetNoBody();
      parser.Parse("");
    }

    if (parser.HasError())
      return std::nullopt;
    return request;
  }
}  // namespace

namespace  // EVENT HANDLERS
{
  void HandleTimeout(EpollManager& epollManager, const epoll_event& event)
  {
    (void)epollManager, (void)event;
    std::cerr << "HandleTimeout: Time out occcured!" << std::endl;
    g_shutdown.store(true);
    g_exitCode.store(ExitCode::kCGIScriptTimeOut);
  }

  void HandleProcess(cgi::CGIProcess& cgiProcess, EpollManager& epollManager, const epoll_event& event)
  {
    using CGIState = cgi::CGIProcess::CGIState;
    using ReturnState = cgi::CGIProcess::ReturnState;

    Logger::SetLogFilter(LogLevel::NONE);

    (void)epollManager;
    if (event.events & EPOLLERR)
    {
      g_shutdown.store(true);
      g_exitCode.store(ExitCode::kFailure);
      std::cerr << "HandleProcess: event error!" << std::endl;
      return;
    }

    if (event.events & EPOLLOUT && cgiProcess.GetState() == CGIState::kRunning)
    {
      if (cgiProcess.SendRequest() == ReturnState::kFail)
      {
        std::cerr << "HandleProcess: SendRequest error!" << errno << std::endl;
      }
    }
    if (event.events & EPOLLIN && cgiProcess.GetState() == CGIState::kRunning)
    {
      if (cgiProcess.AwaitResponse() == ReturnState::kFail)
      {
        std::cerr << "HandleProcess: AwaitResponse error!" << errno << std::endl;
      }
    }

    if (cgiProcess.GetState() == CGIState::kWaitPid)
    {
      cgiProcess.WaitPid();
    }

    if (cgiProcess.IsCompleted())
    {
      const int waitStatus = cgiProcess.GetWaitStatus();
      auto cgiResponse = cgiProcess.ParseCGIResponse();
      if (waitStatus != 0)
      {
        g_exitCode.store(waitStatus);
      }
      else if (!cgiResponse.has_value())
      {
        g_exitCode.store(ExitCode::kInvalidResponse);
      }
      else
      {
        std::cout << *cgiResponse << std::endl;
      }
      g_shutdown.store(true);
    }
    else if (cgiProcess.IsError())
    {
      g_shutdown.store(true);
      g_exitCode.store(ExitCode::kFailure);
      std::cerr << "HandleProcess: process error!" << std::endl;
    }
  }
}  // namespace

int main(void)
{
  // Read in the request
  std::string rawRequest{GetRawRequest()};
  if (rawRequest.empty() || std::cin.fail())
    return ExitCode::kFailure;

  setupSignals();
  EpollManager epollManager;
  if (epollManager.Init() != EpollManager::Result::kOk)
  {
    return ExitCode::kFailure;
  }

  // Parse the request
  auto httpRequest = CreateHTTPRequest(rawRequest);
  if (!httpRequest.has_value())
    return ExitCode::kFailedToCreateHTTPRequest;

  const cgi::IpPort ipPort{"127.0.0.1", "8080"};
  RouteView route;
  route.locationPrefix = "/cgi-bin";
  route.alias.emplace(GetRootPathTo("tests/cgi-bin"));
  route.cgi = true;

  auto cgiRoute = cgi::SetupCGIRoute(httpRequest->GetPath(), route);
  if (!cgiRoute.HasValue())
  {
    return ToExitCode(cgiRoute.GetError());
  }

  // Start up the CGI process
  cgi::CGIRequest cgiRequest{httpRequest.value(), cgiRoute.GetValue(), ipPort, "127.0.0.2:8080", route};
  auto expCgiProcess = cgi::ExecuteCGI(std::move(cgiRequest));
  if (!expCgiProcess.HasValue())
  {
    return ToExitCode(expCgiProcess.GetError());
  }

  cgi::CGIProcess cgiProcess{expCgiProcess.ExtractValue()};
  auto callbackHandleProcess = [&cgiProcess](EpollManager& epollManager, const epoll_event& event)
  {
    HandleProcess(cgiProcess, epollManager, event);
  };

  auto callbackTimeOut = [](EpollManager& epollManager, const epoll_event& event)
  {
    HandleTimeout(epollManager, event);
  };

  auto timerFd = TimerFD::CreateMonotonicClock(2000, 500);
  if (!timerFd.HasValue())
  {
    return ExitCode::kFailedToCreateTimerFd;
  }

  if (epollManager.AddFd(timerFd->GetFD(), EPOLLIN, callbackTimeOut) != EpollManager::Result::kOk)
  {
    return ExitCode::kFailedToRegisterFd;
  }

  if (epollManager.AddFd(cgiProcess.GetSocket().GetFD(), EPOLLIN | EPOLLOUT, callbackHandleProcess) !=
      EpollManager::Result::kOk)
  {
    return ExitCode::kFailedToRegisterFd;
  }

  g_exitCode.store(ExitCode::kSuccess);
  if (epollManager.EventLoop() != EpollManager::Result::kOk)
  {
    return ExitCode::kFailedEventLoop;
  }
  return g_exitCode;
}
