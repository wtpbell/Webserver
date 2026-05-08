/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/13 19:08:02 by bewong        #+#    #+#                 */
/*   Updated: 2026/05/07 23:49:32 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

#include <sys/epoll.h>
#include <sys/socket.h>

#include <cassert>
#include <chrono>
#include <cstdint>
#include <optional>
#include <stdexcept>

#include "Connection.hpp"
#include "ConnectionRegistry.hpp"
#include "EpollManager.hpp"
#include "Logger.hpp"
#include "RequestContext.hpp"
#include "cgi/CGIProcess.hpp"
#include "cgi/CGIResponse.hpp"
#include "config/ServerRegistry.hpp"
#include "io/Socket.hpp"
#include "io/TimerFD.hpp"
#include "string.hpp"

Server::Server(const IpPort& ipPort, Socket::Type type, const ServerRegistry& serverRegistry,
               EpollManager& epollManager)
    : ipPort_(ipPort),
      socket_(Socket::CreateSocket(ipPort.ip.c_str(), ipPort.port.c_str(), AI_PASSIVE, type)),
      serverRegistry_(serverRegistry),
      requestContext_(ipPort_, router_, serverRegistry_, sessionManager_, connectionRegistry_, epollManager,
                      [this](EpollManager& manager, const epoll_event& event)
                      {
                        HandleCGI(manager, event);
                      })
{
  const int yes{1};
  const int no{0};

  socket_.SetNonBlocking(true);
  socket_.SetSockOpt(SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
  if (type == Socket::Type::kIPv6)
  {
    socket_.SetSockOpt(IPPROTO_IPV6, IPV6_V6ONLY, &no, sizeof(no));  // enable dual stack socket
  }
  socket_.Bind();
  socket_.Listen(SOMAXCONN);

  EpollManager::Result result = epollManager.AddFd(static_cast<int>(socket_), EPOLLIN | EPOLLERR,
                                                   [this](EpollManager& manager, const struct epoll_event& event)
                                                   {
                                                     this->Accept(manager, event);
                                                   });
  if (result != EpollManager::Result::kOk)
  {
    // Lets not talk about this
    throw std::runtime_error(
        std::string("EpollManager: failed to add server to epoll: ").append(EpollManager::ToString(result)));
  }
  Logger::Log(LogLevel::INFO, "Server <{}> constructed...", socket_);
}

Server::~Server(void)
{
  Logger::Log(LogLevel::INFO, "Server <{}> shutting down with {} active connections", socket_,
              connectionRegistry_.GetConnectionCount());
}

void Server::Accept(EpollManager& manager, const struct epoll_event& event)
{
  if ((event.events & EPOLLERR) == EPOLLERR)
  {
    Logger::Log(LogLevel::ERROR, "Server <{}> socket error", socket_);
    manager.RemoveFd(socket_.GetFD());
    return;
  }

  auto connectionCallback = [this](EpollManager& manager, const struct epoll_event& event)
  {
    HandleConnection(manager, event);
  };

  auto timerCallback = [this](EpollManager& manager, const struct epoll_event& event)
  {
    HandleTimeout(manager, event);
  };

  const int max_accept = 100;
  for (int i = 0; i < max_accept; ++i)
  {
    Socket client{socket_.Accept4(SOCK_NONBLOCK)};
    if (client.GetFD() == -1)
      break;

    const int clientFd = client.GetFD();
    ConnectionRegistry::ConnectionData* connectionData = connectionRegistry_.CreateConnection(std::move(client));
    if (connectionData == nullptr)
    {
      Logger::Log(LogLevel::CRITICAL, "Server <{}>: Failed to create the connection {}!", socket_, clientFd);
      continue;
    }

    EpollManager::Result addResult =
        manager.AddFd(clientFd, EPOLLERR | EPOLLHUP | EPOLLRDHUP | EPOLLIN, connectionCallback);
    if (addResult != EpollManager::Result::kOk)
    {
      Logger::Log(LogLevel::ERROR, "Server <{}> unable to register client <{}> into epoll: {}. Dropping connection!",
                  socket_, clientFd, EpollManager::ToString(addResult));
      connectionRegistry_.EraseConnection(clientFd);
      continue;
    }

    const TimerFD& timer = connectionData->timer_;
    addResult = manager.AddFd(timer.GetFD(), EPOLLERR | EPOLLHUP | EPOLLRDHUP | EPOLLIN, timerCallback);
    if (addResult != EpollManager::Result::kOk)
    {
      Logger::Log(LogLevel::ERROR,
                  "Server <{}> unable to register timer <{}> for client <{}> into epoll: {}. Dropping connection!",
                  socket_, timer.GetFD(), clientFd, EpollManager::ToString(addResult));
      manager.RemoveFd(clientFd);
      connectionRegistry_.EraseConnection(clientFd);
      continue;
    }

    Logger::Log(LogLevel::INFO, "Server <{}> accepted client connection <{}>", socket_, clientFd);
  }
}

void Server::HandleConnection(EpollManager& manager, const epoll_event& event)
{
  ConnectionRegistry::ConnectionData* connectionData = connectionRegistry_.FindConnection(event.data.fd);
  assert(connectionData != nullptr && "Server could not find the connection!");

  const int connectionFd = event.data.fd;
  Connection& connection = connectionData->connection_;

  if (event.events & (EPOLLHUP | EPOLLRDHUP))
  {
    connection.SetPeerClosed(true);
    if (!connection.HasPendingOutput() && !(event.events & (EPOLLIN | EPOLLOUT)))
    {
      Logger::Log(LogLevel::INFO, "Server <{}>: Closing connection with <{}>", socket_, connection.GetSocket());
      CloseConnection(manager, *connectionData);
      return;
    }
  }

  Connection::State connectionState =
      (event.events & EPOLLERR) == 0 ? Connection::State::kKeepAlive : Connection::State::kError;
  if (event.events & EPOLLIN && connectionState == Connection::State::kKeepAlive)
  {
    connectionState = connection.HandleRequest(requestContext_);
  }
  if (event.events & EPOLLOUT && connectionState == Connection::State::kKeepAlive)
  {
    connectionState = connection.HandleResponse();
  }

  if (connectionState == Connection::State::kError)
  {
    Logger::Log(LogLevel::INFO, "Server <{}>: Internal server error: Connection closed with client <{}>", socket_,
                connection.GetSocket());
    CloseConnection(manager, *connectionData);
  }
  else if (connectionState == Connection::State::kClose)
  {
    Logger::Log(LogLevel::INFO, "Server <{}>: Connection closed with client <{}>", socket_, connection.GetSocket());
    CloseConnection(manager, *connectionData);
  }
  else
  {
    uint32_t pollEvents = EPOLLERR | EPOLLHUP | EPOLLRDHUP;
    if (connection.HasPendingOutput())
    {
      pollEvents |= EPOLLOUT;
    }
    if (!connection.HasPauseReading())
    {
      pollEvents |= EPOLLIN;
    }
    EpollManager::Result modResult = manager.ModifyFd(connectionFd, pollEvents);
    if (modResult != EpollManager::Result::kOk)
    {
      Logger::Log(LogLevel::ERROR, "Server <{}>: failed to update epoll events for client <{}>: {}", socket_,
                  connectionFd, EpollManager::ToString(modResult));
      CloseConnection(manager, *connectionData);
      return;
    }
  }
}

void Server::ErrorCloseCgiProcess(EpollManager& manager, Connection& connection, cgi::CGIProcess& cgiProcess)
{
  const int cgiFd = cgiProcess.GetSocket().GetFD();

  Logger::Log(LogLevel::ERROR, "Server <{}>: Error occurred with cgi process {}: Erasing cgi process", cgiFd);
  connection.UpdateCgiErrorResponse(cgiFd, HTTP::Status::kInternalServerError, cgiProcess.GetRequest(),
                                    requestContext_);
  connectionRegistry_.EraseCgiProcess(cgiFd);
  manager.RemoveFd(cgiFd);
}

void Server::HandleCGI(EpollManager& manager, const epoll_event& event)
{
  using CGIState = cgi::CGIProcess::CGIState;
  using ReturnState = cgi::CGIProcess::ReturnState;

  const int cgiFd = event.data.fd;
  ConnectionRegistry::CGIProcessData* processData = connectionRegistry_.FindCGIProcess(cgiFd);  // return process data
  if (processData == nullptr)
  {
    Logger::Log(LogLevel::ERROR, "HandleCGI: cgiFd {} does not exist in the registry", cgiFd);
    assert(false && "cgi process does not exist in the registry");
    manager.RemoveFd(cgiFd);
    return;
  }

  ConnectionRegistry::ConnectionData* connectionData = connectionRegistry_.FindConnection(cgiFd);
  Connection& connection = connectionData->connection_;

  if (event.events & EPOLLERR)
  {
    ErrorCloseCgiProcess(manager, connection, processData->cgiProcess_);
    return;
  }

  cgi::CGIProcess& cgiProcess = processData->cgiProcess_;
  if (event.events & EPOLLOUT && cgiProcess.GetState() == CGIState::kRunning)
  {
    switch (cgiProcess.SendRequest())
    {
      case ReturnState::kFail:
        connection.UpdateCgiErrorResponse(cgiFd, HTTP::Status::kInternalServerError, cgiProcess.GetRequest(),
                                          requestContext_);
        break;
      case ReturnState::kDone:
        if (manager.ModifyFd(cgiFd, EPOLLERR | EPOLLIN) != EpollManager::Result::kOk)
        {
          ErrorCloseCgiProcess(manager, connection, cgiProcess);
          return;
        }
      case ReturnState::kOk:
        break;
    }
  }

  if (event.events & EPOLLIN && cgiProcess.GetState() == CGIState::kRunning)
  {
    switch (cgiProcess.AwaitResponse())
    {
      case ReturnState::kFail:
        connection.UpdateCgiErrorResponse(cgiFd, HTTP::Status::kInternalServerError, cgiProcess.GetRequest(),
                                          requestContext_);
        break;
      case ReturnState::kDone:
      {
        std::optional<cgi::CGIResponse> cgiResponse = cgiProcess.ParseCGIResponse();
        if (!cgiResponse.has_value())
        {
          connection.UpdateCgiErrorResponse(cgiFd, HTTP::Status::kInternalServerError, cgiProcess.GetRequest(),
                                            requestContext_);
        }
        else
        {
          std::optional<std::string> localTarget = cgiResponse->LocalRedirectTarget();
          if (localTarget.has_value())
          {
            const int newCgiFd = connection.RedirectCgiResponse(cgiFd, *localTarget,
                                                                cgiProcess.GetRequest().GetHostname(), requestContext_);
            if (newCgiFd != cgiFd)
            {
              connectionRegistry_.FindCGIProcess(newCgiFd)->timeStamp_ = processData->timeStamp_;
            }
          }
          else
          {
            connection.UpdateCgiResponse(cgiFd, *cgiResponse);
          }
        }
        break;
      }
      case ReturnState::kOk:
        break;
    }
  }

  switch (cgiProcess.GetState())
  {
    case CGIState::kRunning:
      break;
    case CGIState::kWaitPid:
      if (cgiProcess.WaitPid() == ReturnState::kOk)
      {
        break;
      }
      [[fallthrough]];
    case CGIState::kComplete:
    case CGIState::kError:
      manager.RemoveFd(cgiFd);
      connectionRegistry_.EraseCgiProcess(cgiFd);
      break;
    default:
      assert(false && "HandleCGI: invalid state for cgi process");
      __builtin_unreachable();
  }
}

void Server::HandleTimeout(EpollManager& manager, const epoll_event& event)
{
  namespace chrono = std::chrono;
  using steady_clock = chrono::steady_clock;
  using steady_point = chrono::time_point<steady_clock>;
  using CGIState = cgi::CGIProcess::CGIState;

  const int& timerFd = event.data.fd;
  const int kProcessTimeOut = 10000;      // ms
  const int kConnectionTimeOut = 120000;  // ms

  ConnectionRegistry::ConnectionData* connectionData = connectionRegistry_.FindConnection(timerFd);
  if (connectionData == nullptr)
  {
    Logger::Log(LogLevel::ERROR, "HandleTimeout: timerFd {} not found in the registry", timerFd);
    assert(false && "timerFd not found in the registry");
    manager.RemoveFd(timerFd);
    return;
  }

  if (event.events & EPOLLERR)
  {
    Logger::Log(LogLevel::ERROR, "Server <{}>: Connection <{}> dropped: timerfd error", socket_,
                connectionData->connection_.GetSocket());
    CloseConnection(manager, *connectionData);
    return;
  }

  const steady_point now = steady_clock::now();
  if (chrono::duration_cast<chrono::milliseconds>(now - connectionData->connectionTimestamp_).count() >
      kConnectionTimeOut)
  {
    Logger::Log(LogLevel::INFO, "Server <{}>: Connection <{}> time-out occurred", socket_,
                connectionData->connection_.GetSocket());
    CloseConnection(manager, *connectionData);
    return;
  }

  Connection& connection = connectionData->connection_;
  auto& processes = connectionData->processes_;
  for (ConnectionRegistry::CGIProcessIterator it{processes.begin()}; it != processes.end();)
  {
    const int& cgiFd = it->first;
    const steady_point& timestamp = it->second.timeStamp_;

    if (chrono::duration_cast<chrono::milliseconds>(now - timestamp).count() > kProcessTimeOut)
    {
      cgi::CGIProcess& process = it->second.cgiProcess_;
      switch (process.GetState())
      {
        case CGIState::kRunning:
          connection.UpdateCgiErrorResponse(cgiFd, HTTP::Status::kGatewayTimeout, process.GetRequest(),
                                            requestContext_);
          Logger::Log(LogLevel::INFO, "Server <{}>: cgi process {} time-out occurred at while running", socket_, cgiFd);
          break;
        case CGIState::kWaitPid:
          process.WaitPid();
          [[fallthrough]];
        case CGIState::kComplete:
        case CGIState::kError:
          [[fallthrough]];
        default:
          Logger::Log(LogLevel::INFO, "Server <{}>: process {} time-out occurred at closing ({})", socket_, cgiFd,
                      static_cast<int>(process.GetState()));
          break;
      }
      manager.RemoveFd(cgiFd);
      it = connectionRegistry_.EraseCgiProcess(processes, it);
    }
    else
    {
      ++it;
    }
  }
}

void Server::CloseConnection(EpollManager& manager, ConnectionRegistry::ConnectionData& connectionData)
{
  const int connectionFd = connectionData.connection_.GetSocket().GetFD();
  const int timerFd = connectionData.timer_.GetFD();
  const auto& processes = connectionData.processes_;

  EpollManager::Result removeResult = manager.RemoveFd(timerFd);
  if (removeResult != EpollManager::Result::kOk)
  {
    Logger::Log(LogLevel::WARNING, "Server <{}>: failed to remove timer <{}> from epoll: {}", socket_, timerFd,
                EpollManager::ToString(removeResult));
  }

  for (const auto& process : processes)
  {
    removeResult = manager.RemoveFd(process.first);
    if (removeResult != EpollManager::Result::kOk)
    {
      Logger::Log(LogLevel::WARNING, "Server <{}>: failed to remove cgi socket <{}> from epoll: {}", socket_,
                  process.first, EpollManager::ToString(removeResult));
    }
  }

  removeResult = manager.RemoveFd(connectionFd);
  if (removeResult != EpollManager::Result::kOk)
  {
    Logger::Log(LogLevel::WARNING, "Server <{}>: failed to remove client <{}> from epoll: {}", socket_, connectionFd,
                EpollManager::ToString(removeResult));
  }
  connectionRegistry_.EraseConnection(connectionFd);
}
