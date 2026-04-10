#include "catch_amalgamated.hpp"
#include "http/HTTPResponse.hpp"

TEST_CASE("HTTPResponse SetStatus updates code and reason", "[http][SetStatus]")
{
  HTTPResponse response;
  response.SetStatus(HTTP::Status::kNotFound);

  REQUIRE(response.GetStatusCode() == 404);
  REQUIRE(response.GetReason() == "Not Found");
}

TEST_CASE("HTTPResponse SetHeader overwrites existing header", "[http][SetHeader]")
{
  HTTPResponse response;
  response.SetHeader("Content-Type", "text/plain");
  response.SetHeader("Content-Type", "application/json");

  std::string_view val = response.GetFirstHeaderValueOf("Content-Type");

  REQUIRE_FALSE(val.empty());
  REQUIRE(val == "application/json");
}

TEST_CASE("HTTPResponse GetFirstHeaderValueOf returns empty when missing", "[http][GetFirstHeaderValueOf]")
{
  HTTPResponse response;
  REQUIRE(response.GetFirstHeaderValueOf("X-Nope").empty());
}

TEST_CASE("HTTPResponse body is stored verbatim", "[http][SetBody]")
{
  HTTPResponse response;
  response.SetBody("abc");

  REQUIRE(response.GetBody() == "abc");
  REQUIRE(response.GetBody().size() == 3);
}

TEST_CASE("ToHTTPStatus maps ValidationResult correctly", "[http][ToHTTPStatus]")
{
  REQUIRE(HTTP::ToHTTPStatus(ValidationResult::kBadRequest) == HTTP::Status::kBadRequest);

  REQUIRE(HTTP::ToHTTPStatus(ValidationResult::kURITooLong) == HTTP::Status::kURITooLong);

  REQUIRE(HTTP::ToHTTPStatus(ValidationResult::kPayloadTooLarge) == HTTP::Status::kPayloadTooLarge);

  REQUIRE(HTTP::ToHTTPStatus(ValidationResult::kNotImplemented) == HTTP::Status::kNotImplemented);

  REQUIRE(HTTP::ToHTTPStatus(ValidationResult::kVersionNotSupported) == HTTP::Status::kVersionNotSupported);
}

TEST_CASE("ToReasonPhrase returns non-empty for all HTTP statuses", "[http][ToReasonPhrase]")
{
  using HTTP::Status;

  const Status all[] = {Status::kOk,
                        Status::kCreated,
                        Status::kNoContent,
                        Status::kBadRequest,
                        Status::kUnauthorized,
                        Status::kForbidden,
                        Status::kNotFound,
                        Status::kMethodNotAllowed,
                        Status::kPayloadTooLarge,
                        Status::kURITooLong,
                        Status::kUnsupportedMediaType,
                        Status::kRangeNotSatisfiable,
                        Status::kInternalServerError,
                        Status::kNotImplemented,
                        Status::kBadGateway,
                        Status::kServiceUnavailable,
                        Status::kGatewayTimeout,
                        Status::kVersionNotSupported};

  for (auto s : all)
    REQUIRE(HTTP::ToReasonPhrase(s).size() > 0);
}
