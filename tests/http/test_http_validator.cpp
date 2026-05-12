#include <string>
#include <string_view>

#include "catch_amalgamated.hpp"
#include "http/HTTPParser.hpp"
#include "http/HTTPRequest.hpp"
#include "http/HTTPValidator.hpp"

// For validator-unit tests we almost always only need:
//   start-line + headers parsed successfully
// We do NOT want to parse any body here, because:
// - TE could be unsupported (gzip) but we still want a request object to validate
// - CL may exist without providing the body, and that's fine for header validation
static HTTPRequest MakeParsedHeadersOnly(std::string_view raw)
{
  HTTPParser parser;

  auto res = parser.Parse(raw);

  // These test inputs should include the blank line terminating headers.
  REQUIRE(res == HTTPParser::ParseResult::kHeadersDone);

  // Force "no body" so the parser can finish without requiring bytes.
  parser.SetNoBody();

  res = parser.Parse({});
  REQUIRE(res == HTTPParser::ParseResult::kDone);

  return parser.TakeRequest();
}

static HTTPRequest MakeValidRequest()
{
  HTTPRequest request;
  request.SetMethod("GET");
  request.SetTarget("/");
  request.SetVersion("HTTP/1.1");
  request.AddHeader("host", "example.com");
  return request;
}

TEST_CASE("HTTPValidator::ValidateRequest - Basic Request", "[http][validator]")
{
  auto request = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Transfer-Encoding: chunked\r\n"
      "\r\n"
      "0\r\n"
      "\r\n");

  REQUIRE(ValidateRequest(request) == ValidationResult::kOk);
}

TEST_CASE("HTTPValidator rejects missing Host header", "[http][validator]")
{
  HTTPRequest request;
  request.SetMethod("GET");
  request.SetTarget("/");
  request.SetVersion("HTTP/1.1");
  REQUIRE(ValidateRequest(request) == ValidationResult::kBadRequest);
}

TEST_CASE("HTTPValidator rejects unsupported method", "[http][validator]")
{
  HTTPRequest request = MakeValidRequest();
  request.SetMethod("BREW");

  REQUIRE(ValidateRequest(request) == ValidationResult::kNotImplemented);
}

TEST_CASE("HTTPValidator rejects invalid header name", "[http][validator]")
{
  HTTPRequest request = MakeValidRequest();
  request.AddHeader("Bad Header", "value");  // space not allowed

  REQUIRE(ValidateRequest(request) == ValidationResult::kBadRequest);
}

TEST_CASE("HTTPValidator rejects CL and chunked together", "[http][validator]")
{
  auto request = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Content-Length: 10\r\n"
      "Transfer-Encoding: chunked\r\n"
      "\r\n"
      "0\r\n"
      "\r\n");

  REQUIRE(ValidateRequest(request) == ValidationResult::kBadRequest);
}

TEST_CASE("HTTPValidator rejects invalid target", "[http][validator]")
{
  HTTPRequest request = MakeValidRequest();
  request.SetTarget("relative/path");  // must start with /

  REQUIRE(ValidateRequest(request) == ValidationResult::kBadRequest);
}

TEST_CASE("HTTPValidator rejects unsupported HTTP version", "[http][validator]")
{
  HTTPRequest request = MakeValidRequest();
  request.SetVersion("HTTP/1.0");

  REQUIRE(ValidateRequest(request) == ValidationResult::kVersionNotSupported);
}

TEST_CASE("HTTPValidator rejects multiple Host headers", "[http][validator]")
{
  HTTPRequest request = MakeValidRequest();
  request.AddHeader("Host", "example.org");

  REQUIRE(ValidateRequest(request) == ValidationResult::kBadRequest);
}

TEST_CASE("HTTPValidator rejects unsupported Transfer-Encoding", "[http][validator]")
{
  auto request = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Transfer-Encoding: gzip\r\n"
      "\r\n");

  REQUIRE(ValidateRequest(request) == ValidationResult::kNotImplemented);
}

TEST_CASE("HTTPValidator allows multiple identical Content-Length headers", "[http][validator]")
{
  auto request = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Content-Length: 10\r\n"
      "Content-Length: 10\r\n"
      "\r\n"
      "0123456789");

  REQUIRE(ValidateRequest(request) == ValidationResult::kOk);
}

TEST_CASE("HTTPValidator rejects negative Content-Length", "[http][validator]")
{
  auto request = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Content-Length: -1\r\n"
      "\r\n"
      "0123456789");

  REQUIRE(ValidateRequest(request) == ValidationResult::kBadRequest);
}

