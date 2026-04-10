#include <string>
#include <string_view>

#include "catch_amalgamated.hpp"
#include "http/HTTPCookie.hpp"
#include "http/HTTPParser.hpp"
#include "http/HTTPRequest.hpp"
#include "http/HTTPUtils.hpp"
#include "http/SessionManager.hpp"

static HTTPRequest MakeParsedHeadersOnly(std::string_view raw)
{
  HTTPParser parser;

  auto res = parser.Parse(raw);
  REQUIRE(res == HTTPParser::ParseResult::kHeadersDone);
  parser.SetNoBody();
  res = parser.Parse({});
  REQUIRE(res == HTTPParser::ParseResult::kDone);

  return parser.TakeRequest();
}

static std::string ExtractSid(const HTTPResponse& res)
{
  std::string_view header = res.GetFirstHeaderValueOf("set-cookie");
  REQUIRE(!header.empty());

  const std::string_view key = "session_id=";
  std::size_t start = header.find(key);
  REQUIRE(start != std::string::npos);

  start += key.length();

  std::size_t end = header.find(';', start);
  if (end == std::string::npos)
    end = header.size();

  return std::string(header.substr(start, end - start));
}

static void AttachCookiesForTest(HTTPRequest& request)
{
  using CookieMap = HTTPRequest::CookieMap;
  const auto* vals = request.GetHeaderValuesOf("cookie");
  if (!vals || vals->empty())
    return;

  CookieMap cookies;
  REQUIRE(HTTP::cookie::ParseCookieHeader((*vals)[0], &cookies));
  request.SetCookies(std::move(cookies));
}

TEST_CASE("SessionManager creates session and sets Set-Cookie when missing", "[sessionManager]")
{
  SessionManager sm;
  HTTPRequest request = MakeParsedHeadersOnly(
      "GET / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "\r\n");
  HTTPResponse res;

  sm.UseOrCreateSessionAt(request, res, 1000);

  std::string sid = ExtractSid(res);
  REQUIRE(sid.size() == 32);
  REQUIRE(res.HasHeader("Set-Cookie"));
  REQUIRE(sm.SessionCount() == 1);
  REQUIRE(sm.HasSession(sid));
}

TEST_CASE("SessionManager reuses session_id cookie and does not reissue Set-Cookie", "[session]")
{
  SessionManager sm;

  HTTPRequest req1 = MakeParsedHeadersOnly(
      "GET / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "\r\n");
  HTTPResponse res1;
  sm.UseOrCreateSessionAt(req1, res1, 1000);

  std::string sid = ExtractSid(res1);
  REQUIRE(sm.HasSession(sid));

  HTTPRequest req2 =
      MakeParsedHeadersOnly(("GET / HTTP/1.1\r\n"
                             "Host: example.com\r\n"
                             "Cookie: session_id=" +
                             sid +
                             "\r\n"
                             "\r\n"));

  AttachCookiesForTest(req2);

  HTTPResponse res2;
  sm.UseOrCreateSessionAt(req2, res2, 1010);

  REQUIRE_FALSE(res2.HasHeader("set-cookie"));
  REQUIRE(sm.SessionCount() == 1);
  REQUIRE(sm.HasSession(sid));
}

TEST_CASE("SessionManager TTL cleanup expires old sessions", "[session][ttl]")
{
  SessionManager sm;

  HTTPRequest req1 = MakeParsedHeadersOnly(
      "GET / HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "\r\n");
  HTTPResponse res1;
  sm.UseOrCreateSessionAt(req1, res1, 1000);
  std::string sid = ExtractSid(res1);
  REQUIRE(sm.HasSession(sid));

  // Move time past TTL + 1
  HTTPRequest req2 =
      MakeParsedHeadersOnly(("GET / HTTP/1.1\r\n"
                             "Host: example.com\r\n"
                             "Cookie: session_id=" +
                             sid +
                             "\r\n"
                             "\r\n"));
  HTTPResponse res2;

  sm.UseOrCreateSessionAt(req2, res2, 1000 + HTTP::kTTL + 1);

  REQUIRE(res2.HasHeader("set-cookie"));
  REQUIRE(sm.SessionCount() == 1);
  REQUIRE_FALSE(sm.HasSession(sid));
  REQUIRE(sm.HasSession(ExtractSid(res2)));
}
