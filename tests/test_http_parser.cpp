#include "catch_amalgamated.hpp"
#include "http/HTTPParser.hpp"
#include "http/HTTPRequest.hpp"
#include "http/HTTPTypes.hpp"

using ParseRes = HTTPParser::ParseResult;

static std::string MakeHugeHeaderValue(std::size_t minBytes)
{
  return std::string(minBytes, 'a');
}

// Parse like the server’s mechanics (but without validation):
//   1) Parse raw bytes until HeadersDone / Done / Error
//   2) If HeadersDone -> pick a framing mode from headers
//   3) Continue parsing buffered bytes with Parse({})

static ParseRes ParseToCompletion(HTTPParser& parser, std::string_view raw)
{
  ParseRes r = parser.Parse(raw);

  while (r == ParseRes::HeadersDone)
  {
    const HTTPRequest& req = parser.GetRequest();

    if (req.HasHeader("transfer-encoding"))
      parser.SetChunked();
    else if (auto len = req.GetContentLength())
      parser.SetContentLength(*len);
    else
      parser.SetNoBody();

    r = parser.Parse({});
  }

  return r;
}

TEST_CASE("HTTPParser::Parse - Basic Request", "[httpparser]")
{
  HTTPParser parser;
  std::string request =
      "GET /index.html HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "User-Agent: test-client/1.0\r\n"
      "\r\n";

  auto result = ParseToCompletion(parser, request);

  REQUIRE(result == ParseRes::Done);
  REQUIRE(parser.IsComplete() == true);
  REQUIRE(parser.HasError() == false);
}

TEST_CASE("HTTPParser::Parse - Multiple trailing CRLF", "[httpparser]")
{
  HTTPParser parser;
  std::string request =
      "\r\n\r\n\r\n"
      "\r\n"
      "\r\n"
      "\r\n"
      "GET /index.html HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "User-Agent: test-client/1.0\r\n"
      "\r\n\r\n";

  auto result = ParseToCompletion(parser, request);

  REQUIRE(result == ParseRes::Done);
  REQUIRE(parser.IsComplete() == true);
  REQUIRE(parser.HasError() == false);
}

TEST_CASE("HTTPParser::Parse - Multiple Spaces After Colon", "[httpparser]")
{
  HTTPParser parser;
  std::string request =
      "GET /index.html HTTP/1.1\r\n"
      "Host:      example.com\r\n"
      "User-Agent:       test-client/1.0\r\n"
      "\r\n";

  auto result = ParseToCompletion(parser, request);

  REQUIRE(result == ParseRes::Done);
  REQUIRE(parser.IsComplete() == true);
  REQUIRE(parser.HasError() == false);
}

TEST_CASE("HTTPParser::Parse - Header Whitespace Before Colon", "[httpparser]")
{
  HTTPParser parser;
  std::string request =
      "GET / HTTP/1.1\r\n"
      "Host : example.com\r\n"
      "\r\n";

  auto result = ParseToCompletion(parser, request);

  REQUIRE(result == ParseRes::Error);
  REQUIRE(parser.HasError() == true);
}

TEST_CASE("HTTPParser::Parse - Header Without Space After Colon", "[httpparser]")
{
  HTTPParser parser;
  std::string request =
      "GET / HTTP/1.1\r\n"
      "Host:example.com\r\n"
      "\r\n";

  auto result = ParseToCompletion(parser, request);

  REQUIRE(result == ParseRes::Done);
  REQUIRE(parser.HasError() == false);
  REQUIRE(parser.IsComplete() == true);
}

TEST_CASE("HTTPParser::Parse - Header With Tab After Colon", "[httpparser]")
{
  HTTPParser parser;
  std::string request =
      "GET / HTTP/1.1\r\n"
      "Host:\texample.com\r\n"
      "\r\n";

  auto result = ParseToCompletion(parser, request);

  REQUIRE(result == ParseRes::Done);
  REQUIRE(parser.HasError() == false);
}

TEST_CASE("HTTPParser::Parse - Duplicate Headers Allowed", "[httpparser]")
{
  HTTPParser parser;
  std::string request =
      "GET / HTTP/1.1\r\n"
      "X-Test: a\r\n"
      "X-Test: b\r\n"
      "\r\n";

  auto result = ParseToCompletion(parser, request);

  REQUIRE(result == ParseRes::Done);
  REQUIRE(parser.HasError() == false);
}

TEST_CASE("HTTPParser::Parse - Content-Length zero", "[httpparser]")
{
  HTTPParser parser;
  std::string request =
      "POST / HTTP/1.1\r\n"
      "Content-Length: 0\r\n"
      "\r\n";

  auto result = ParseToCompletion(parser, request);

  REQUIRE(result == ParseRes::Done);
  REQUIRE(parser.IsComplete() == true);
  REQUIRE(parser.HasError() == false);
}

