/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   RouteView.hpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: jstuhrin <jstuhrin@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/03/27 13:51:00 by jstuhrin      #+#    #+#                 */
/*   Updated: 2026/04/02 10:10:11 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTEVIEW_HPP
#define ROUTEVIEW_HPP

#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <type_traits>

struct RouteView
{
    enum class MethodMask : unsigned
    {
      kNone = 0,
      kGet = 1u << 0,
      kPost = 1u << 1,
      kDelete = 1u << 2
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
    MethodMask allowedMask = MethodMask::kGet;

    std::optional<ReturnRule> returnRule;
    std::map<std::uint16_t, std::string> errorPages;

    bool cgi = false;
    std::optional<std::map<std::string, std::filesystem::path, std::less<>>> cgiExePaths;
};

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
