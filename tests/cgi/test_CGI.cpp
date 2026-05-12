/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   test_CGI.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/02/17 15:05:08 by jboon         #+#    #+#                 */
/*   Updated: 2026/02/17 15:05:08 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <filesystem>
#include <string_view>

#include "catch_amalgamated.hpp"
#include "cgi/CGI.hpp"
#include "config/RouteView.hpp"

namespace
{
  namespace FileSystem = std::filesystem;
  using namespace cgi;

  Path GetRoot(void)
  {
    try
    {
      return {FileSystem::current_path() / "tests/"};
    }
    catch (...)
    {
    }
    FAIL("Failed to retrieve root directory");
  }

  RouteView CreateRouteView(void)
  {
    RouteView route;
    route.root = GetRoot();
    route.locationPrefix = "/cgi-bin";
    route.cgi = true;
    route.autoindex = false;
    route.allowedMask = RouteView::MethodMask::kGet;
    return route;
  }

}  // namespace

TEST_CASE("SetupCGIRoute - Script only", "[cgi][Helper-CGI]")
{
  std::string_view target{"/cgi-bin/run.cgi"};
  Path scriptPath{GetRoot() / "cgi-bin/run.cgi"};
  auto cgiRoute = cgi::SetupCGIRoute(target, CreateRouteView());

  REQUIRE(cgiRoute.HasValue());
  REQUIRE(cgiRoute.GetValue().script_ == scriptPath);
  REQUIRE(cgiRoute.GetValue().resource_ == "/");
  REQUIRE(cgiRoute.GetValue().fullResource_ == "");
}

TEST_CASE("SetupCGIRoute - Script /w resource", "[cgi][Helper-CGI]")
{
  Path root{GetRoot()};
  std::string_view target{"/cgi-bin/run.cgi/web/page/index.html"};
  Path scriptPath{GetRoot() / "cgi-bin/run.cgi"};

  auto cgiRoute = cgi::SetupCGIRoute(target, CreateRouteView());

  REQUIRE(cgiRoute.HasValue());
  REQUIRE(cgiRoute.GetValue().script_ == scriptPath);
  REQUIRE(cgiRoute.GetValue().resource_ == "/web/page/index.html");
  REQUIRE(cgiRoute.GetValue().fullResource_ == "");
}

TEST_CASE("SetupCGIRoute - Script soft link", "[cgi][Helper-CGI][!mayfail]")
{
  std::string_view target{"/cgi-bin/exec_me.sh/sym_direct_response"};
  auto cgiRoute = cgi::SetupCGIRoute(target, CreateRouteView());

  REQUIRE(cgiRoute.HasValue());
}

TEST_CASE("SetupCGIRoute - Script No Perms", "[cgi][Helper-CGI]")
{
  std::string_view target{"/cgi-bin/no_exec_perms.sh"};
  auto cgiRoute = cgi::SetupCGIRoute(target, CreateRouteView());

  REQUIRE_FALSE(cgiRoute.HasValue());
  REQUIRE(cgiRoute.GetError() == CGIErrorCode::kMissingPermissionsError);
}

TEST_CASE("SetupCGIRoute - Script not found", "[cgi][Helper-CGI]")
{
  std::string_view target{"/cgi-bin/lalallalalalalala/lalalalal/allalalalalalallaal/llalalala/lalalala.cgi"};
  auto cgiRoute = cgi::SetupCGIRoute(target, CreateRouteView());

  REQUIRE_FALSE(cgiRoute.HasValue());
  REQUIRE(cgiRoute.GetError() == CGIErrorCode::kNotFoundError);
}
