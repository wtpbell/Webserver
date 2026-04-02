/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   ServerView.hpp                                     :+:    :+:           */
/*                                                      +:+                   */
/*   By: jstuhrin <jstuhrin@student.codam.nl>          +#+                    */
/*                                                    +#+                     */
/*   Created: 2026/03/27 13:51:00 by jstuhrin       #+#    #+#                */
/*   Updated: 2026/03/27 10:53:00 by jstuhrin       ########   odam.nl        */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERVIEW_HPP
#define SERVERVIEW_HPP

#include <string>
#include <type_traits>
#include <vector>

#include "RouteView.hpp"

struct ServerView
{
    struct IpPort
    {
        std::string ip;
        std::string port;
        bool operator<(const IpPort& other) const
        {
          if (ip != other.ip)
          {
            return ip < other.ip;
          }
          return port < other.port;
        }
    };
    std::vector<std::string> hostNames;
    std::vector<IpPort> ipPortList;
    std::vector<RouteView> routes;
};

static_assert(std::is_nothrow_move_constructible_v<ServerView>);

#endif
