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

#include "catch_amalgamated.hpp"
#include "cgi/CGI.hpp"

namespace
{
  namespace FileSystem = std::filesystem;
  using namespace cgi;

  Path GetRoot(void)
  {
    try
    {
      return {FileSystem::current_path() / "tests/cgi-bin"};
    }
    catch (...)
    {
    }
    FAIL("Failed to retrieve root directory");
  }
}  // namespace

TEST_CASE("SetupCGIRoute - Script only", "[cgi][Helper-CGI]")
{
  Path root{GetRoot()};
  Path target{"/cgi-bin/run.cgi"};
  Path script_path = root / "run.cgi";

  auto cgi_route = cgi::SetupCGIRoute(target, root, "/cgi-bin");

  REQUIRE(cgi_route.HasValue());
  REQUIRE(cgi_route.GetValue().script_ == script_path);
  REQUIRE(cgi_route.GetValue().resource_ == "");
  REQUIRE(cgi_route.GetValue().full_resource_ == "");
}

TEST_CASE("SetupCGIRoute - Script /w resource", "[cgi][Helper-CGI]")
{
  Path root{GetRoot()};
  Path target{"/cgi-bin/run.cgi/web/page/index.html"};
  Path script_path = root / "run.cgi";

  auto cgi_route = cgi::SetupCGIRoute(target, root, "/cgi-bin");

  REQUIRE(cgi_route.HasValue());
  REQUIRE(cgi_route.GetValue().script_ == script_path);
  REQUIRE(cgi_route.GetValue().resource_ == "/web/page/index.html");
  REQUIRE(cgi_route.GetValue().full_resource_ == "");
}

TEST_CASE("SetupCGIRoute - Script soft link", "[cgi][Helper-CGI]")
{
  Path root{GetRoot()};
  Path target{"/cgi-bin/exec_me.sh/sym_direct_response"};
  Path script_path = root / "run.cgi";

  auto cgi_route = cgi::SetupCGIRoute(target, root, "/cgi-bin");

  REQUIRE_FALSE(cgi_route.HasValue());
  REQUIRE(cgi_route.GetError() == CGIErrorCode::kScriptIsSymlinkError);
}

TEST_CASE("SetupCGIRoute - Script No Perms", "[cgi][Helper-CGI]")
{
  Path root{GetRoot()};
  Path target{"/cgi-bin/no_exec_perms.sh"};
  Path script_path = root / "run.cgi";

  auto cgi_route = cgi::SetupCGIRoute(target, root, "/cgi-bin");

  REQUIRE_FALSE(cgi_route.HasValue());
  REQUIRE(cgi_route.GetError() == CGIErrorCode::kScriptMissingPermissionsError);
}

TEST_CASE("SetupCGIRoute - Script not found", "[cgi][Helper-CGI]")
{
  Path root{GetRoot()};
  Path target{"/cgi-bin/lalallalalalalala/lalalalal/allalalalalalallaal/llalalala/lalalala.cgi"};
  Path script_path = root / "run.cgi";

  auto cgi_route = cgi::SetupCGIRoute(target, root, "/cgi-bin");

  REQUIRE_FALSE(cgi_route.HasValue());
  REQUIRE(cgi_route.GetError() == CGIErrorCode::kScriptNotFoundError);
}

TEST_CASE("ReplaceScriptRoot - default", "[cgi][Helper-CGI]")
{
  Path root{"/data/www/"};
  std::string_view script{"/cgi-bin/script.cgi"};

  Path result = ReplaceScriptRoot(script, "cgi-bin", root);
  CAPTURE(root, script);
  REQUIRE(result == "/data/www/script.cgi");
}

TEST_CASE("ReplaceScriptRoot - extended root path", "[cgi][Helper-CGI]")
{
  Path root{"/data/www/extra/directory/cgi-bin"};
  std::string_view script{"/cgi-bin/script.cgi"};

  Path result = ReplaceScriptRoot(script, "cgi-bin", root);
  CAPTURE(root, script);
  REQUIRE(result == "/data/www/extra/directory/cgi-bin/script.cgi");
}

TEST_CASE("ReplaceScriptRoot - map onto /", "[cgi][Helper-CGI]")
{
  Path root{"/data/www/cgi-bin"};
  std::string_view script{"/script.cgi"};

  Path result = ReplaceScriptRoot(script, "/", root);
  CAPTURE(root, script);
  REQUIRE(result == "/data/www/cgi-bin/script.cgi");
}

TEST_CASE("ReplaceScriptRoot - mapping too large", "[cgi][Helper-CGI]")
{
  Path root{"/data/www/cgi-bin"};
  std::string_view script{"/cgi-bin/script.cgi"};

  Path result = ReplaceScriptRoot(script, "/data/www/cgi-bin/", root);
  CAPTURE(root, script);
  REQUIRE(result == "");
}

