/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ConnectionRegistry.hpp                             :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/04/13 18:26:09 by jboon         #+#    #+#                 */
/*   Updated: 2026/04/27 18:47:42 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTIONREGISTRY_H_
#define CONNECTIONREGISTRY_H_

#include <chrono>
#include <unordered_map>

#include "Connection.hpp"
#include "cgi/CGIProcess.hpp"
#include "io/Socket.hpp"
#include "io/TimerFD.hpp"

class ConnectionRegistry
{
    using steady_clock = std::chrono::steady_clock;
    using steady_point = std::chrono::time_point<steady_clock>;

  public:
    struct CGIProcessData
    {
        cgi::CGIProcess cgiProcess_;
        steady_point timeStamp_;

        CGIProcessData(cgi::CGIProcess&& cgiProcess, steady_point&& timeStamp)
            : cgiProcess_(std::move(cgiProcess)), timeStamp_(timeStamp)
        {
        }
    };
    using CGIProcessIterator = std::map<int, CGIProcessData, std::less<>>::iterator;

    struct ConnectionData
    {
        Connection connection_;
        TimerFD timer_;
        steady_point connectionTimestamp_;
        std::map<int, CGIProcessData, std::less<>> processes_;

        ConnectionData(Socket&& socket, TimerFD&& timer, steady_point&& connectionTimestamp)
            : connection_(socket), timer_(timer), connectionTimestamp_(connectionTimestamp)
        {
        }
    };

    ConnectionRegistry(void);

    ConnectionData* CreateConnection(Socket&& socket);
    bool EmplaceCGIProcess(ConnectionData& connectionData, cgi::CGIProcess&& process);
    ConnectionData* FindConnection(int Fd);
    CGIProcessData* FindCGIProcess(int cgiFd);
    void EraseConnection(int connectionFd);
    CGIProcessIterator EraseCgiProcess(std::map<int, CGIProcessData, std::less<>>& processes, CGIProcessIterator cgiIt);
    void EraseCgiProcess(int cgiFd);
    std::size_t GetConnectionCount(void);

  private:
    std::unordered_map<int, ConnectionData> connections_;
    std::unordered_map<int, int> FdToConnection_;
};

#endif  //  CONNECTIONREGISTRY_H_
