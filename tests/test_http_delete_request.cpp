#include <filesystem>
#include <fstream>

#include "catch_amalgamated.hpp"
#include "config/RouteView.hpp"
#include "http/HTTPRequest.hpp"
#include "http/HTTPResponse.hpp"
#include "router/RequestHandler.hpp"

namespace fs = std::filesystem;

static void SetupRoute(RouteView& route)
{
  route.root = "./tests/test_files";
  route.index = "index.html";
  route.autoindex = false;
  route.allowedMask = RouteView::MethodMask::kGet;
}

static void SetupDeleteRequest(HTTPRequest& request, const std::string& target)
{
  request.SetMethod("DELETE");
  request.SetTarget(target);
  request.SetVersion("HTTP/1.1");
  request.AddHeader("Host", "localhost");
}

static fs::path MakeDeleteSandbox(const char* name)
{
  const fs::path base = fs::path("./tests/test_files/delete_sandbox") / name;
  std::error_code ec;
  fs::remove_all(base, ec);
  fs::create_directories(base, ec);
  return base;
}

TEST_CASE("RequestHandler - DELETE not allowed => 405", "[delete][methods]")
{
  HTTPRequest request;
  SetupDeleteRequest(request, "/dir/a.txt");

  RouteView route;
  SetupRoute(route);

  HTTPResponse res = request_handler::HandleMethods(request, route, request.GetPath());

  REQUIRE(res.GetStatusCode() == 405);
  REQUIRE(res.GetFirstHeaderValueOf("Allow") == "GET");
}

TEST_CASE("RequestHandler - DELETE allowed => 204", "[delete][methods]")
{
  fs::path base = MakeDeleteSandbox("delete");
  RouteView route;
  SetupRoute(route);
  route.root = base.string();
  route.allowedMask = RouteView::MethodMask::kGet | RouteView::MethodMask::kDelete;
  std::ofstream(base / "delete.txt") << "delete";
  REQUIRE(fs::exists(base / "delete.txt"));

  HTTPRequest request;
  SetupDeleteRequest(request, "/delete.txt");

  HTTPResponse res = request_handler::HandleMethods(request, route, request.GetPath());

  REQUIRE(res.GetStatusCode() == 204);
  REQUIRE(!fs::exists(base / "delete.txt"));
}

TEST_CASE("RequestHandler - DELETE non-existent file", "[delete][non-existent]")
{
  HTTPRequest request;
  SetupDeleteRequest(request, "/non_existent_file");

  RouteView route;
  SetupRoute(route);
  route.allowedMask = RouteView::MethodMask::kDelete;

  HTTPResponse res = request_handler::HandleMethods(request, route, /*remainder=*/request.GetPath());

  REQUIRE(res.GetStatusCode() == 404);
}

TEST_CASE("RequestHandler - DELETE with trailing slash => 400", "[delete][methods]")
{
  fs::path base = MakeDeleteSandbox("trailing slash");
  RouteView route;
  SetupRoute(route);
  route.root = base.string();
  route.allowedMask = RouteView::MethodMask::kGet | RouteView::MethodMask::kDelete;
  std::ofstream(base / "slash.txt") << "delete";

  HTTPRequest request;
  SetupDeleteRequest(request, "/slash.txt/");

  HTTPResponse res = request_handler::HandleMethods(request, route, request.GetPath());

  REQUIRE(res.GetStatusCode() == 400);
  REQUIRE(!fs::exists(base / "delete.txt"));
}

TEST_CASE("RequestHandler - DELETE / => 403", "[delete][root]")
{
  const fs::path base = MakeDeleteSandbox("delete_root");

  RouteView route;
  SetupRoute(route);
  route.root = base.string();
  route.allowedMask = RouteView::MethodMask::kGet | RouteView::MethodMask::kDelete;

  HTTPRequest request;
  SetupDeleteRequest(request, "/");

  HTTPResponse res = request_handler::HandleMethods(request, route, request.GetPath());

  REQUIRE(res.GetStatusCode() == 403);
}

TEST_CASE("RequestHandler - DELETE directory => 403", "[delete][dir]")
{
  const fs::path base = MakeDeleteSandbox("delete_dir");
  fs::create_directories(base / "dir");

  RouteView route;
  SetupRoute(route);
  route.root = base.string();
  route.allowedMask = RouteView::MethodMask::kGet | RouteView::MethodMask::kDelete;

  HTTPRequest request;
  SetupDeleteRequest(request, "/dir");

  HTTPResponse res = request_handler::HandleMethods(request, route, request.GetPath());

  REQUIRE(res.GetStatusCode() == 403);
}

TEST_CASE("RequestHandler - DELETE path traversal => 403", "[delete][path-traversal]")
{
  const fs::path base = MakeDeleteSandbox("delete_dir");
  fs::create_directories(base / "dir");

  RouteView route;
  SetupRoute(route);
  route.root = base.string();
  route.allowedMask = RouteView::MethodMask::kGet | RouteView::MethodMask::kDelete;

  HTTPRequest request;
  SetupDeleteRequest(request, "../../dir");

  HTTPResponse res = request_handler::HandleMethods(request, route, request.GetPath());

  REQUIRE(res.GetStatusCode() == 403);
}

TEST_CASE("RequestHandler - DELETE with alias removes file under alias base", "[delete][alias]")
{
  const fs::path aliasBase = MakeDeleteSandbox("alias_delete");
  std::ofstream(aliasBase / "dead.txt") << "bye";
  REQUIRE(fs::exists(aliasBase / "dead.txt"));

  HTTPRequest request;
  SetupDeleteRequest(request, "/static/dead.txt");

  RouteView route;
  SetupRoute(route);
  route.alias = aliasBase;
  route.allowedMask = RouteView::MethodMask::kGet | RouteView::MethodMask::kDelete;

  HTTPResponse res = request_handler::HandleMethods(request, route, "/dead.txt");  // remainder

  REQUIRE(res.GetStatusCode() == 204);
  REQUIRE_FALSE(fs::exists(aliasBase / "dead.txt"));
}

TEST_CASE("RequestHandler - DELETE alias blocks traversal escape", "[delete][alias][security]")
{
  const fs::path aliasBase = MakeDeleteSandbox("alias_delete_escape");

  HTTPRequest request;
  SetupDeleteRequest(request, "/static/../secret.txt");

  RouteView route;
  SetupRoute(route);
  route.alias = aliasBase;
  route.allowedMask = RouteView::MethodMask::kGet | RouteView::MethodMask::kDelete;

  HTTPResponse res = request_handler::HandleMethods(request, route, "/../secret.txt");  // remainder

  REQUIRE(res.GetStatusCode() == 403);
}
