#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "catch_amalgamated.hpp"
#include "cgi/CGIProcess.hpp"
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
  using VariantResponse = std::variant<std::monostate, HTTPResponse, cgi::CGIProcess>;

  HTTPRequest MakeGet(std::string_view target)
  {
    HTTPRequest request;
    request.SetMethod(HTTP::Method::kGet);
    REQUIRE(request.SetTarget(target));
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
    std::map<ServerView::IpPort,
             std::map<std::string_view, std::map<std::string_view, RouteView*, ServerRegistry::SizeComparator>>>
        routeViewMap;
    std::map<ServerView::IpPort, std::map<std::string_view, RouteView*, ServerRegistry::SizeComparator>>
        defaultServerRouteViewMap;

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

  HTTPResponse& GetHTTPResponse(VariantResponse& variantResponse)
  {
    REQUIRE(std::holds_alternative<HTTPResponse>(variantResponse));
    return std::get<HTTPResponse>(variantResponse);
  }

  std::filesystem::path GetWwwRoot(void)
  {
    return std::filesystem::current_path() / "tests/www";
  }

}  // namespace

TEST_CASE("Router - remainder behavior via DispatchHandler", "[router][remainder]")
{
  Router r;
  RouteView root;
  root.locationPrefix = "/";
  root.root = GetWwwRoot();

  RouteView st;
  st.locationPrefix = "/static";
  st.root = GetWwwRoot();

  SECTION("When path equals locationPrefix, remainder becomes '/'")
  {
    RouterFixture fx({st});
    VariantResponse res = r.Dispatch(MakeGet("/static"), st, fx.registry, fx.ipPort, fx.hostName);
    REQUIRE(static_cast<int>(GetHTTPResponse(res).GetStatusCode()) == 301);
  }

  SECTION("Suffix remainder includes leading slash for non-root prefixes")
  {
    RouterFixture fx({st});
    VariantResponse res = r.Dispatch(MakeGet("/static/a/b"), st, fx.registry, fx.ipPort, fx.hostName);
    REQUIRE(static_cast<int>(GetHTTPResponse(res).GetStatusCode()) == 200);
    REQUIRE(GetHTTPResponse(res).GetBody() == "b\n");
  }

  SECTION("Root prefix '/' produces remainder without leading slash (current behavior)")
  {
    RouterFixture fx({root});
    VariantResponse res = r.Dispatch(MakeGet("/static/a/b"), root, fx.registry, fx.ipPort, fx.hostName);
    REQUIRE(static_cast<int>(GetHTTPResponse(res).GetStatusCode()) == 200);
    REQUIRE(GetHTTPResponse(res).GetBody() == "b\n");
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
    VariantResponse variantResponse{r.Dispatch(MakeGet("/not-here"), rr, fx.registry, fx.ipPort, fx.hostName)};
    HTTPResponse& res = GetHTTPResponse(variantResponse);
    REQUIRE(static_cast<int>(res.GetStatusCode()) == 302);
    REQUIRE(res.GetFirstHeaderValueOf("Location") == "/");
  }

  SECTION("return 3xx with empty target => 302")
  {
    RouteView rr = root;
    rr.locationPrefix = "/bad";
    rr.returnRule = RouteView::ReturnRule{302, ""};

    RouterFixture fx({rr});
    VariantResponse variantResponse{r.Dispatch(MakeGet("/bad"), rr, fx.registry, fx.ipPort, fx.hostName)};
    HTTPResponse& res = GetHTTPResponse(variantResponse);
    REQUIRE(static_cast<int>(res.GetStatusCode()) == 302);
  }

  SECTION("return 404 uses MakeError body (not empty)")
  {
    RouteView rr = root;
    rr.locationPrefix = "/missing";
    rr.returnRule = RouteView::ReturnRule{404, ""};

    RouterFixture fx({rr});
    VariantResponse variantResponse{r.Dispatch(MakeGet("/missing"), rr, fx.registry, fx.ipPort, fx.hostName)};
    HTTPResponse& res = GetHTTPResponse(variantResponse);
    REQUIRE(static_cast<int>(res.GetStatusCode()) == 404);
    REQUIRE(res.GetBody().find("Not Found") != std::string::npos);
  }
}

TEST_CASE("Router - error_page behavior", "[router][error_page]")
{
  Router r;
  RouteView root;
  root.locationPrefix = "/";
  root.root = GetWwwRoot();

  SECTION("Internal error_page replaces body but preserves original status")
  {
    root.errorPages.clear();
    root.errorPages[404] = "/err.html";
    RouterFixture fx({root});
    VariantResponse variantResponse{r.Dispatch(MakeGet("/missing"), root, fx.registry, fx.ipPort, fx.hostName)};
    HTTPResponse& res = GetHTTPResponse(variantResponse);
    REQUIRE(static_cast<int>(res.GetStatusCode()) == 404);
    REQUIRE(res.GetBody() == "404_ERRORPAGE\n");
  }

  SECTION("If error page points to same path, do not loop")
  {
    root.errorPages.clear();
    root.errorPages[404] = "/missing";
    RouterFixture fx({root});
    VariantResponse variantResponse{r.Dispatch(MakeGet("/missing"), root, fx.registry, fx.ipPort, fx.hostName)};
    HTTPResponse& res = GetHTTPResponse(variantResponse);
    REQUIRE(static_cast<int>(res.GetStatusCode()) == 404);
    REQUIRE(res.GetBody().find("Not Found") != std::string::npos);
  }

  SECTION("External error_page redirects with 302 and Location")
  {
    root.errorPages.clear();
    root.errorPages[404] = "https://example.com/404.html";
    RouterFixture fx({root});
    VariantResponse variantResponse{r.Dispatch(MakeGet("/missing"), root, fx.registry, fx.ipPort, fx.hostName)};
    HTTPResponse& res = GetHTTPResponse(variantResponse);
    REQUIRE(static_cast<int>(res.GetStatusCode()) == 302);
    REQUIRE(res.GetFirstHeaderValueOf("Location") == "https://example.com/404.html");
  }

  SECTION("error_page applies to return-generated errors too")
  {
    RouteView rootRoute;
    rootRoute.locationPrefix = "/";
    rootRoute.root = GetWwwRoot();

    RouteView rr;
    rr.root = GetWwwRoot();
    rr.locationPrefix = "/ret404";
    rr.returnRule = RouteView::ReturnRule{404, ""};
    rr.errorPages.clear();
    rr.errorPages[404] = "/err.html";

    RouterFixture fx({rootRoute, rr});
    VariantResponse variantResponse{r.Dispatch(MakeGet("/ret404"), rr, fx.registry, fx.ipPort, fx.hostName)};
    HTTPResponse& res = GetHTTPResponse(variantResponse);
    REQUIRE(static_cast<int>(res.GetStatusCode()) == 404);
    REQUIRE(res.GetBody() == "404_ERRORPAGE\n");
  }
}

TEST_CASE("Router - dispatch uses supplied matched route", "[router][dispatch]")
{
  Router r;
  RouteView b;
  b.root = GetWwwRoot();
  b.locationPrefix = "/static/images";

  RouterFixture fx({b});
  HTTPRequest request;
  request.SetMethod("GET");
  REQUIRE(request.SetTarget("/static/images/logo.png"));

  VariantResponse variantResponse{r.Dispatch(request, b, fx.registry, fx.ipPort, fx.hostName)};
  HTTPResponse& res = GetHTTPResponse(variantResponse);
  REQUIRE(static_cast<int>(res.GetStatusCode()) == 200);
}
