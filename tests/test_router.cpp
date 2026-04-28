#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "catch_amalgamated.hpp"
#include "config/ServerRegistry.hpp"
#include "config/ServerView.hpp"
#include "http/HTTPRequest.hpp"
#include "http/HTTPResponse.hpp"
#include "http/HTTPStatus.hpp"
#include "http/ResponseFactory.hpp"
#include "router/RequestHandler.hpp"
#include "router/Router.hpp"


namespace
{

  HTTPRequest MakeGet(std::string_view target)
  {
    HTTPRequest request;
    request.SetMethod(HTTP::Method::kGet);
    REQUIRE(request.SetTarget(target));
    request.SetComplete(true);
    return request;
  }

  static ServerRegistry MakeRegistry(const ServerView::IpPort& ipPort, std::string hostName,
                                     std::vector<RouteView> routes)
  {
    std::vector<ServerView> serverViews(1);
    serverViews[0].ipPort = ipPort;
    serverViews[0].hostNames.push_back(std::move(hostName));
    serverViews[0].routes = std::move(routes);

    std::map<ServerView::IpPort, std::vector<ServerView*>> serverViewMap;
    std::map<ServerView::IpPort, std::map<std::string, std::map<std::string, RouteView*>>> routeViewMap;
    std::map<ServerView::IpPort, std::map<std::string, RouteView*>> defaultServerRouteViewMap;

    ServerView& sv = serverViews[0];
    serverViewMap[ipPort].push_back(&sv);

    for (std::string& host : sv.hostNames)
    {
      for (RouteView& route : sv.routes)
      {
        routeViewMap[ipPort][host][route.locationPrefix] = &route;
      }
    }

    for (RouteView& route : sv.routes)
    {
      defaultServerRouteViewMap[ipPort][route.locationPrefix] = &route;
    }

    return ServerRegistry(std::move(serverViews), std::move(serverViewMap), std::move(routeViewMap),
                          std::move(defaultServerRouteViewMap));
  }

  struct RouterFixture
  {
      ServerView::IpPort ipPort;
      std::string hostName;
      ServerRegistry registry;

      explicit RouterFixture(std::vector<RouteView> routes)
          : ipPort{"127.0.0.1", "8080"}, hostName("a.com"), registry(MakeRegistry(ipPort, hostName, std::move(routes)))
      {
      }
  };

}  // namespace

TEST_CASE("Router - remainder behavior via DispatchHandler", "[router][remainder]")
{
  Router r;

  RouteView root;
  root.locationPrefix = "/";

  RouteView st;
  st.locationPrefix = "/static";

  SECTION("When path equals locationPrefix, remainder becomes '/'")
  {
    RouterFixture fx({st});

    r.SetDispatchHookForTest(
        [](const HTTPRequest& request, const RouteView& route) -> HTTPResponse
        {
          REQUIRE(route.locationPrefix == "/static");
          const std::string_view rem = request_handler::ComputeRouteTail(request.GetPath(), route.locationPrefix);
          REQUIRE(rem == "/");
          return HTTP::response::MakeText(HTTP::Status::kOk, "kOk", "text/plain");
        });

    HTTPResponse res = r.Dispatch(MakeGet("/static"), st, fx.registry, fx.ipPort, fx.hostName);
    REQUIRE(static_cast<int>(res.GetStatusCode()) == 200);
  }

  SECTION("Suffix remainder includes leading slash for non-root prefixes")
  {
    RouterFixture fx({st});

    r.SetDispatchHookForTest(
        [](const HTTPRequest& request, const RouteView& route) -> HTTPResponse
        {
          REQUIRE(route.locationPrefix == "/static");
          const std::string_view rem = request_handler::ComputeRouteTail(request.GetPath(), route.locationPrefix);
          REQUIRE(rem == "/a/b");
          return HTTP::response::MakeText(HTTP::Status::kOk, "kOk", "text/plain");
        });

    (void)r.Dispatch(MakeGet("/static/a/b"), st, fx.registry, fx.ipPort, fx.hostName);
  }

  SECTION("Root prefix '/' produces remainder without leading slash (current behavior)")
  {
    RouterFixture fx({root});

    r.SetDispatchHookForTest(
        [](const HTTPRequest& request, const RouteView& route) -> HTTPResponse
        {
          REQUIRE(route.locationPrefix == "/");
          const std::string_view rem = request_handler::ComputeRouteTail(request.GetPath(), route.locationPrefix);
          REQUIRE(rem == "static/a/b");
          return HTTP::response::MakeText(HTTP::Status::kOk, "kOk", "text/plain");
        });

    (void)r.Dispatch(MakeGet("/static/a/b"), root, fx.registry, fx.ipPort, fx.hostName);
  }
}

