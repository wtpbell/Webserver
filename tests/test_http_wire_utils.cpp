#include "./http/HTTPUtils.hpp"
#include "catch_amalgamated.hpp"

TEST_CASE("HTTP::wire::URLDecode decodes percent-encoded path characters")
{
  REQUIRE(HTTP::wire::URLDecode("/upload/my%20file.txt") == "/upload/my file.txt");
  REQUIRE(HTTP::wire::URLDecode("%2Fupload") == "/upload");
  REQUIRE(HTTP::wire::URLDecode("/upload/my%20file+v1.txt") == "/upload/my file+v1.txt");
}

TEST_CASE("HTTP::wire::URLDecode keeps plus as literal plus for URL paths")
{
  REQUIRE(HTTP::wire::URLDecode("/upload/my+file.txt") == "/upload/my+file.txt");
  REQUIRE(HTTP::wire::URLDecode("/search/a+b") == "/search/a+b");
}

TEST_CASE("HTTP::wire::URLDecode keeps invalid percent sequences unchanged")
{
  REQUIRE(HTTP::wire::URLDecode("/file%ZZ.txt") == "/file%ZZ.txt");
  REQUIRE(HTTP::wire::URLDecode("/file%2") == "/file%2");
  REQUIRE(HTTP::wire::URLDecode("/file%") == "/file%");
}

TEST_CASE("HTTP::wire::URLEncode encodes unsafe path characters")
{
  REQUIRE(HTTP::wire::URLEncode("/upload/my file.txt") == "/upload/my%20file.txt");
  REQUIRE(HTTP::wire::URLEncode("/upload/my+file.txt") == "/upload/my%2Bfile.txt");
  REQUIRE(HTTP::wire::URLEncode("/upload/my photo+cat.jpg") == "/upload/my%20photo%2Bcat.jpg");
}

TEST_CASE("HTTP::wire::URLEncode keeps URL path separators")
{
  REQUIRE(HTTP::wire::URLEncode("/upload/file.txt") == "/upload/file.txt");
  REQUIRE(HTTP::wire::URLEncode("/dir name/") == "/dir%20name/");
}

TEST_CASE("HTTP::wire::URLEncode and URLDecode round-trip path strings")
{
  const std::string path = "/upload/my photo+cat.jpg";
  REQUIRE(HTTP::wire::URLDecode(HTTP::wire::URLEncode(path)) == path);
}

TEST_CASE("HTTP::wire::GetMimeType returns common content types")
{
  REQUIRE(HTTP::wire::GetMimeType("index.html") == "text/html");
  REQUIRE(HTTP::wire::GetMimeType("style.css") == "text/css");
}
