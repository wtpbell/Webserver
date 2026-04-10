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

static void SetupPostRequest(HTTPRequest& request, const std::string& target, std::string body)
{
  request.SetMethod("POST");
  request.SetTarget(target);
  request.SetVersion("HTTP/1.1");
  request.AddHeader("Host", "localhost");
  request.SetBody(std::move(body));
}

static fs::path MakePostSandbox(const char* name)
{
  const fs::path base = fs::path("./tests/test_files/post_sandbox") / name;
  std::error_code ec;
  fs::remove_all(base, ec);
  fs::create_directories(base, ec);
  return base;
}

TEST_CASE("RequestHandler - POST not allowed returns 405", "[post][methods]")
{
  HTTPRequest request;
  SetupPostRequest(request, "/a.txt", "hi");

  RouteView route;
  SetupRoute(route);

  HTTPResponse res = request_handler::HandleMethods(request, route, request.GetPath());

  REQUIRE(res.GetStatusCode() == 405);
  REQUIRE(res.GetFirstHeaderValueOf("Allow") == "GET");
}

TEST_CASE("RequestHandler - POST creates new file => 201", "[post][fs]")
{
  const fs::path base = MakePostSandbox("create_new");
  RouteView route;
  SetupRoute(route);
  route.root = base.string();

  route.allowedMask = RouteView::MethodMask::kPost;

  HTTPRequest request;
  SetupPostRequest(request, "/new.txt", "hello");

  HTTPResponse res = request_handler::HandleMethods(request, route, request.GetPath());

  REQUIRE(res.GetStatusCode() == 201);
  REQUIRE(fs::exists(base / "new.txt"));

  std::ifstream ifs(base / "new.txt", std::ios::binary);
  std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
  REQUIRE(content == "hello");
}

TEST_CASE("RequestHandler - POST / => 400", "[post][root]")
{
  const fs::path base = MakePostSandbox("post_root");

  RouteView route;
  SetupRoute(route);
  route.root = base.string();
  route.allowedMask = RouteView::MethodMask::kGet | RouteView::MethodMask::kPost;

  HTTPRequest request;
  SetupPostRequest(request, "/", "data");

  HTTPResponse res = request_handler::HandleMethods(request, route, request.GetPath());

  REQUIRE(res.GetStatusCode() == 400);
}

TEST_CASE("RequestHandler - POST overwrites existing file => 204", "[post][fs]")
{
  const fs::path base = MakePostSandbox("overwrite");
  fs::create_directories(base);
  std::ofstream(base / "a.txt") << "old";

  RouteView route;
  SetupRoute(route);
  route.root = base.string();
  route.allowedMask = RouteView::MethodMask::kGet | RouteView::MethodMask::kPost;

  HTTPRequest request;
  SetupPostRequest(request, "/a.txt", "new");

  HTTPResponse res = request_handler::HandleMethods(request, route, request.GetPath());

  REQUIRE(res.GetStatusCode() == 204);
  REQUIRE(res.GetBody().empty());

  std::ifstream ifs(base / "a.txt", std::ios::binary);
  std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
  REQUIRE(content == "new");
}

TEST_CASE("RequestHandler - POST with trailing slash => 400", "[post]")
{
  const fs::path base = MakePostSandbox("trailing slash");

  RouteView route;
  SetupRoute(route);
  route.root = base.string();
  route.allowedMask = RouteView::MethodMask::kGet | RouteView::MethodMask::kPost;

  HTTPRequest request;
  SetupPostRequest(request, "/trailing.txt/", "/");

  HTTPResponse res = request_handler::HandleMethods(request, route, request.GetPath());

  REQUIRE(res.GetStatusCode() == 400);
}

TEST_CASE("RequestHandler - POST body too large => 413", "[post][limit]")
{
  const fs::path base = MakePostSandbox("too_large");

  RouteView route;
  SetupRoute(route);
  route.root = base.string();
  route.allowedMask = RouteView::MethodMask::kGet | RouteView::MethodMask::kPost;
  route.clientMaxBody = 3;

  HTTPRequest request;
  SetupPostRequest(request, "/large.txt", "Dont expect me less than 3");

  HTTPResponse res = request_handler::HandleMethods(request, route, request.GetPath());

  REQUIRE(res.GetStatusCode() == 413);
}