TEST_CASE("Router - return rules", "[router][return]")
{
  Router r;

  RouteView root;
  root.locationPrefix = "/";

  SECTION("return 302 sets Location")
  {
    RouteView rr = root;
    rr.locationPrefix = "/not-here";
    rr.returnRule = RouteView::ReturnRule{302, "/"};

    RouterFixture fx({rr});
    HTTPResponse res = r.Dispatch(MakeGet("/not-here"), rr, fx.registry, fx.ipPort, fx.hostName);
    REQUIRE(static_cast<int>(res.GetStatusCode()) == 302);
    REQUIRE(res.GetFirstHeaderValueOf("Location") == "/");
  }

  SECTION("return 3xx with empty target => 302")
  {
    RouteView rr = root;
    rr.locationPrefix = "/bad";
    rr.returnRule = RouteView::ReturnRule{302, ""};

    RouterFixture fx({rr});
    HTTPResponse res = r.Dispatch(MakeGet("/bad"), rr, fx.registry, fx.ipPort, fx.hostName);
    REQUIRE(static_cast<int>(res.GetStatusCode()) == 302);
  }

  SECTION("return 404 uses MakeError body (not empty)")
  {
    RouteView rr = root;
    rr.locationPrefix = "/missing";
    rr.returnRule = RouteView::ReturnRule{404, ""};

    RouterFixture fx({rr});
    HTTPResponse res = r.Dispatch(MakeGet("/missing"), rr, fx.registry, fx.ipPort, fx.hostName);
    REQUIRE(static_cast<int>(res.GetStatusCode()) == 404);
    REQUIRE(res.GetBody().find("Not Found") != std::string::npos);
  }
}

