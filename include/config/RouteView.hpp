/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   RouteView.hpp                                       :+:    :+:           */
/*                                                      +:+                   */
/*   By: jstuhrin <jstuhrin@student.codam.nl>          +#+                    */
/*                                                    +#+                     */
/*   Created: 2026/03/27 13:51:00 by jstuhrin       #+#    #+#                */
/*   Updated: 2026/03/27 10:53:00 by jstuhrin       ########   odam.nl        */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTEVIEW_HPP
#define ROUTEVIEW_HPP

#include <cstdint>
#include <string>
#include <filesystem>
#include <optional>
#include <map>
#include <type_traits>

struct RouteView
{
  enum class MethodMask : unsigned
  {
    None = 0,
    Get = 1u << 0,
    Post = 1u << 1,
    Delete = 1u << 2,
    All = Get | Post | Delete,
  };

  struct ReturnRule
  {
    std::uint16_t code;
    std::string target;
  };

  std::string locationPrefix = "/";

  std::filesystem::path root = "./www";
  std::optional<std::filesystem::path> alias;
  std::string index = "index.html";

  bool autoindex = false;
  std::size_t clientMaxBody = 1u << 20;
  MethodMask allowedMask = MethodMask::Get;

  std::optional<ReturnRule> returnRule;
  std::map<std::uint16_t, std::string> errorPages;

  bool cgi = false;
  std::optional<std::map<std::string, std::filesystem::path>> cgiExePaths;
};

// Do we need Has() ?

constexpr RouteView::MethodMask operator|(RouteView::MethodMask a, RouteView::MethodMask b)
{
  return static_cast<RouteView::MethodMask>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

constexpr bool Has(RouteView::MethodMask set, RouteView::MethodMask bit)
{
  return (static_cast<unsigned>(set) & static_cast<unsigned>(bit)) != 0;
}

static_assert(std::is_nothrow_move_constructible_v<RouteView>);

#endif