TEST_CASE("HTTPParser::Parse - Content-Length Body Incomplete", "[httpparser]")
{
  HTTPParser parser;
  std::string request =
      "POST / HTTP/1.1\r\n"
      "Content-Length: 5\r\n"
      "\r\n"
      "abc";

  auto result = ParseToCompletion(parser, request);

  REQUIRE(result == ParseRes::NeedMoreData);
  REQUIRE(parser.HasError() == false);
  REQUIRE(parser.IsComplete() == false);
}

TEST_CASE("HTTPParser::Parse - Multiple Content-Length Headers", "[httpparser]")
{
  HTTPParser parser;
  std::string request =
      "POST / HTTP/1.1\r\n"
      "Content-Length: 5\r\n"
      "Content-Length: 5\r\n"
      "\r\n"
      "hello";

  auto result = ParseToCompletion(parser, request);

  REQUIRE(result == ParseRes::Done);
  REQUIRE(parser.HasError() == false);
}

TEST_CASE("HTTPParser::Parse - Many Headers Without Body", "[httpparser]")
{
  HTTPParser parser;
  std::string request =
      "GET / HTTP/1.1\r\n"
      "A: 1\r\n"
      "B: 2\r\n"
      "C: 3\r\n"
      "D: 4\r\n"
      "E: 5\r\n"
      "\r\n";

  auto result = ParseToCompletion(parser, request);

  REQUIRE(result == ParseRes::Done);
  REQUIRE(parser.IsComplete() == true);
  REQUIRE(parser.HasError() == false);
}

TEST_CASE("HTTPParser::Parse - Invalid HTTP Version", "[httpparser]")
{
  HTTPParser parser;
  std::string request =
      "GET / HTTP/1.5\r\n"
      "Host: example.com\r\n"
      "\r\n";

  auto result = ParseToCompletion(parser, request);

  REQUIRE(result == ParseRes::Done);
  REQUIRE(parser.HasError() == false);
  REQUIRE(parser.IsComplete() == true);
}

TEST_CASE("HTTPParser::Parse - Allow Multiple Headers", "[httpparser]")
{
  HTTPParser parser;
  std::string request =
      "POST /submit HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "User-Agent: TestAgent/1.0\r\n"
      "Connection: close\r\n"
      "\r\n";

  auto result = ParseToCompletion(parser, request);

  REQUIRE(result == ParseRes::Done);
  REQUIRE(parser.HasError() == false);
  REQUIRE(parser.IsComplete() == true);
}

// RFC 7230, Section 3.2: Header field names are case-insensitive.
TEST_CASE("HTTPParser::Parse - Uppercase Headers Allowed", "[httpparser]")
{
  HTTPParser parser;
  std::string request =
      "POST /submit HTTP/1.1\r\n"
      "HOST: example.com\r\n"
      "CONNECTION: close\r\n"
      "\r\n";

  auto result = ParseToCompletion(parser, request);

  REQUIRE(result == ParseRes::Done);
  REQUIRE(parser.HasError() == false);
  REQUIRE(parser.IsComplete() == true);
}

TEST_CASE("HTTPParser::Parse - Incomplete Request", "[httpparser]")
{
  HTTPParser parser;
  std::string request =
      "GET / HTTP/1.1\r\n"
      "Host: example.com\r\n";

  auto result = ParseToCompletion(parser, request);

  REQUIRE(result == ParseRes::NeedMoreData);
  REQUIRE(parser.HasError() == false);
  REQUIRE(parser.IsComplete() == false);
}

TEST_CASE("HTTPParser::Parse - Empty Query String Allowed", "[httpparser]")
{
  HTTPParser parser;
  std::string request =
      "POST /submit? HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "Connection: close\r\n"
      "\r\n";

  auto result = ParseToCompletion(parser, request);

  REQUIRE(result == ParseRes::Done);
  REQUIRE(parser.HasError() == false);
  REQUIRE(parser.IsComplete() == true);
}

TEST_CASE("HTTPParser::Parse - Invalid Request", "[httpparser]")
{
  HTTPParser parser;
  std::string request = "INVALID REQUEST\r\n\r\n";

  auto result = ParseToCompletion(parser, request);

  REQUIRE(result == ParseRes::Error);
  REQUIRE(parser.HasError() == true);
  REQUIRE(parser.IsComplete() == false);
}

