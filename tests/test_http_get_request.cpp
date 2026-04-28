#include <filesystem>
#include <fstream>
#include <iostream>

#include "catch_amalgamated.hpp"
#include "config/ServerRegistry.hpp"
#include "config/ServerView.hpp"
#include "http/HTTPRequest.hpp"
#include "router/RequestHandler.hpp"
#include "router/Router.hpp"

/*
GET:
    - file exists + readable → 200
    - file missing → 404
    - path is directory and url missing trailing / → 301
    - directory + index exists → 200
    - directory + no index + autoindex on → 200 with listing
    - directory + no index + autoindex off → 403
*/

namespace fs = std::filesystem;

static void EnsureAutoindexFixture()
{
  fs::remove_all("./tests/test_files/dir");
  fs::create_directories("./tests/test_files/dir/sub");
  std::ofstream("./tests/test_files/dir/a.txt") << "hello";
}
static void SetupRoute(RouteView& route)
{
  route.root = "./tests/test_files";
  route.index = "index.html";
  route.autoindex = false;
  route.allowedMask = RouteView::MethodMask::kGet;
}

static void SetupServer(RouteView& server)
{
  server.root = "./tests/test_files";
  server.index = "index.html";
  server.autoindex = false;
  server.allowedMask = RouteView::MethodMask::kGet;
}

static void SetupGetRequest(HTTPRequest& request, const std::string& target)
{
  request.SetMethod("GET");
  request.SetTarget(target);
  request.SetVersion("HTTP/1.1");
  request.AddHeader("Host", "localhost");
}

static fs::path MakeAliasSandbox(const char* name)
{
  const fs::path base = fs::path("./tests/test_files/alias_root") / name;
  std::error_code ec;
  fs::remove_all(base, ec);          // safe cleanup of sandbox only
  fs::create_directories(base, ec);  // ensure it exists
  return base;
}

static void WriteTextFile(const fs::path& p, const std::string& text)
{
  fs::create_directories(p.parent_path());
  std::ofstream ofs(p, std::ios::binary | std::ios::trunc);
  ofs << text;
}

static void CreatePermissionDeniedFile(const fs::path& p)
{
  fs::create_directories(p.parent_path());
  {
    std::ofstream ofs(p);
    ofs << "secret";
  }
  fs::permissions(p, fs::perms::owner_read | fs::perms::group_read | fs::perms::others_read, fs::perm_options::remove);
}

static ServerRegistry MakeRouterRegistry(const ServerView::IpPort& ipPort, const std::string& hostName,
                                         std::vector<RouteView> routes)
{
  std::vector<ServerView> serverViews(1);
  serverViews[0].ipPort = ipPort;
  serverViews[0].hostNames.push_back(hostName);
  serverViews[0].routes = std::move(routes);

  std::map<ServerView::IpPort, std::vector<ServerView*>> serverViewMap;
  std::map<ServerView::IpPort, std::map<std::string_view, std::map<std::string_view, RouteView*, ServerRegistry::SizeComparator>>> routeViewMap;
  std::map<ServerView::IpPort, std::map<std::string_view, RouteView*, ServerRegistry::SizeComparator>> defaultServerRouteViewMap;

  serverViewMap[ipPort].push_back(&serverViews[0]);

  for (std::string& host : serverViews[0].hostNames)
  {
    for (RouteView& route : serverViews[0].routes)
      routeViewMap[ipPort][host].emplace(route.locationPrefix, &route);
  }

  for (RouteView& route : serverViews[0].routes)
    defaultServerRouteViewMap[ipPort].emplace(route.locationPrefix, &route);

  return ServerRegistry(std::move(serverViews), std::move(serverViewMap), std::move(routeViewMap),
                        std::move(defaultServerRouteViewMap));
}

//************************************************** without alias ****************************************************/

TEST_CASE("RequestHandler - GET / uses index", "[HandleGet]")
{
  HTTPRequest request;
  SetupGetRequest(request, "/");

  RouteView route;
  SetupRoute(route);

  HTTPResponse res = request_handler::HandleMethods(request, route);
  REQUIRE(res.GetStatusCode() == 200);
}

TEST_CASE("RequestHandler - GET / serves index when present", "[HandleGet]")
{
  fs::create_directories("./tests/test_files");
  std::ofstream("./tests/test_files/index.html") << "home";

  HTTPRequest request;
  SetupGetRequest(request, "/");

  RouteView route;
  SetupRoute(route);

  HTTPResponse res = request_handler::HandleMethods(request, route);

  REQUIRE(res.GetStatusCode() == 200);
  REQUIRE(res.GetBody().find("home") != std::string::npos);
}

