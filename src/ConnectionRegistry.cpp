/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ConnectionRegistry.cpp                             :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/04/14 14:53:43 by jboon         #+#    #+#                 */
/*   Updated: 2026/05/03 17:20:38 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "ConnectionRegistry.hpp"

#include <cassert>

#include "Logger.hpp"
#include "cgi/CGIProcess.hpp"
#include "io/Socket.hpp"
#include "io/TimerFD.hpp"

ConnectionRegistry::ConnectionRegistry(void)
{
  connections_.reserve(1024);
  FdToConnection_.reserve(1024);
}

ConnectionRegistry::ConnectionData* ConnectionRegistry::CreateConnection(Socket&& socket)
{
  const int interval = 1;
  const int connectionFd = socket.GetFD();

  Expected<TimerFD, int> timer = TimerFD::CreateMonotonicClock(0, interval);
  if (!timer.HasValue())
  {
    Logger::Log(LogLevel::ERROR, "CreateConnection: Failed to create TimerFd for connection");
    return nullptr;
  }

  auto [itFdToConnection, fdInserted] = FdToConnection_.try_emplace(timer->GetFD(), connectionFd);
  if (!fdInserted)
  {
    Logger::Log(LogLevel::ERROR, "CreateConnection: duplicate entry of timerFd in FdToConnection");
    assert(false && "duplicate entry of timerFd in FdToConnection");
    return nullptr;
  }

  auto [it, inserted] =
      connections_.try_emplace(connectionFd, std::move(socket), timer.ExtractValue(), steady_clock::now());
  if (!inserted)
  {
    Logger::Log(LogLevel::ERROR, "CreateConnection: duplicate entry of connectionFd {} in connections", connectionFd);
    assert(false && "duplicate entry of connectionFd in connections");
    FdToConnection_.erase(itFdToConnection);
    return nullptr;
  }

  return &it->second;
}

ConnectionRegistry::ConnectionData* ConnectionRegistry::FindConnection(int Fd)
{
  auto connectionIterator = connections_.find(Fd);
  if (connectionIterator != connections_.end())
  {
    return &connectionIterator->second;
  }
  auto iterator = FdToConnection_.find(Fd);
  if (iterator == FdToConnection_.end())
  {
    return nullptr;
  }
  connectionIterator = connections_.find(iterator->second);
  if (connectionIterator == connections_.end())
  {
    return nullptr;
  }
  return &connectionIterator->second;
}

bool ConnectionRegistry::EmplaceCGIProcess(ConnectionData& connectionData, cgi::CGIProcess&& process)
{
  const int connectionFd = connectionData.connection_.GetSocket().GetFD();
  const int cgiFd = process.GetSocket().GetFD();

  if (!FdToConnection_.try_emplace(cgiFd, connectionFd).second)
  {
    Logger::Log(LogLevel::ERROR,
                "EmplaceCGIProcess: entry (cgiFd: {}) => (connectionFd: {}) already exist in FdToConnection", cgiFd,
                connectionFd);
    assert((connectionData.processes_.count(cgiFd) == 1) && "entry of cgiFd already exist in FdToConnection");
    return false;
  }

  auto& processes = connectionData.processes_;
  if (!processes.try_emplace(cgiFd, std::move(process), steady_clock::now()).second)
  {
    Logger::Log(LogLevel::ERROR,
                "EmplaceCGIProcess: entry of (cgiFd: {}) => (connectionFd: {}) already exist in processes", cgiFd,
                connectionFd);
    assert(false && "entry of cgiFd already exist in processes");
    FdToConnection_.erase(cgiFd);
    return false;
  }

  return true;
}

ConnectionRegistry::CGIProcessData* ConnectionRegistry::FindCGIProcess(const int cgiFd)
{
  ConnectionData* connectionData = FindConnection(cgiFd);
  if (connectionData == nullptr)
  {
    return nullptr;
  }
  auto cgiIterator = connectionData->processes_.find(cgiFd);
  if (cgiIterator == connectionData->processes_.end())
  {
    return nullptr;
  }
  return &cgiIterator->second;
}

void ConnectionRegistry::EraseConnection(int connectionFd)
{
  auto it = connections_.find(connectionFd);
  if (it == connections_.end())
  {
    return;
  }

  ConnectionData& connectionData = it->second;
  FdToConnection_.erase(connectionData.timer_.GetFD());

  const auto& processes = connectionData.processes_;
  for (const auto& process : processes)
  {
    FdToConnection_.erase(process.first);
  }
  connections_.erase(connectionFd);
}

ConnectionRegistry::CGIProcessIterator ConnectionRegistry::EraseCgiProcess(
    std::map<int, CGIProcessData, std::less<>>& processes, CGIProcessIterator cgiIt)
{
  FdToConnection_.erase(cgiIt->first);
  return processes.erase(cgiIt);
}

void ConnectionRegistry::EraseCgiProcess(const int cgiFd)
{
  ConnectionData* connectionData = FindConnection(cgiFd);
  if (connectionData == nullptr)
  {
    Logger::Log(LogLevel::ERROR, "EraseCgiProcess: No matching ConnectioData found for cgiFd {} in the registry",
                cgiFd);
  }
  else
  {
    connectionData->processes_.erase(cgiFd);
  }
  FdToConnection_.erase(cgiFd);
}

std::size_t ConnectionRegistry::GetConnectionCount(void)
{
  return connections_.size();
}