TEST_CASE("ReplaceScriptRoot - partial mapping", "[cgi][Helper-CGI]")
{
  Path root{"/data/www/cgi-bin"};
  std::string_view script{"/cgi-bin/level01/level02/level03/script.cgi"};

  Path result = ReplaceScriptRoot(script, "/cgi-bin/", root);
  CAPTURE(root, script);
  REQUIRE(result == "/data/www/cgi-bin/level01/level02/level03/script.cgi");
}

TEST_CASE("ReplaceScriptRoot - complete mapping", "[cgi][Helper-CGI]")
{
  Path root{"/data/www/cgi-bin"};
  std::string_view script{"/cgi-bin/level01/level02/level03/script.cgi"};

  Path result = ReplaceScriptRoot(script, "/cgi-bin/level01/level02/level03", root);
  CAPTURE(root, script);
  REQUIRE(result == "/data/www/cgi-bin/script.cgi");
}

TEST_CASE("ReplaceScriptRoot - full mapping", "[cgi][Helper-CGI]")
{
  Path root{"/data/www/cgi-bin"};
  std::string_view script{"/cgi-bin/level01/level02/level03/script.cgi"};

  Path result = ReplaceScriptRoot(script, "/cgi-bin/level01/level02/level03/script.cgi", root);
  CAPTURE(root, script);
  REQUIRE(result == "/data/www/cgi-bin/");
}

TEST_CASE("ReplaceScriptRoot - mapping larger than script", "[cgi][Helper-CGI]")
{
  Path root{"/data/www/cgi-bin"};
  std::string_view script{"/cgi-bin/script.cgi"};

  Path result = ReplaceScriptRoot(script, "/cgi-bin/level01/level02/level03/", root);
  CAPTURE(root, script);
  REQUIRE(result == "");
}

TEST_CASE("ReplaceScriptRoot - unmatch mapping", "[cgi][Helper-CGI]")
{
  Path root{"/data/www/cgi-bin"};
  std::string_view script{"/cgi-bin/script.cgi"};

  Path result = ReplaceScriptRoot(script, "/cgi-bin-bin/", root);
  CAPTURE(root, script);
  REQUIRE(result == "");
}

TEST_CASE("ExtractResourcePath - no resource", "[cgi][Helper-CGI]")
{
  std::string_view script{"/data/www/cgi-bin/script.cgi"};
  std::string_view target{"/cgi-bin/script.cgi"};

  Path resource = ExtractResourcePath(script, target);
  CAPTURE(script, target);
  REQUIRE(resource == "");
}

TEST_CASE("ExtractResourcePath - with resource", "[cgi][Helper-CGI]")
{
  std::string_view script{"/data/www/cgi-bin/script.cgi"};
  std::string_view target{"/cgi-bin/script.cgi/book/pages/index.html"};

  Path resource = ExtractResourcePath(script, target);
  CAPTURE(script, target);
  REQUIRE(resource == "/book/pages/index.html");
}

TEST_CASE("ExtractResourcePath - empty script", "[cgi][Helper-CGI]")
{
  std::string_view script{""};
  std::string_view target{"/cgi-bin/script.cgi/book/pages/index.html"};

  Path resource = ExtractResourcePath(script, target);
  CAPTURE(script, target);
  REQUIRE(resource == "");
}

TEST_CASE("ExtractResourcePath - empty target", "[cgi][Helper-CGI]")
{
  std::string_view script{"/data/www/cgi-bin/script.cgi"};
  std::string_view target{""};

  Path resource = ExtractResourcePath(script, target);
  CAPTURE(script, target);
  REQUIRE(resource == "");
}

TEST_CASE("ExtractResourcePath - non-matching script part", "[cgi][Helper-CGI]")
{
  std::string_view script{"/this/is/sparta.cgi"};
  std::string_view target{"/cgi-bin/script.cgi/book/pages/index.html"};

  Path resource = ExtractResourcePath(script, target);
  CAPTURE(script, target);
  REQUIRE(resource == "");
}

TEST_CASE("ExtractResourcePath - script exceeds target", "[cgi][Helper-CGI]")
{
  std::string_view script{"/data/www/cgi-bin/script.cgi/website/pages/paragraphs/index.html"};
  std::string_view target{"/cgi-bin/script.cgi/book/pages/index.html"};

  Path resource = ExtractResourcePath(script, target);
  CAPTURE(script, target);
  REQUIRE(resource == "");
}

TEST_CASE("ExtractResourcePath - target prefix not matching", "[cgi][Helper-CGI]")
{
  std::string_view script{"/data/www/cgi-bin/script.cgi"};
  std::string_view target{"/uuu/cgi-bin/script.cgi/book/pages/index.html"};

  Path resource = ExtractResourcePath(script, target);
  CAPTURE(script, target);
  REQUIRE(resource == "");
}