TEST_CASE("RequestHandler - GET non-existent file", "[InspectPath]")
{
  HTTPRequest request;
  SetupGetRequest(request, "/non_existent_file");

  RouteView route;
  SetupRoute(route);

  HTTPResponse res = request_handler::HandleMethods(request, route);

  REQUIRE(res.GetStatusCode() == 404);
}

TEST_CASE("RequestHandler - GET permission denied file", "[ReadFile]")
{
  const fs::path file = "./tests/test_files/permission_denied";
  CreatePermissionDeniedFile(file);

  HTTPRequest request;
  SetupGetRequest(request, "/permission_denied");

  RouteView route;
  SetupRoute(route);

  HTTPResponse res = request_handler::HandleMethods(request, route);
  REQUIRE(res.GetStatusCode() == 403);

  fs::permissions(file, fs::perms::owner_read | fs::perms::owner_write, fs::perm_options::add);
  fs::remove(file);
}

TEST_CASE("RequestHandler - GET directory without trailing slash redirects to slash", "[HandleGet]")
{
  HTTPRequest request;
  SetupGetRequest(request, "/dir");

  RouteView route;
  SetupRoute(route);

  HTTPResponse res = request_handler::HandleMethods(request, route);

  REQUIRE(res.GetStatusCode() == 301);
  REQUIRE(res.GetFirstHeaderValueOf("Location") == "/dir/");
}

TEST_CASE("RequestHandler - GET directory with trailing slash (no index, autoindex off) returns 403",
          "[HandleDirectoryGet]")
{
  EnsureAutoindexFixture();

  HTTPRequest request;
  SetupGetRequest(request, "/dir/");

  RouteView route;
  SetupRoute(route);

  HTTPResponse res = request_handler::HandleMethods(request, route);

  REQUIRE(res.GetStatusCode() == 403);
}

TEST_CASE("RequestHandler - GET autoindex", "[HandleDirectoryGet]")
{
  HTTPRequest request;
  SetupGetRequest(request, "/dir/");

  RouteView route;
  SetupRoute(route);
  route.autoindex = true;

  HTTPResponse res = request_handler::HandleMethods(request, route);

  REQUIRE(res.GetStatusCode() == 200);
}

TEST_CASE("RequestHandler - GET autoindex lists files and dirs", "[HandleDirectoryGet]")
{
  EnsureAutoindexFixture();

  HTTPRequest request;
  SetupGetRequest(request, "/dir/");

  RouteView route;
  SetupRoute(route);
  route.autoindex = true;

  HTTPResponse res = request_handler::HandleMethods(request, route);

  REQUIRE(res.GetStatusCode() == 200);

  const std::string& body = res.GetBody();

  //   std::cout << body << std::endl;
  REQUIRE(body.find("<title>Index of /dir/</title>") != std::string::npos);
  REQUIRE(body.find("<h1>Index of /dir/</h1>") != std::string::npos);

  REQUIRE(body.find("a.txt") != std::string::npos);
  REQUIRE(body.find("href='/dir/a.txt'") != std::string::npos);

  REQUIRE(body.find("sub/") != std::string::npos);
  REQUIRE(body.find("href='/dir/sub/'") != std::string::npos);
}
TEST_CASE("RequestHandler - GET directory with index serves index", "[HandleDirectoryGet]")
{
  EnsureAutoindexFixture();
  std::ofstream("./tests/test_files/dir/index.html") << "IDX";

  HTTPRequest request;
  SetupGetRequest(request, "/dir/");

  RouteView route;
  SetupRoute(route);
  route.autoindex = true;  // doesn’t matter; index should win

  HTTPResponse res = request_handler::HandleMethods(request, route);

  REQUIRE(res.GetStatusCode() == 200);
  REQUIRE(res.GetBody().find("IDX") != std::string::npos);
}

TEST_CASE("RequestHandler - Default route allows GET only - POST => 405", "[HandleMethods]")
{
  std::filesystem::create_directories("./tests/test_files");
  std::ofstream("./tests/test_files/a.txt") << "hi";

  HTTPRequest request;
  SetupGetRequest(request, "/a.txt");
  request.SetMethod("POST");

  RouteView route;
  SetupRoute(route);  // sets root/index/autoindex
  route.root = "./tests/test_files";
  route.allowedMask = RouteView::MethodMask::kGet;  // explicit default

  HTTPResponse res = request_handler::HandleMethods(request, route);

  REQUIRE(res.GetStatusCode() == 405);
  REQUIRE(res.GetFirstHeaderValueOf("Allow") == "GET");
}

