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
#include <vector>
#include <utility>
#include <cstdint>
#include <type_traits>

#include "RouteView.hpp"

struct ServerView
{
  std::vector<std::string> hostNames;
  std::vector<std::pair<std::string, std::uint16_t>> ipPortList;
  std::vector<RouteView> routes;
};

static_assert(std::is_nothrow_move_constructible_v<ServerView>);

#endif