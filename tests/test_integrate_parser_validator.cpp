#include <optional>
#include <string_view>

#include "catch_amalgamated.hpp"
#include "http/HTTPParser.hpp"
#include "http/HTTPRequest.hpp"
#include "http/HTTPValidator.hpp"

struct ParsedAndValidated
{
    HTTPParser::ParseResult parse;
    std::optional<ValidationResult> validate;
};

// Integration flow:
// - Parse until headers are done
// - Validate headers
// - If valid, decide body framing and continue parsing
static ParsedAndValidated ParseAndValidate(std::string_view raw)
{
  HTTPParser parser;

  HTTPParser::ParseResult pres = parser.Parse(raw);
  if (pres == HTTPParser::ParseResult::kNeedMoreData || pres == HTTPParser::ParseResult::kError)
    return {pres, std::nullopt};
  if (pres == HTTPParser::ParseResult::kHeadersDone)
  {
    const HTTPRequest& partial = parser.GetRequest();
    const ValidationResult vr = ValidateRequest(partial);

    if (vr != ValidationResult::kOk)
      return {pres, vr};
    if (partial.HasHeader("transfer-encoding"))
      parser.SetChunked();
    else if (auto len = partial.GetContentLength())
      parser.SetContentLength(*len);
    else
      parser.SetNoBody();
    pres = parser.Parse({});
    if (pres != HTTPParser::ParseResult::kDone)
      return {pres, std::nullopt};
  }
  if (pres != HTTPParser::ParseResult::kDone)
    return {pres, std::nullopt};
  HTTPRequest request = parser.TakeRequest();
  return {pres, ValidateRequest(request)};
}

TEST_CASE("Parser + Validator: valid chunked request", "[http][integration]")
{
  auto res = ParseAndValidate(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Transfer-Encoding: chunked\r\n"
      "\r\n"
      "0\r\n"
      "\r\n");

  REQUIRE(res.parse == HTTPParser::ParseResult::kDone);
  REQUIRE(res.validate == ValidationResult::kOk);
}

TEST_CASE("Parser + Validator: invalid HTTP version", "[http][integration]")
{
  auto res = ParseAndValidate(
      "POST / HTTP/1.0\r\n"
      "Host: example.com\r\n"
      "Transfer-Encoding: chunked\r\n"
      "\r\n"
      "0\r\n"
      "\r\n");

  // Parser should fail at start-line level.
  REQUIRE(res.validate == ValidationResult::kVersionNotSupported);
}

TEST_CASE("Parser + Validator: reject CL and chunked together", "[http][integration]")
{
  auto res = ParseAndValidate(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Content-Length: 10\r\n"
      "Transfer-Encoding: chunked\r\n"
      "\r\n"
      "0\r\n"
      "\r\n");

  // Parser reaches end-of-headers; validator rejects the header combination.
  REQUIRE(res.parse == HTTPParser::ParseResult::kHeadersDone);
  REQUIRE(res.validate == ValidationResult::kBadRequest);
}

TEST_CASE("Parser+Validator: TE split across lines rejected before body parsing", "[http][integration]")
{
  auto res = ParseAndValidate(
      "POST / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Transfer-Encoding: gzip\r\n"
      "Transfer-Encoding: chunked\r\n"
      "\r\n"
      "0\r\n"
      "\r\n");

  REQUIRE(res.parse == HTTPParser::ParseResult::kHeadersDone);
  REQUIRE(res.validate == ValidationResult::kBadRequest);
}
