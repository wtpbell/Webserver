#include "catch_amalgamated.hpp"
#include "http/HTTPRequest.hpp"

TEST_CASE("Path normalization via request target", "[request][path]")
{
  HTTPRequest request;

  SECTION("Basic paths")
  {
    request.SetTarget("/");
    REQUIRE(request.GetPath() == "/");

    request.SetTarget("/test");
    REQUIRE(request.GetPath() == "/test");

    request.SetTarget("/test/foo");
    REQUIRE(request.GetPath() == "/test/foo");
  }

  SECTION("Dot segments")
  {
    request.SetTarget("/./test");
    REQUIRE(request.GetPath() == "/test");

    request.SetTarget("/test/./foo");
    REQUIRE(request.GetPath() == "/test/foo");

    request.SetTarget("/test/../foo");
    REQUIRE(request.GetPath() == "/foo");

    const std::string previous{request.GetPath()};
    request.SetTarget("/../test");

    REQUIRE(request.GetPath() == previous);
  }

  SECTION("Multiple slashes")
  {
    request.SetTarget("//test");
    REQUIRE(request.GetPath() == "/test");

    request.SetTarget("/test///foo");
    REQUIRE(request.GetPath() == "/test/foo");
  }

  SECTION("Trailing slashes")
  {
    request.SetTarget("/test/");
    REQUIRE(request.GetPath() == "/test/");

    request.SetTarget("/");
    REQUIRE(request.GetPath() == "/");
  }

  SECTION("Path traversal attempts")
  {
    request.SetTarget("/safe/path");
    const std::string previous{request.GetPath()};

    request.SetTarget("/../etc/passwd");
    REQUIRE(request.GetPath() == previous);

    request.SetTarget("/foo/../../bar");
    REQUIRE(request.GetPath() == previous);
  }

  SECTION("Complex cases")
  {
    request.SetTarget("/././test");
    REQUIRE(request.GetPath() == "/test");

    request.SetTarget("/a/b/c/./../../d");
    REQUIRE(request.GetPath() == "/a/d");
  }

  SECTION("Edge cases")
  {
    request.SetTarget("/valid");
    const std::string previous{request.GetPath()};

    request.SetTarget("");
    REQUIRE(request.GetPath() == previous);

    request.SetTarget("test");
    REQUIRE(request.GetPath() == previous);

    request.SetTarget("./test");
    REQUIRE(request.GetPath() == previous);

    request.SetTarget("../test");
    REQUIRE(request.GetPath() == previous);

    request.SetTarget("../../test");
    REQUIRE(request.GetPath() == previous);

    request.SetTarget("/../../../test");
    REQUIRE(request.GetPath() == previous);
  }
}
