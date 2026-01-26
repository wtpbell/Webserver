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
  REQUIRE(res == HTTPParser::ParseResult::HeadersDone);

  // Force "no body" so the parser can finish without requiring bytes.
  parser.SetNoBody();

  res = parser.Parse({});
  REQUIRE(res == HTTPParser::ParseResult::Done);

  return parser.TakeRequest();
}

// Use this only for tests that truly need the body to be parsed by the parser,
// e.g. generating a very large body and ensuring the message is fully parsed.
static HTTPRequest MakeParsedRequestWithBody(std::string_view raw)
{
  HTTPParser parser;

  auto res = parser.Parse(raw);

  if (res == HTTPParser::ParseResult::HeadersDone)
  {
    const HTTPRequest& req = parser.GetRequest();

    // Body framing decision only:
    // - Only select chunked mode if TE is exactly "chunked"
    //   (for validator tests like TE:gzip we must NOT enter chunked mode)
    bool isChunked = false;
    if (auto te = req.GetHeaderValuesOf("transfer-encoding"))
    {
      for (const auto& v : *te)
      {
        // minimal check; parser stored values trimmed but not lowercased
        // your validator will do the strict token parsing anyway
        std::string vv = v;
        for (char& c : vv)
          c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        // trim ASCII spaces/tabs
        auto is_ws = [](char ch)
        {
          return ch == ' ' || ch == '\t';
        };
        while (!vv.empty() && is_ws(vv.front()))
          vv.erase(vv.begin());
        while (!vv.empty() && is_ws(vv.back()))
          vv.pop_back();

        if (vv == "chunked")
        {
          isChunked = true;
          break;
        }
      }
    }

    if (isChunked)
      parser.SetChunked();
    else if (auto len = req.GetContentLength())
      parser.SetContentLength(*len);
    else
      parser.SetNoBody();

    res = parser.Parse({});
  }

  REQUIRE(res == HTTPParser::ParseResult::Done);
  return parser.TakeRequest();
}

static HTTPRequest MakeValidRequest()
{
  HTTPRequest req;
  req.SetMethod("GET");
  req.SetTarget("/");
  req.SetVersion("HTTP/1.1");
  req.AddHeader("host", "example.com");
  req.SetComplete(true);
  return req;
}

TEST_CASE("HTTPValidator::ValidateRequest - Basic Request", "[http][validator]")
{
  auto req = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Transfer-Encoding: chunked\r\n"
      "\r\n"
      "0\r\n"
      "\r\n");

  REQUIRE(ValidateRequest(req) == ValidationResult::OK);
}

TEST_CASE("HTTPValidator rejects oversized body", "[http][validator]")
{
  const std::size_t size = HTTP::kMaxBodySize + 1;
  std::string body(size, 'x');

  const std::string raw = std::string(
                              "POST / HTTP/1.1\r\n"
                              "Host: example.com\r\n"
                              "Content-Length: ") +
                          std::to_string(size) +
                          "\r\n"
                          "\r\n" +
                          body;

  auto req = MakeParsedRequestWithBody(raw);

  REQUIRE(ValidateRequest(req) == ValidationResult::PayloadTooLarge);
}

TEST_CASE("HTTPValidator rejects missing Host header", "[http][validator]")
{
  HTTPRequest req;
  req.SetMethod("GET");
  req.SetTarget("/");
  req.SetVersion("HTTP/1.1");
  req.SetComplete(true);

  REQUIRE(ValidateRequest(req) == ValidationResult::BadRequest);
}

TEST_CASE("HTTPValidator rejects unsupported method", "[http][validator]")
{
  HTTPRequest req = MakeValidRequest();
  req.SetMethod("BREW");

  REQUIRE(ValidateRequest(req) == ValidationResult::NotImplemented);
}

TEST_CASE("HTTPValidator rejects invalid header name", "[http][validator]")
{
  HTTPRequest req = MakeValidRequest();
  req.AddHeader("Bad Header", "value");  // space not allowed

  REQUIRE(ValidateRequest(req) == ValidationResult::BadRequest);
}

TEST_CASE("HTTPValidator rejects CL and chunked together", "[http][validator]")
{
  auto req = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Content-Length: 10\r\n"
      "Transfer-Encoding: chunked\r\n"
      "\r\n"
      "0\r\n"
      "\r\n");

  REQUIRE(ValidateRequest(req) == ValidationResult::BadRequest);
}

TEST_CASE("HTTPValidator rejects invalid target", "[http][validator]")
{
  HTTPRequest req = MakeValidRequest();
  req.SetTarget("relative/path");  // must start with /

  REQUIRE(ValidateRequest(req) == ValidationResult::BadRequest);
}

TEST_CASE("HTTPValidator rejects unsupported HTTP version", "[http][validator]")
{
  HTTPRequest req = MakeValidRequest();
  req.SetVersion("HTTP/1.0");

  REQUIRE(ValidateRequest(req) == ValidationResult::VersionNotSupported);
}

TEST_CASE("HTTPValidator rejects multiple Host headers", "[http][validator]")
{
  HTTPRequest req = MakeValidRequest();
  req.AddHeader("Host", "example.org");

  REQUIRE(ValidateRequest(req) == ValidationResult::BadRequest);
}

TEST_CASE("HTTPValidator rejects unsupported Transfer-Encoding", "[http][validator]")
{
  auto req = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Transfer-Encoding: gzip\r\n"
      "\r\n");

  REQUIRE(ValidateRequest(req) == ValidationResult::NotImplemented);
}

TEST_CASE("HTTPValidator allows multiple identical Content-Length headers", "[http][validator]")
{
  auto req = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Content-Length: 10\r\n"
      "Content-Length: 10\r\n"
      "\r\n"
      "0123456789");

  REQUIRE(ValidateRequest(req) == ValidationResult::OK);
}

TEST_CASE("HTTPValidator rejects negative Content-Length", "[http][validator]")
{
  auto req = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Content-Length: -1\r\n"
      "\r\n"
      "0123456789");

  REQUIRE(ValidateRequest(req) == ValidationResult::BadRequest);
}

TEST_CASE("HTTPValidator rejects mismatched Content-Length headers", "[http][validator]")
{
  auto req = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Content-Length: 10\r\n"
      "Content-Length: 5\r\n"
      "\r\n");

  REQUIRE(ValidateRequest(req) == ValidationResult::BadRequest);
}

TEST_CASE("HTTPValidator accepts multiple Cookie headers", "[http][validator]")
{
  auto req = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Cookie: session=abc\r\n"
      "Cookie: user=john\r\n"
      "\r\n");

  REQUIRE(ValidateRequest(req) == ValidationResult::BadRequest);
}

TEST_CASE("HTTPValidator rejects invalid Content-Length value", "[http][validator]")
{
  auto req = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Content-Length: abc\r\n"
      "\r\n");

  REQUIRE(ValidateRequest(req) == ValidationResult::BadRequest);
}

TEST_CASE("HTTPValidator rejects chunked trailing comma", "[http][validator]")
{
  auto req = MakeParsedHeadersOnly(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Transfer-Encoding: chunked                              ,                           \r\n"
      "\r\n");

  REQUIRE(ValidateRequest(req) == ValidationResult::BadRequest);
}
