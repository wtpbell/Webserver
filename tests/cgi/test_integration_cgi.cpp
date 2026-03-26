/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   test_integration_cgi.cpp                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/02/19 10:35:00 by jboon         #+#    #+#                 */
/*   Updated: 2026/02/24 17:21:09 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <thread>

#include "cgi/CGI.hpp"
#include "cgi/CGIProcess.hpp"
#include "http/HTTPParser.hpp"
#include "http/HTTPRequest.hpp"

#define BUF_SIZE 1024

namespace FileSystem = std::filesystem;

enum ExitCode
{
  kSuccess = EXIT_SUCCESS,
  kFailure = EXIT_FAILURE,
  kFailedToCreateHTTPRequest = 3,  // safe exit code range is from 3-125
  kCGIScriptNotFound = 4,
  kCGIScriptIsSymlink,
  kCGIScriptIsDirectory,
  kCGIScriptMissingPermission,
  kCGIForkFailure,
  kCGIRedirectionFailure,
  kCGIScriptFailureToExec,
  kCGISocketPairFailure,
  kCGIScriptTimeOut,
  kCGIInvalidResponse,
};

class StopWatch
{
  public:
    void Start(std::chrono::milliseconds max_duration)
    {
      timed_out = false;
      max_duration_ = max_duration;
      start_ = std::chrono::high_resolution_clock::now();
    }

    bool Tick(void)
    {
      clock end = std::chrono::high_resolution_clock::now();
      timed_out = (end - start_) >= max_duration_;
      return !timed_out;
    }

    bool TimedOut(void) const noexcept
    {
      return timed_out;
    }

    using clock = std::chrono::time_point<std::chrono::system_clock>;
    using milliseconds = std::chrono::milliseconds;

  private:
    milliseconds max_duration_;
    clock start_;
    bool timed_out;
};

FileSystem::path GetRootPathTo(std::string_view dir)
{
  return {FileSystem::current_path() / dir};
}

std::string GetRawRequest(void)
{
  char buffer[BUF_SIZE];
  std::string message;

  while (std::cin.good() && std::cin.get(buffer, BUF_SIZE, '\0'))
    message.append(buffer);
  return message;
}

std::optional<HTTPRequest> CreateHTTPRequest(std::string_view message)
{
  HTTPParser parser;

  auto result = parser.Parse(message);
  if (result == HTTPParser::ParseResult::Error || result == HTTPParser::ParseResult::NeedMoreData)
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

int HandleProcess(cgi::CGIProcess& process)
{
  using namespace std::chrono_literals;
  StopWatch stop_watch;

  stop_watch.Start(2000ms);
  while (stop_watch.Tick() && !process.IsError() && !process.IsCompleted())
  {
    switch (process.GetState())
    {
      case cgi::CGIProcess::CGIState::kSendRequest:
        process.SendRequest();
        break;
      case cgi::CGIProcess::CGIState::kAwaitResponse:
        process.AwaitResponse();
        break;
      case cgi::CGIProcess::CGIState::kWaitPid:
        process.WaitPid();
        break;
      default:
        return ExitCode::kCGIScriptTimeOut;
        break;
    }
    std::this_thread::sleep_for(50ms);
  }

  if (stop_watch.TimedOut())
  {
    return ExitCode::kCGIScriptTimeOut;
  }

  if (process.IsError())
  {
    return ExitCode::kFailure;
  }

  if (process.GetWaitStatus() == 0)
  {
    if (process.GetResponse().GetStatus().status_code >= 500)
    {
      return ExitCode::kCGIInvalidResponse;
    }
    std::cout << process.GetResponse() << std::endl;
    return 0;
  }

  return process.GetWaitStatus();
}

int main(void)
{
  // Read in the request
  std::string raw_request{GetRawRequest()};
  if (raw_request.empty() || std::cin.fail())
    return ExitCode::kFailure;

  // Parse the request
  auto http_request = CreateHTTPRequest(raw_request);
  if (!http_request.has_value())
    return ExitCode::kFailedToCreateHTTPRequest;

  // Start up the CGI process
  auto process =
      cgi::ExecuteCGI(http_request.value(), {GetRootPathTo("tests/cgi-bin")}, "127.0.0.1:8080", "127.0.0.2:8080");
  if (!process.HasValue())
  {
    return ExitCode::kCGIScriptNotFound + (static_cast<int>(process.GetError()) - 1);
  }

  // Process the cgi
  std::signal(SIGPIPE, SIG_IGN);
  return HandleProcess(process.GetValue());
}