TEST_CASE("RequestHandler - blocks symlink escape outside root", "[security][symlink]")
{
  std::error_code ec;

  fs::create_directories("./tests/test_files/root");
  fs::create_directories("./tests/test_files/outside");
  std::ofstream("./tests/test_files/outside/secret.txt") << "nope";

  fs::remove("./tests/test_files/root/link", ec);
  fs::create_symlink("../outside/secret.txt", "./tests/test_files/root/link", ec);

  if (ec)
    SKIP("symlink not supported on this system/CI");

  // Verify symlink actually escapes root
  const fs::path root = fs::weakly_canonical("./tests/test_files/root");
  const fs::path resolved = fs::weakly_canonical("./tests/test_files/root/link");

  bool inside = std::equal(root.begin(), root.end(), resolved.begin());
  REQUIRE_FALSE(inside);

  HTTPRequest request;
  SetupGetRequest(request, "/link");

  RouteView route;
  SetupRoute(route);
  route.root = "./tests/test_files/root";

  HTTPResponse res = request_handler::HandleMethods(request, route);

  REQUIRE(res.GetStatusCode() == 403);
}

TEST_CASE("RequestHandler - allowed_methods enables POST - POST succeeds", "[HandleMethods]")
{
  fs::create_directories("./tests/test_files");
  std::ofstream("./tests/test_files/a.txt") << "hi";

  HTTPRequest request;
  SetupGetRequest(request, "/a.txt");
  request.SetMethod("POST");

  RouteView route;
  SetupRoute(route);
  route.root = "./tests/test_files";
  route.allowedMask = RouteView::MethodMask::kGet | RouteView::MethodMask::kPost;

  HTTPResponse res = request_handler::HandleMethods(request, route);

  REQUIRE(res.GetStatusCode() == 204);  // or assert exact expected POST status
}

TEST_CASE("RequestHandler - Unsupported method => 501", "[HandleMethods]")
{
  HTTPRequest request;
  SetupGetRequest(request, "/a.txt");
  request.SetMethod("PUT");

  RouteView route;
  SetupRoute(route);

  HTTPResponse res = request_handler::HandleMethods(request, route);

  REQUIRE(res.GetStatusCode() == 501);
}

// These tests validate that ResolvePath computes the route remainder internally.

TEST_CASE("RequestHandler - alias maps remainder to alias file", "[alias][handler]")
{
  const fs::path aliasBase = MakeAliasSandbox("_ut_handler_map_file");
  WriteTextFile(aliasBase / "hello.txt", "hi");

  HTTPRequest request;
  SetupGetRequest(request, "/static/hello.txt");

  RouteView route;
  SetupRoute(route);
  route.locationPrefix = "/static";
  route.alias = aliasBase;
  route.allowedMask = RouteView::MethodMask::kGet;

  HTTPResponse res = request_handler::HandleMethods(request, route);

  REQUIRE(res.GetStatusCode() == 200);
  REQUIRE(res.GetBody().find("hi") != std::string::npos);
}

TEST_CASE("RequestHandler - alias directory without trailing slash redirects", "[alias][handler]")
{
  const fs::path aliasBase = MakeAliasSandbox("_ut_handler_dir_redirect");
  fs::create_directories(aliasBase / "dir");

  HTTPRequest request;
  SetupGetRequest(request, "/static/dir");  // no trailing slash

  RouteView route;
  SetupRoute(route);
  route.locationPrefix = "/static";
  route.alias = aliasBase;
  route.allowedMask = RouteView::MethodMask::kGet;

  HTTPResponse res = request_handler::HandleMethods(request, route);

  REQUIRE(res.GetStatusCode() == 301);
  REQUIRE(res.GetFirstHeaderValueOf("Location") == "/static/dir/");
}

TEST_CASE("RequestHandler - alias blocks symlink escape (ResolvePath containment)", "[alias][handler][security]")
{
  const fs::path aliasBase = MakeAliasSandbox("_ut_handler_symlink_escape");
  const fs::path outside = MakeAliasSandbox("_ut_handler_symlink_outside");
  WriteTextFile(outside / "secret.txt", "secret");

  std::error_code ec;
  fs::create_directory_symlink(fs::weakly_canonical(outside, ec), aliasBase / "escape", ec);
  REQUIRE_FALSE(ec);

  HTTPRequest request;
  SetupGetRequest(request, "/static/escape/secret.txt");

  RouteView route;
  SetupRoute(route);
  route.locationPrefix = "/static";
  route.alias = aliasBase;
  route.allowedMask = RouteView::MethodMask::kGet;

  HTTPResponse res = request_handler::HandleMethods(request, route);

  REQUIRE(res.GetStatusCode() == 403);
}

