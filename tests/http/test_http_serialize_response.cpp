#include <string>

#include "catch_amalgamated.hpp"
#include "http/HTTPResponse.hpp"
#include "http/HTTPUtils.hpp"

static bool contains(const std::string& str, const std::string& substr)
{
  return str.find(substr) != std::string::npos;
}

TEST_CASE("SerializeResponse basic 200 kOk with body", "SerializeResponse")
{
  HTTPResponse response;
  response.SetStatus(HTTP::Status::kOk);
  response.SetHeader("Content-Type", "text/plain");
  response.SetBody("Success\n");  // 8 bytes

  const std::string wire = HTTP::wire::SerializeResponse(response);

  REQUIRE(contains(wire, "HTTP/1.1 200 OK\r\n"));
  REQUIRE(contains(wire, "\r\n\r\n"));  // header/body separator
  REQUIRE(contains(wire, "Content-Length: 8\r\n"));

  // content-type may be stored lowercase depending on your impl
  const bool ok = contains(wire, "content-type: text/plain\r\n") || contains(wire, "Content-Type: text/plain\r\n");

  INFO("wire=\n" << wire);
  REQUIRE(ok);

  // Body should appear after the blank line
  REQUIRE(wire.rfind("Success\n") != std::string::npos);
}

TEST_CASE("SerializeResponse overwrites user Content-Length", "SerializeResponse")
{
  HTTPResponse response;
  response.SetStatus(HTTP::Status::kOk);
  response.SetHeader("Content-Type", "text/plain");
  response.SetHeader("Content-Length", "999");
  response.SetBody("Success\n");

  const std::string wire = HTTP::wire::SerializeResponse(response);

  INFO("wire=\n" << wire);
  REQUIRE(contains(wire, "Content-Length: 8\r\n"));
  REQUIRE_FALSE(contains(wire, "Content-Length: 999\r\n"));
}

TEST_CASE("SerializeResponse ignore Transfer-Encoding", "SerializeResponse")
{
  HTTPResponse response;
  response.SetStatus(HTTP::Status::kOk);
  response.SetHeader("Content-Type", "text/plain");
  response.SetHeader("Transfer-Encoding", "chunked");
  response.SetBody("Success\n");

  const std::string wire = HTTP::wire::SerializeResponse(response);

  INFO("wire=\n" << wire);
  REQUIRE(contains(wire, "Content-Length: 8\r\n"));
  REQUIRE_FALSE(contains(wire, "Transfer-Encoding: chunked\r\n"));
}

TEST_CASE("SerializeResponse doesn't force Connection close unless explicitly set", "SerializeResponse")
{
  HTTPResponse response;
  response.SetStatus(HTTP::Status::kOk);
  response.SetBody("x");

  const std::string wire = HTTP::wire::SerializeResponse(response);

  INFO("wire=\n" << wire);
  REQUIRE(contains(wire, "Content-Length: 1\r\n"));
  REQUIRE_FALSE(contains(wire, "Connection: close\r\n"));
}

TEST_CASE("SerializeResponse preserves explicit Connection: close", "SerializeResponse")
{
  HTTPResponse response;
  response.SetStatus(HTTP::Status::kOk);
  response.SetHeader("Connection", "close");
  response.SetBody("x");

  const std::string wire = HTTP::wire::SerializeResponse(response);
  const bool close = contains(wire, "Connection: close\r\n") || contains(wire, "connection: close\r\n");

  INFO("wire=\n" << wire);
  REQUIRE(close);
}

TEST_CASE("SerializeResponse empty body: content-length: 0", "SerializeResponse")
{
  HTTPResponse response;
  response.SetStatus(HTTP::Status::kOk);
  response.SetBody("");

  const std::string wire = HTTP::wire::SerializeResponse(response);

  INFO("wire=\n" << wire);
  REQUIRE(contains(wire, "Content-Length: 0\r\n"));
  REQUIRE(contains(wire, "\r\n\r\n"));
  // Ensure it ends right after headers (no extra CRLF required, but body empty)
  REQUIRE(wire.find("\r\n\r\n") != std::string::npos);
}