TEST_CASE("Router - error_page behavior", "[router][error_page]")
{
  Router r;

  RouteView root;
  root.locationPrefix = "/";

  SECTION("Internal error_page replaces body but preserves original status")
  {
    root.errorPages.clear();
    root.errorPages[404] = "/err.html";
    RouterFixture fx({root});

    r.SetDispatchHookForTest(
        [](const HTTPRequest& request, const RouteView&) -> HTTPResponse
        {
          if (request.GetPath() == "/missing")
            return HTTP::response::MakeError(HTTP::Status::kNotFound);
          if (request.GetPath() == "/err.html")
            return HTTP::response::MakeText(HTTP::Status::kOk, "ERRORPAGE", "text/plain");
          return HTTP::response::MakeText(HTTP::Status::kOk, "kOk", "text/plain");
        });

    HTTPResponse res = r.Dispatch(MakeGet("/missing"), root, fx.registry, fx.ipPort, fx.hostName);
    REQUIRE(static_cast<int>(res.GetStatusCode()) == 404);
    REQUIRE(res.GetBody() == "ERRORPAGE");
  }

  SECTION("If error page points to same path, do not loop")
  {
    root.errorPages.clear();
    root.errorPages[404] = "/missing";
    RouterFixture fx({root});

    r.SetDispatchHookForTest(
        [](const HTTPRequest& request, const RouteView&) -> HTTPResponse
        {
          if (request.GetPath() == "/missing")
            return HTTP::response::MakeError(HTTP::Status::kNotFound);
          return HTTP::response::MakeText(HTTP::Status::kOk, "kOk", "text/plain");
        });

    HTTPResponse res = r.Dispatch(MakeGet("/missing"), root, fx.registry, fx.ipPort, fx.hostName);
    REQUIRE(static_cast<int>(res.GetStatusCode()) == 404);
    REQUIRE(res.GetBody().find("Not Found") != std::string::npos);
  }

  SECTION("If error page itself fails, keep original response")
  {
    root.errorPages.clear();
    root.errorPages[404] = "/missing_err.html";
    RouterFixture fx({root});

    r.SetDispatchHookForTest(
        [](const HTTPRequest& request, const RouteView&) -> HTTPResponse
        {
          if (request.GetPath() == "/missing" || request.GetPath() == "/missing_err.html")
            return HTTP::response::MakeError(HTTP::Status::kNotFound);
          return HTTP::response::MakeText(HTTP::Status::kOk, "kOk", "text/plain");
        });

    HTTPResponse res = r.Dispatch(MakeGet("/missing"), root, fx.registry, fx.ipPort, fx.hostName);
    REQUIRE(static_cast<int>(res.GetStatusCode()) == 404);
    REQUIRE(res.GetBody().find("Not Found") != std::string::npos);
  }

  SECTION("External error_page redirects with 302 and Location")
  {
    root.errorPages.clear();
    root.errorPages[404] = "https://example.com/404.html";
    RouterFixture fx({root});

    r.SetDispatchHookForTest(
        [](const HTTPRequest& request, const RouteView&) -> HTTPResponse
        {
          if (request.GetPath() == "/missing")
            return HTTP::response::MakeError(HTTP::Status::kNotFound);
          return HTTP::response::MakeText(HTTP::Status::kOk, "kOk", "text/plain");
        });

    HTTPResponse res = r.Dispatch(MakeGet("/missing"), root, fx.registry, fx.ipPort, fx.hostName);
    REQUIRE(static_cast<int>(res.GetStatusCode()) == 302);
    REQUIRE(res.GetFirstHeaderValueOf("Location") == "https://example.com/404.html");
  }

  SECTION("error_page applies to return-generated errors too")
  {
    RouteView rootRoute;
    rootRoute.locationPrefix = "/";

    RouteView rr;
    rr.locationPrefix = "/ret404";
    rr.returnRule = RouteView::ReturnRule{404, ""};
    rr.errorPages.clear();
    rr.errorPages[404] = "/err.html";

    RouterFixture fx({rootRoute, rr});

    r.SetDispatchHookForTest(
        [](const HTTPRequest& request, const RouteView&) -> HTTPResponse
        {
          if (request.GetPath() == "/err.html")
            return HTTP::response::MakeText(HTTP::Status::kOk, "ERRORPAGE", "text/plain");
          return HTTP::response::MakeText(HTTP::Status::kOk, "OK", "text/plain");
        });

    HTTPResponse res = r.Dispatch(MakeGet("/ret404"), rr, fx.registry, fx.ipPort, fx.hostName);
    REQUIRE(static_cast<int>(res.GetStatusCode()) == 404);
    REQUIRE(res.GetBody() == "ERRORPAGE");
  }
}

TEST_CASE("Router - dispatch uses supplied matched route", "[router][dispatch]")
{
  Router r;

  RouteView b;
  b.locationPrefix = "/static/images";

  RouterFixture fx({b});

  r.SetDispatchHookForTest(
      [](const HTTPRequest&, const RouteView& route) -> HTTPResponse
      {
        REQUIRE(route.locationPrefix == "/static/images");
        return HTTP::response::MakeText(HTTP::Status::kOk, "ok", "text/plain");
      });

  HTTPRequest request;
  request.SetMethod("GET");
  REQUIRE(request.SetTarget("/static/images/logo.png"));
  request.SetComplete(true);

  HTTPResponse res = r.Dispatch(request, b, fx.registry, fx.ipPort, fx.hostName);

  REQUIRE(static_cast<int>(res.GetStatusCode()) == 200);
}