/*************************************************** with alias *******************************************************/

// Internally Dispatch calls, MatchRoute, ComputerRemainder, DispatchHandler
TEST_CASE("Router::Dispatch - alias serves file (end-to-end)", "[alias][router]")
{
  const fs::path aliasBase = MakeAliasSandbox("_ut_router_serves_file");
  WriteTextFile(aliasBase / "hello.txt", "hi");

  RouteView server;
  SetupServer(server);

  RouteView route;
  SetupRoute(route);
  route.locationPrefix = "/static";
  route.alias = aliasBase;

  Router router;

  HTTPRequest request;
  SetupGetRequest(request, "/static/hello.txt");

  const ServerView::IpPort ipPort = {"127.0.0.1", "8080"};
  const std::string hostName = "localhost";
  ServerRegistry registry = MakeRouterRegistry(ipPort, hostName, {route});

  HTTPResponse res = router.Dispatch(request, route, registry, ipPort, hostName);

  REQUIRE(res.GetStatusCode() == 200);
  REQUIRE(res.GetBody().find("hi") != std::string::npos);
}

TEST_CASE("Router::Dispatch - alias dir without slash redirects (end-to-end)", "[alias][router][remainder]")
{
  const fs::path aliasBase = MakeAliasSandbox("_ut_router_dir_redirect");
  fs::create_directories(aliasBase / "dir");

  RouteView server;
  SetupServer(server);

  RouteView route;
  SetupRoute(route);
  route.locationPrefix = "/static";
  route.alias = aliasBase;

  Router router;

  HTTPRequest request;
  SetupGetRequest(request, "/static/dir");  // no trailing slash

  const ServerView::IpPort ipPort = {"127.0.0.1", "8080"};
  const std::string hostName = "localhost";
  ServerRegistry registry = MakeRouterRegistry(ipPort, hostName, {route});

  HTTPResponse res = router.Dispatch(request, route, registry, ipPort, hostName);

  REQUIRE(res.GetStatusCode() == 301);
  REQUIRE(res.GetFirstHeaderValueOf("Location") == "/static/dir/");
}

TEST_CASE("Router::Dispatch - alias blocks traversal escape (end-to-end)", "[alias][router][security]")
{
  const fs::path aliasBase = MakeAliasSandbox("_ut_router_escape");

  RouteView server;
  SetupServer(server);

  RouteView route;
  SetupRoute(route);
  route.locationPrefix = "/static";
  route.alias = aliasBase;

  Router router;

  HTTPRequest request;
  SetupGetRequest(request, "/static/../secret.txt");

  const ServerView::IpPort ipPort = {"127.0.0.1", "8080"};
  const std::string hostName = "localhost";
  ServerRegistry registry = MakeRouterRegistry(ipPort, hostName, {route});

  HTTPResponse res = router.Dispatch(request, route, registry, ipPort, hostName);

  REQUIRE(res.GetStatusCode() == 404);
}

TEST_CASE("Router::Dispatch - normalized .. escapes alias match", "[alias][router][security]")
{
  const fs::path aliasBase = MakeAliasSandbox("_ut_router_escape_norm");

  RouteView server;
  SetupServer(server);

  RouteView route;
  SetupRoute(route);
  route.locationPrefix = "/static";
  route.alias = aliasBase;

  Router router;

  HTTPRequest request;
  SetupGetRequest(request, "/static/../secret.txt");

  const ServerView::IpPort ipPort = {"127.0.0.1", "8080"};
  const std::string hostName = "localhost";
  ServerRegistry registry = MakeRouterRegistry(ipPort, hostName, {route});

  HTTPResponse res = router.Dispatch(request, route, registry, ipPort, hostName);

  REQUIRE(res.GetStatusCode() == 404);
}

TEST_CASE("RequestHandler - GET alias permission denied file => 403", "[alias][handler][security]")
{
  const fs::path aliasBase = MakeAliasSandbox("_ut_alias_perm_denied");
  const fs::path file = aliasBase / "secret.txt";
  CreatePermissionDeniedFile(file);

  HTTPRequest request;
  SetupGetRequest(request, "/static/secret.txt");

  RouteView route;
  SetupRoute(route);
  route.locationPrefix = "/static";
  route.alias = aliasBase;
  route.allowedMask = RouteView::MethodMask::kGet;

  HTTPResponse res = request_handler::HandleMethods(request, route);

  REQUIRE(res.GetStatusCode() == 403);

  fs::permissions(file, fs::perms::owner_read | fs::perms::owner_write, fs::perm_options::add);
}
