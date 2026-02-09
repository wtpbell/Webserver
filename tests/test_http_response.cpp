#include "catch_amalgamated.hpp"
#include "http/HTTPResponse.hpp"

TEST_CASE("HTTPResponse SetStatus updates code and reason", "[http][SetStatus]")
{
  HTTPResponse resp;
  resp.SetStatus(HTTP::Status::NOT_FOUND);

  REQUIRE(resp.GetStatusCode() == 404);
  REQUIRE(resp.GetReason() == "Not Found");
}

TEST_CASE("HTTPResponse SetHeader overwrites existing header", "[http][SetHeader]")
{
  HTTPResponse resp;
  resp.SetHeader("Content-Type", "text/plain");
  resp.SetHeader("Content-Type", "application/json");

  std::string_view val = resp.GetFirstHeaderValueOf("Content-Type");

  REQUIRE_FALSE(val.empty());
  REQUIRE(val == "application/json");
}

TEST_CASE("HTTPResponse GetFirstHeaderValueOf returns empty when missing", "[http][GetFirstHeaderValueOf]")
{
  HTTPResponse resp;
  REQUIRE(resp.GetFirstHeaderValueOf("X-Nope").empty());
}

TEST_CASE("HTTPResponse body is stored verbatim", "[http][SetBody]")
{
  HTTPResponse resp;
  resp.SetBody("abc");

  REQUIRE(resp.GetBody() == "abc");
  REQUIRE(resp.GetBody().size() == 3);
}

TEST_CASE("ToHTTPStatus maps ValidationResult correctly", "[http][ToHTTPStatus]")
{
  REQUIRE(HTTP::ToHTTPStatus(ValidationResult::BadRequest) == HTTP::Status::BAD_REQUEST);

  REQUIRE(HTTP::ToHTTPStatus(ValidationResult::URITooLong) == HTTP::Status::URI_TOO_LONG);

  REQUIRE(HTTP::ToHTTPStatus(ValidationResult::PayloadTooLarge) == HTTP::Status::PAYLOAD_TOO_LARGE);

  REQUIRE(HTTP::ToHTTPStatus(ValidationResult::NotImplemented) == HTTP::Status::NOT_IMPLEMENTED);

  REQUIRE(HTTP::ToHTTPStatus(ValidationResult::VersionNotSupported) == HTTP::Status::HTTP_VERSION_NOT_SUPPORTED);
}

TEST_CASE("ToReasonPhrase returns non-empty for all HTTP statuses", "[http][ToReasonPhrase]")
{
  using HTTP::Status;

  const Status all[] = {Status::OK,
                        Status::CREATED,
                        Status::NO_CONTENT,
                        Status::BAD_REQUEST,
                        Status::UNAUTHORIZED,
                        Status::FORBIDDEN,
                        Status::NOT_FOUND,
                        Status::METHOD_NOT_ALLOWED,
                        Status::PAYLOAD_TOO_LARGE,
                        Status::URI_TOO_LONG,
                        Status::UNSUPPORTED_MEDIA_TYPE,
                        Status::RANGE_NOT_SATISFIABLE,
                        Status::INTERNAL_SERVER_ERROR,
                        Status::NOT_IMPLEMENTED,
                        Status::BAD_GATEWAY,
                        Status::SERVICE_UNAVAILABLE,
                        Status::GATEWAY_TIMEOUT,
                        Status::HTTP_VERSION_NOT_SUPPORTED};

  for (auto s : all)
    REQUIRE(HTTP::ToReasonPhrase(s).size() > 0);
}