TEST_CASE("HTTPParser::Parse - Chunked Request", "[httpparser]")
{
  HTTPParser parser;
  std::string request = "POST /upload HTTP/1.1\r\n"
  "Transfer-Encoding: chunked\r\n"
      "Transfer-Encoding: chunked\r\n"
      "\r\n"
      "5\r\n"
      "hello\r\n"
      "6\r\n"
      " world\r\n"
      "0\r\n"
      "\r\n";

  auto result = ParseToCompletion(parser, request);

  REQUIRE(result == ParseRes::Done);
  REQUIRE(parser.IsComplete() == true);
  REQUIRE(parser.HasError() == false);
}

TEST_CASE("HTTPParser state/result invariant", "[httpparser][invariant]")
{
  HTTPParser parser;

  auto r = ParseToCompletion(parser, "GET / HTTP/1.1\r\n\r\n");

  if (r == ParseRes::Done)
  {
    REQUIRE(parser.IsComplete());
    REQUIRE_FALSE(parser.HasError());
  }
  else if (r == ParseRes::Error)
  {
    REQUIRE(parser.HasError());
    REQUIRE_FALSE(parser.IsComplete());
  }
  else
  {
    REQUIRE_FALSE(parser.IsComplete());
    REQUIRE_FALSE(parser.HasError());
  }
}

TEST_CASE("HTTPParser::Parse - Multiple Parses", "[httpparser]")
{
  HTTPParser parser;

  std::string part1 = "GET /index.html HTTP/1.1\r\n";
  std::string part2 = "Host: example.com\r\n";
  std::string part3 =
      "Content-Length: 16\r\n"
      "User-Agent: test-client/1.0\r\n"
      "\r\n";
  std::string part4 = "this ";
  std::string part5 = "is ";
  std::string part6 = "the ";
  std::string part7 = "body";

  auto result = parser.Parse(part1);
  REQUIRE(result == ParseRes::NeedMoreData);
  REQUIRE(parser.IsComplete() == false);

  result = parser.Parse(part2);
  REQUIRE(result == ParseRes::NeedMoreData);
  REQUIRE(parser.IsComplete() == false);

  // headers end here -> HeadersDone now
  result = parser.Parse(part3);
  REQUIRE(result == ParseRes::HeadersDone);
  REQUIRE(parser.IsComplete() == false);

  // decide body mode and continue
  const HTTPRequest& req = parser.GetRequest();
  REQUIRE(req.GetContentLength().has_value());
  parser.SetContentLength(*req.GetContentLength());

  result = parser.Parse({});
  REQUIRE(result == ParseRes::NeedMoreData);

  result = parser.Parse(part4);
  REQUIRE(result == ParseRes::NeedMoreData);
  REQUIRE(parser.IsComplete() == false);

  result = parser.Parse(part5);
  REQUIRE(result == ParseRes::NeedMoreData);
  REQUIRE(parser.IsComplete() == false);

  result = parser.Parse(part6);
  REQUIRE(result == ParseRes::NeedMoreData);
  REQUIRE(parser.IsComplete() == false);

  result = parser.Parse(part7);
  REQUIRE(result == ParseRes::Done);
  REQUIRE(parser.IsComplete() == true);
}

TEST_CASE("HTTPParser rejects non-empty trailer after last chunk", "[http][parser]")
{
  HTTPParser parser;
  auto res = ParseToCompletion(parser,
                               "POST / HTTP/1.1\r\n"
                               "Host: example.com\r\n"
                               "Transfer-Encoding: chunked\r\n"
                               "\r\n"
                               "0\r\n"
                               " \r\n");

  REQUIRE(res == HTTPParser::ParseResult::Error);
}
TEST_CASE("HTTPParser rejects headers section larger than kMaxHeaderSize (complete lines)", "[http][parser]")
{
  HTTPParser p;
  std::string huge = MakeHugeHeaderValue(HTTP::kMaxHeaderSize + 64);
  std::string req =
      "GET / HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "X-Big: " +
      huge +
      "\r\n"
      "\r\n";

  HTTPParser::ParseResult r = p.Parse(req);

  REQUIRE(r == HTTPParser::ParseResult::Error);
  REQUIRE(p.HasError());
  REQUIRE(p.GetError() == ValidationResult::PayloadTooLarge);
}

TEST_CASE("HTTPParser rejects headers larger than kMaxHeaderSize even without CRLF", "[http][parser]")
{
  HTTPParser p;
  std::string huge = MakeHugeHeaderValue(HTTP::kMaxHeaderSize + 64);
  std::string partial =
      "GET / HTTP/1.1\r\n"
      "Host: localhost\r\n"
      "X-Big: " +
      huge;  // <-- no "\r\n" yet

  HTTPParser::ParseResult r = p.Parse(partial);

  REQUIRE(r == HTTPParser::ParseResult::Error);
  REQUIRE(p.HasError());
  REQUIRE(p.GetError() == ValidationResult::PayloadTooLarge);
}