TEST_CASE("RequestHandler - POST into missing parent => 404", "[post][fs]")
{
  const fs::path base = MakePostSandbox("missing_parent");

  RouteView route;
  SetupRoute(route);
  route.root = base.string();
  route.allowedMask = RouteView::MethodMask::kGet | RouteView::MethodMask::kPost;
  route.clientMaxBody = 1024;

  HTTPRequest request;
  SetupPostRequest(request, "/no_such_dir/file.txt", "hi");

  HTTPResponse res = request_handler::HandleMethods(request, route, request.GetPath());

  REQUIRE(res.GetStatusCode() == 404);
  REQUIRE_FALSE(fs::exists(base / "no_such_dir" / "file.txt"));
}

TEST_CASE("RequestHandler - POST to existing directory => 400", "[post][fs]")
{
  const fs::path base = MakePostSandbox("post_to_dir");
  fs::create_directories(base / "dir");

  RouteView route;
  SetupRoute(route);
  route.root = base.string();
  route.allowedMask = RouteView::MethodMask::kGet | RouteView::MethodMask::kPost;

  HTTPRequest request;
  SetupPostRequest(request, "/dir", "hi");

  HTTPResponse res = request_handler::HandleMethods(request, route, request.GetPath());

  REQUIRE(res.GetStatusCode() == 400);
}

TEST_CASE("RequestHandler - POST with alias writes under alias base", "[post][alias]")
{
  const fs::path aliasBase = MakePostSandbox("alias_post");
  // aliasBase is where files should land

  HTTPRequest request;
  SetupPostRequest(request, "/static/x.txt", "hi");

  RouteView route;
  SetupRoute(route);
  route.alias = aliasBase;
  route.allowedMask = RouteView::MethodMask::kGet | RouteView::MethodMask::kPost;

  HTTPResponse res = request_handler::HandleMethods(request, route, "/x.txt");  // remainder
  REQUIRE(res.GetStatusCode() == 201);
  REQUIRE(fs::exists(aliasBase / "x.txt"));
}

TEST_CASE("RequestHandler - POST path traversal => 400", "[post][security]")
{
  const fs::path base = MakePostSandbox("post_traversal");

  RouteView route;
  SetupRoute(route);
  route.root = base.string();
  route.allowedMask = RouteView::MethodMask::kPost;

  HTTPRequest request;
  SetupPostRequest(request, "/../escape.txt", "evil");

  HTTPResponse res = request_handler::HandleMethods(request, route, request.GetPath());

  REQUIRE(res.GetStatusCode() == 400);
  REQUIRE_FALSE(fs::exists(base / "escape.txt"));
}

TEST_CASE("RequestHandler - POST alias blocks traversal escape", "[post][alias][security]")
{
  const fs::path aliasBase = MakePostSandbox("alias_post_escape");

  HTTPRequest request;
  SetupPostRequest(request, "/static/../secret.txt", "hi");

  RouteView route;
  SetupRoute(route);
  route.alias = aliasBase;
  route.allowedMask = RouteView::MethodMask::kGet | RouteView::MethodMask::kPost;

  HTTPResponse res = request_handler::HandleMethods(request, route, "/../secret.txt");

  REQUIRE(res.GetStatusCode() == 403);
}

TEST_CASE("RequestHandler - POST empty body creates empty file", "[post][fs]")
{
  const fs::path base = MakePostSandbox("empty_body");

  RouteView route;
  SetupRoute(route);
  route.root = base.string();
  route.allowedMask = RouteView::MethodMask::kPost;

  HTTPRequest request;
  SetupPostRequest(request, "/empty.txt", "");

  HTTPResponse res = request_handler::HandleMethods(request, route, request.GetPath());

  REQUIRE(res.GetStatusCode() == 201);
  REQUIRE(fs::exists(base / "empty.txt"));
  REQUIRE(fs::file_size(base / "empty.txt") == 0);
}