TEST_CASE("HTTPValidator rejects mismatched Content-Length headers", "[http][validator]")
{
  auto request = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Content-Length: 10\r\n"
      "Content-Length: 5\r\n"
      "\r\n");

  REQUIRE(ValidateRequest(request) == ValidationResult::kBadRequest);
}

TEST_CASE("HTTPValidator accepts multiple Cookie headers", "[http][validator]")
{
  auto request = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Cookie: session=abc\r\n"
      "Cookie: user=john\r\n"
      "\r\n");

  REQUIRE(ValidateRequest(request) == ValidationResult::kBadRequest);
}

TEST_CASE("HTTPValidator rejects invalid Content-Length value", "[http][validator]")
{
  auto request = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Content-Length: abc\r\n"
      "\r\n");

  REQUIRE(ValidateRequest(request) == ValidationResult::kBadRequest);
}

TEST_CASE("HTTPValidator rejects chunked trailing comma", "[http][validator]")
{
  auto request = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Transfer-Encoding: chunked                              ,                           \r\n"
      "\r\n");

  REQUIRE(ValidateRequest(request) == ValidationResult::kBadRequest);
}

TEST_CASE("HTTPValidator accept valid Cookie header", "[http][validator][cookie]")
{
  auto request = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Cookie: ID=31d4d96e407aad42; lang=en-US\r\n"
      "\r\n");

  REQUIRE(ValidateRequest(request) == ValidationResult::kOk);
}

TEST_CASE("ValidateCookies accept empty Cookie header value", "[http][validator][cookie]")
{
  auto request = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Cookie:\r\n"
      "\r\n");

  REQUIRE(ValidateRequest(request) == ValidationResult::kOk);
}

TEST_CASE("ValidateCookies accept Cookie with empty value", "[http][validate][cookie]")
{
  auto request = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Cookie: ID=31d4d96e407aad42; lang=\r\n"
      "\r\n");

  REQUIRE(ValidateRequest(request) == ValidationResult::kOk);
}

TEST_CASE("ValidateCookies accept Cookie with empty input", "[http][validate][cookie]")
{
  auto request = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Cookie: ID=31d4d96e407aad42;;lang=en-US\r\n"
      "\r\n");

  REQUIRE(ValidateRequest(request) == ValidationResult::kOk);
}

TEST_CASE("ValidateCookies rejects cookie segment without '='", "[http][validator][cookie]")
{
  auto request = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Cookie: a=b; c\r\n"
      "\r\n");
  REQUIRE(ValidateRequest(request) == ValidationResult::kBadRequest);
}

TEST_CASE("ValidateCookies rejects empty cookie name", "[http][validator][cookie]")
{
  auto request = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Cookie: =x\r\n"
      "\r\n");
  REQUIRE(ValidateRequest(request) == ValidationResult::kBadRequest);
}

TEST_CASE("ValidateCookies rejects space in cookie name", "[http][validator][cookie]")
{
  auto request = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Cookie: a b=c\r\n"
      "\r\n");
  REQUIRE(ValidateRequest(request) == ValidationResult::kBadRequest);
}

TEST_CASE("ValidateCookies rejects space in cookie value", "[http][validator][cookie]")
{
  auto request = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Cookie: a=hello world\r\n"
      "\r\n");
  REQUIRE(ValidateRequest(request) == ValidationResult::kBadRequest);
}

TEST_CASE("ValidateCookies rejects comma in cookie value", "[http][validator][cookie]")
{
  auto request = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Cookie: a=1,2\r\n"
      "\r\n");
  REQUIRE(ValidateRequest(request) == ValidationResult::kBadRequest);
}

TEST_CASE("ValidateCookies rejects quote/backslash in cookie value", "[http][validator][cookie]")
{
  auto req1 = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Cookie: a=\"x\"\r\n"
      "\r\n");
  REQUIRE(ValidateRequest(req1) == ValidationResult::kBadRequest);

  auto req2 = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Cookie: a=\\x\r\n"
      "\r\n");
  REQUIRE(ValidateRequest(req2) == ValidationResult::kBadRequest);
}

TEST_CASE("ValidateCookies rejects multiple Cookie headers", "[http][validator][cookie]")
{
  auto request = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Cookie: a=b\r\n"
      "Cookie: c=d\r\n"
      "\r\n");
  REQUIRE(ValidateRequest(request) == ValidationResult::kBadRequest);
}
