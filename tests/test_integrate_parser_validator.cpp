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
  if (pres == HTTPParser::ParseResult::NeedMoreData || pres == HTTPParser::ParseResult::Error)
    return {pres, std::nullopt};
  if (pres == HTTPParser::ParseResult::HeadersDone)
  {
    const HTTPRequest& partial = parser.GetRequest();
    const ValidationResult vr = ValidateRequest(partial);

    if (vr != ValidationResult::OK)
      return {pres, vr};
    if (partial.HasHeader("transfer-encoding"))
      parser.SetChunked();
    else if (auto len = partial.GetContentLength())
      parser.SetContentLength(*len);
    else
      parser.SetNoBody();
    pres = parser.Parse({});
    if (pres != HTTPParser::ParseResult::Done)
      return {pres, std::nullopt};
  }
  if (pres != HTTPParser::ParseResult::Done)
    return {pres, std::nullopt};
  HTTPRequest req = parser.TakeRequest();
  return {pres, ValidateRequest(req)};
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

  REQUIRE(res.parse == HTTPParser::ParseResult::Done);
  REQUIRE(res.validate == ValidationResult::OK);
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
  REQUIRE(res.validate == ValidationResult::VersionNotSupported);
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
  REQUIRE(res.parse == HTTPParser::ParseResult::HeadersDone);
  REQUIRE(res.validate == ValidationResult::BadRequest);
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

  REQUIRE(res.parse == HTTPParser::ParseResult::HeadersDone);
  REQUIRE(res.validate == ValidationResult::BadRequest);
}
