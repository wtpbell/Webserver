/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   test_CGIParser.cpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/02/24 17:21:57 by jboon         #+#    #+#                 */
/*   Updated: 2026/03/17 00:26:53 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <string_view>

#include "catch_amalgamated.hpp"
#include "cgi/CGIParser.hpp"

namespace
{
  using CGIHeaders = std::unordered_map<std::string, std::string>;

  bool HasHeaderEntry(const CGIHeaders& headers, const char* key, const char* value)
  {
    return headers.count(key) == 1 && headers.at(key) == value;
  }
}  // namespace

TEST_CASE("Valid IsToken", "[cgi][CGIParser]")
{
  auto token = GENERATE(as<std::string_view>{},
                        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
                        "1234567890!#$%&'*+-.`^_{|}~",
                        "a", "token.without.spaces");

  CAPTURE(token);
  REQUIRE(cgi::CGIParser::IsToken(token));
}

TEST_CASE("Invalid IsToken", "[cgi][CGIParser]")
{
  auto token =
      GENERATE(as<std::string_view>{}, "", " ", "          ", "\n", "a\x02hello", "token with spaces", "token         ",
               "token ", "         token", " token", "\a\a\a\a\a\a\atoken", "token\x06\x06\x06\x06", "token\ntoken",
               "token\ttoken", "token token", "token?token", "token=token");

  CAPTURE(token);
  REQUIRE_FALSE(cgi::CGIParser::IsToken(token));
}

TEST_CASE("Valid IsQuotedString", "[cgi][CGIParser]")
{
  auto quoted_string = GENERATE(as<std::string_view>{}, "\"\"", "\"hello world\"", "\"\t\"", "\" \"", "\"''\"",
                                "\"             \"", "\"\t\t\t\"", "\"HelloWorld\"");

  CAPTURE(quoted_string);
  REQUIRE(cgi::CGIParser::IsQuotedString(quoted_string));
}

TEST_CASE("Invalid IsQuotedString", "[cgi][CGIParser]")
{
  auto quoted_string =
      GENERATE(as<std::string_view>{}, "", "hello, world", "\"hello world", "hello world\"", "\"", "\"\"\"", "\"\"\"\"",
               "\"hello\nworld\"", "\"'\n'\"", "\"\a\ahello world\"", "\"hello\a\"", "''", "'Hello, World'",
               "\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"", "\"\"\"\"\"\"\"\"\"\"", "Hello\aWorld");

  CAPTURE(quoted_string);
  REQUIRE_FALSE(cgi::CGIParser::IsQuotedString(quoted_string));
}

TEST_CASE("Valid IsText", "[cgi][CGIParser]")
{
  auto text =
      GENERATE(as<std::string_view>{}, "a", "hello world", " ", "'\"~,./<>?[]}{-=_", " hello",
               "hello "
               "some more text to test out to see-if-long-texts-are allowed but only the printable ofcourse",
               " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");

  CAPTURE(text);
  REQUIRE(cgi::CGIParser::IsText(text));
}

TEST_CASE("Invalid IsText", "[cgi][CGIParser]")
{
  auto text = GENERATE(as<std::string_view>{}, "", "Hello\nWorld", "\t", "\n", "\nJohn Doe", "Test this string\t",
                       "Test this\tstring",
                       "try out this non-printable\x1b character"
                       "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16"
                       "\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x7f");

  CAPTURE(text);
  REQUIRE_FALSE(cgi::CGIParser::IsText(text));
}

TEST_CASE("Valid IsParameter", "[cgi][CGIParser]")
{
  auto parameter = GENERATE(as<std::string_view>{}, "h=hello", "hi=hello ", "hi=\"how=are=you=doing\"",
                            "header=\"quoted-string\"", "h=\" hello\"", "h=\"hello \"", "h=\" hello \"",
                            "header=\"quo te d\t-string\"", "header=\" 'header=value ' \"", "'header'=value");

  CAPTURE(parameter);
  REQUIRE(cgi::CGIParser::IsParameter(parameter));
}

TEST_CASE("Invalid IsParameter", "[cgi][CGIParser]")
{
  auto parameter = GENERATE(as<std::string_view>{}, "", "\n",
                            "\t"
                            "header",
                            "header= \"quo te d\t-string\"", "header= ", "a= hello", "b=hello a", "c= hello ", "=value",
                            "header =value", "\"header\"=value", "\"header=\"value", "\"header=value\"",
                            "hi=how=are=you=doing", "hi=h\"ow=are=you=doing\"", "header=v\alue");

  CAPTURE(parameter);
  REQUIRE_FALSE(cgi::CGIParser::IsParameter(parameter));
}

TEST_CASE("Null byte checks", "[cgi][CGIParser]")
{
  std::string s;
  s.resize(20);

  REQUIRE_FALSE(cgi::CGIParser::IsText(s));
  REQUIRE_FALSE(cgi::CGIParser::IsToken(s));
  REQUIRE_FALSE(cgi::CGIParser::IsQuotedString(s));
  REQUIRE_FALSE(cgi::CGIParser::IsParameter(s));

  s = "Hello, World!";
  s.insert(s.begin() + 3, '\0');
  REQUIRE_FALSE(cgi::CGIParser::IsText(s));
  REQUIRE_FALSE(cgi::CGIParser::IsToken(s));
  REQUIRE_FALSE(cgi::CGIParser::IsQuotedString(s));
  REQUIRE_FALSE(cgi::CGIParser::IsParameter(s));

  s = "header=value";
  s.insert(s.find_first_of('='), 1, '\0');
  REQUIRE_FALSE(cgi::CGIParser::IsParameter(s));

  s = "header=value";
  s.insert(s.find_first_of('=') + 1, 1, '\0');
  REQUIRE_FALSE(cgi::CGIParser::IsParameter(s));

  s = "\"\"";
  s.insert(s.find_first_of('\"') + 1, 1, '\0');
  REQUIRE_FALSE(cgi::CGIParser::IsQuotedString(s));

  s = "\"Some text inside here\"";
  s.insert(s.begin() + 8, '\0');
  REQUIRE_FALSE(cgi::CGIParser::IsQuotedString(s));
}

TEST_CASE("Valid MediaType", "[cgi][CGIParser]")
{
  auto media_type =
      GENERATE(as<std::string>{}, "application/json", "application/ld+json", "application/msword", "application/pdf",
               "application/sql", "application/vnd.api+json", "application/vnd.microsoft.portable-executable",
               "application/vnd.ms-excel", "application/vnd.ms-powerpoint", "application/vnd.oasis.opendocument.text",
               "application/vnd.openxmlformats-officedocument.presentationml.presentation",
               "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",
               "application/vnd.openxmlformats-officedocument.wordprocessingml.document",
               "application/x-www-form-urlencoded", "application/xml", "application/zip", "application/zstd",
               "audio/mpeg", "audio/ogg", "image/avif", "image/jpeg", "image/png", "image/svg+xml", "image/tiff",
               "model/obj", "multipart/form-data", "text/plain", "text/css", "text/csv", "text/html", "text/javascrip",
               "text/xml", "video/x-ms-wm", "x-conference/x-cooltalk", "application/vnd.fujitsu.oasys3");
  auto parameter = GENERATE(as<std::string_view>{}, "", ";charset=us-ascii", ";charset=UTF-8", ";charset=iso-8859-x",
                            ";name=\"myFile\"; filename=\"img.jpg\"",
                            "; boundary=---------------------------8721656041911415653955004498",
                            "; header=value; header=value; header=value; header=value; header=value",
                            ";header=value;header=value;header=value",
                            ";\t\theader=value; header=value\t;header=value   ", ";\tcharset=utf-8");
  media_type.append(parameter);

  CAPTURE(media_type);
  REQUIRE(cgi::CGIParser::IsMediaType(media_type));
}

TEST_CASE("Invalid MediaType", "[cgi][CGIParser]")
{
  auto media_type = GENERATE(
      as<std::string>{}, "application/ json", "application /ld+json", "application/msword/msword",
      "application/pdf application/pdf", "applic\ation/sql", "application vnd.api+json",
      "applicationvnd.microsoft.portable-executable", "application/vnd.(ms-excel)", "application/\tvnd.ms-powerpoint",
      "application\t/vnd.oasis.opendocument.text", "application\t/\tvnd.openxml", "/plain", "text/", "plain", "text");
  auto parameter = GENERATE(
      as<std::string_view>{}, "", ";charset=us-ascii", ";charset=UTF-8", ";charset=iso-8859-x",
      ";name=\"myFile\"; filename=\"img.jpg\"", "; boundary=---------------------------8721656041911415653955004498",
      "; header=value; header=value; header=value; header=value; header=value",
      ";header=value;header=value;header=value", ";\t\theader=value; header=value\t;header=value   ");
  media_type.append(parameter);

  CAPTURE(media_type);
  REQUIRE_FALSE(cgi::CGIParser::IsMediaType(media_type));
}

TEST_CASE("Invalid MediaType parameters", "[cgi][CGIParser]")
{
  auto media_type = GENERATE(as<std::string>{}, "text/plain", "application/json", "text/html", "image/png",
                             "application/vnd.ms-powerpoint");
  auto parameter = GENERATE(as<std::string_view>{}, ";", ";;", ";;;;;;;;;;", "; ", "; ; ", "; ; ; ;", ";charset=UTF-8;",
                            ";charset=UTF-8 ;", ";charset=UTF-8 ; ", ";charset=UTF-8;charset=UTF-8;", ";\"\"",
                            ";\"charset=UTF-8\"", ";a;b;c", ";;charset=UTF-8", ";charset=UTF-8;;",
                            ";header=", ";=value", ";header=;value", "header;=;value", " charset=UTF-8");
  media_type.append(parameter);

  CAPTURE(media_type);
  REQUIRE_FALSE(cgi::CGIParser::IsMediaType(media_type));
}

TEST_CASE("Valid IsFieldContent", "[cgi][CGIParser]")
{
  auto field_content = GENERATE(
      as<std::string_view>{}, "", " ", "\t", " \t", "a", "adf045ias", "hello world!", "hello\tTab!", "\",\"",
      "40i.Q*Ku9+'2dO|0$ljp", "sPP#NYiP!0QC}{*A+Jb8mt$C1p\t\t\t\t\tkdYt9}FYKY+|jkWMuwh#zb4\t",
      "\"sP]PNY/iP0?QC](*A[Jb.mt$C1pkd^Yt9}FYKY+]jkWMuwh#zb4\"", "\"\"", "\"\"\"\"", "\"value\"\"value\"",
      "\"value\" token \"value\"", "token \"value\" token \"value\"", "token \"value\" token \"value\" token",
      "\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"", "'' ", "!#$%&'*+-.`^_{|}~", "\"?\"", "'', ", "','", ")(", ">", "<",
      "?", "   \t token    token \"quote\"\"quote\" \"quote\" token\"quote\"token\"quote\"   \t token \"quote\"",
      "t45AYzA4}S{F}-{quGd\\");

  CAPTURE(field_content);
  REQUIRE(cgi::CGIParser::IsFieldContent(field_content));
}

TEST_CASE("Invalid IsFieldContent", "[cgi][CGIParser]")
{
  auto field_content =
      GENERATE(as<std::string_view>{}, "\x1", "\x3", "\x7", "\x19", "Hello, \a!", "t45AYzA4}S{F}-{quGd\\\x11",
               "hello,\x7Fworld!", "sP]PNY/iP0?QC](*A[Jb\x7mt$C1pkdYt9}FYKY+]jkWMuwh#zb4", "\"", "\"\"\"",
               "\"value\"value\"token", "40i.Q*Ku9+'2dO|0£ljp",
               "   \t token    token \"quote\"\"quote\" \"quote\" token\"quote\"token\"quote\"  \a\t token \"quote\"");

  CAPTURE(field_content);
  REQUIRE_FALSE(cgi::CGIParser::IsFieldContent(field_content));
}

TEST_CASE("Valid IsLocation", "[cgi][CGIParser]")
{
  auto location = GENERATE(as<std::string_view>{}, "http://hello.world.com", "https://google.com",
                           "https://www.google.com", "https://hello-world.com?key=value&key1=value1", "/", "/path",
                           "https://%20%20google.com", "https://google%20google.com", "https://google%FFgoogle.com",
                           "https://google%00google.com", "https://google%00google.com", "https://localhost",
                           "https://local%1A%1A%1A%1A%1A%1A%1Ahost", "https://local%1A333host", "/path/to/file.txt",
                           "/./current", "/../parent", "/some-path/path*path/", "/index.html", "/%20");

  CAPTURE(location);
  REQUIRE(cgi::CGIParser::IsLocation(location));
}

TEST_CASE("Invalid IsLocation", "[cgi][CGIParser]")
{
  auto location =
      GENERATE(as<std::string_view>{}, "https://", "http://", "https:/", "http:/", "http:/hello.world.com",
               "https:/google.com", "://", "ftp://", "https:/\a/", "\ahttps://", "https://\a", "http:/\a/", "\ahttp://",
               "http://\a", "%20", "https://%2localhost", "http://local%", "http://localhost%20%2",
               "https://localhost%7xF", "/\a/index.html", "./current/index.html", ".", "..", "fdsa", ",/");

  CAPTURE(location);
  REQUIRE_FALSE(cgi::CGIParser::IsLocation(location));
}

TEST_CASE("CGI parse without status field", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Content-Type: text/plain; charset=utf-8\n"
      "Server: webserve/cgi\n"
      "Connection: Keep-Alive\n"
      "X-CGI-Type: Some custom data here\n"
      "\n"
      "hello, world! What a beautiful day it is for some unit tests!"};

  std::string_view raw_body{"hello, world! What a beautiful day it is for some unit tests!"};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE(cgi_response.has_value());
  REQUIRE(cgi_response->GetStatus().status_code == 200);
  REQUIRE(cgi_response->GetStatus().reason == "OK");
  REQUIRE(cgi_response->GetBody() == raw_body);

  const auto& headers = cgi_response->GetHeaders();
  REQUIRE(HasHeaderEntry(headers, "Content-Type", "text/plain; charset=utf-8"));
  REQUIRE(headers.count("Status") == 0);
  REQUIRE(HasHeaderEntry(headers, "Server", "webserve/cgi"));
  REQUIRE(HasHeaderEntry(headers, "Connection", "Keep-Alive"));
  REQUIRE(HasHeaderEntry(headers, "X-CGI-Type", "Some custom data here"));
  REQUIRE(cgi_response->GetCookies().empty());
}

TEST_CASE("CGI parse with status field", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Content-Type: text/plain; charset=utf-8\n"
      "Status: 404 Page not found\n"
      "Server: webserve/cgi\n"
      "Connection: Keep-Alive\n"
      "X-CGI-Type: Some custom data here\n"
      "\n"
      "hello, world! What a beautiful day it is for some unit tests!"};

  std::string_view raw_body{"hello, world! What a beautiful day it is for some unit tests!"};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE(cgi_response.has_value());
  REQUIRE(cgi_response->GetStatus().status_code == 404);
  REQUIRE(cgi_response->GetStatus().reason == "Page not found");
  REQUIRE(cgi_response->GetBody() == raw_body);

  const auto& headers = cgi_response->GetHeaders();
  REQUIRE(HasHeaderEntry(headers, "Content-Type", "text/plain; charset=utf-8"));
  REQUIRE(headers.count("Status") == 0);
  REQUIRE(HasHeaderEntry(headers, "Server", "webserve/cgi"));
  REQUIRE(HasHeaderEntry(headers, "Connection", "Keep-Alive"));
  REQUIRE(HasHeaderEntry(headers, "X-CGI-Type", "Some custom data here"));
  REQUIRE(cgi_response->GetCookies().empty());
}

TEST_CASE("CGI parse with status (code, no reason) field", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Content-Type: text/plain; charset=utf-8\n"
      "Status: 403\n"
      "Server: webserve/cgi\n"
      "Connection: Keep-Alive\n"
      "X-CGI-Type: Some custom data here\n"
      "\n"
      "hello, world! What a beautiful day it is for some unit tests!"};

  std::string_view raw_body{"hello, world! What a beautiful day it is for some unit tests!"};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE(cgi_response.has_value());
  REQUIRE(cgi_response->GetStatus().status_code == 403);
  REQUIRE(cgi_response->GetStatus().reason == "");
  REQUIRE(cgi_response->GetBody() == raw_body);

  const auto& headers = cgi_response->GetHeaders();
  REQUIRE(HasHeaderEntry(headers, "Content-Type", "text/plain; charset=utf-8"));
  REQUIRE(headers.count("Status") == 0);
  REQUIRE(HasHeaderEntry(headers, "Server", "webserve/cgi"));
  REQUIRE(HasHeaderEntry(headers, "Connection", "Keep-Alive"));
  REQUIRE(HasHeaderEntry(headers, "X-CGI-Type", "Some custom data here"));
  REQUIRE(cgi_response->GetCookies().empty());
}

TEST_CASE("CGI parse weirdly formatted", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Server: \t   webserve/cgi\n"
      "Status:  302 Redirect Me                 \n"
      "Connection: Keep-Alive  \n"
      "Content-Type:\ttext/plain;     \t   charset=utf-8\t\t\n"
      "X-CGI-Type:   Some custom data here \t\t\t\t\n"
      "\n"};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE(cgi_response.has_value());
  REQUIRE(cgi_response->GetStatus().status_code == 302);
  REQUIRE(cgi_response->GetStatus().reason == "Redirect Me");
  REQUIRE(cgi_response->GetBody() == "");

  const auto& headers = cgi_response->GetHeaders();
  REQUIRE(HasHeaderEntry(headers, "Content-Type", "text/plain;     \t   charset=utf-8"));
  REQUIRE(headers.count("Status") == 0);
  REQUIRE(HasHeaderEntry(headers, "Server", "webserve/cgi"));
  REQUIRE(HasHeaderEntry(headers, "Connection", "Keep-Alive"));
  REQUIRE(HasHeaderEntry(headers, "X-CGI-Type", "Some custom data here"));
  REQUIRE(cgi_response->GetCookies().empty());
}

TEST_CASE("CGI parse response with multi value", "[cgi][CGIParser]")
{
  auto has_header_entry = [](const CGIHeaders& headers, const char* key, const char* value) -> bool
  {
    return headers.count(key) == 1 && headers.at(key) == value;
  };

  std::string_view raw_response{
      "Content-Type: text/plain; charset=utf-8\n"
      "Server: webserve/cgi\n"
      "Connection: Keep-Alive\n"
      "Server: webserve/cgi\n"
      "X-CGI-Type: Some custom data here\n"
      "\n"
      "hello, world! What a beautiful day it is for some unit tests!"};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE(cgi_response.has_value());
  REQUIRE(cgi_response->GetStatus().status_code == 200);
  REQUIRE(cgi_response->GetStatus().reason == "OK");
  REQUIRE(cgi_response->GetBody() == "hello, world! What a beautiful day it is for some unit tests!");

  const auto& headers = cgi_response->GetHeaders();
  REQUIRE(has_header_entry(headers, "Server", "webserve/cgi, webserve/cgi"));
  REQUIRE(cgi_response->GetCookies().empty());
}

TEST_CASE("CGI parse empty raw response", "[cgi][CGIParser]")
{
  auto raw_response =
      GENERATE(as<std::string_view>{}, "", "\n", "\n\n  \n                \n", " ", "                            ");

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE_FALSE(cgi_response.has_value());
}

TEST_CASE("Status header mispelled", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Content-Type: text/plain; charset=utf-8\n"
      "Stetus: 404 Page not found\n"
      "Server: webserve/cgi\n"
      "\n"
      "SOMEBODY ONCE TOLD ME ..."};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE(cgi_response.has_value());
  REQUIRE_FALSE(cgi_response->GetStatus().status_code == 404);
  REQUIRE_FALSE(cgi_response->GetStatus().reason == "Page not found");
}

TEST_CASE("Status header lowercase", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Content-Type: text/plain; charset=utf-8\n"
      "status: 404 Page not found\n"
      "Server: webserve/cgi\n"
      "\n"
      "SOMEBODY ONCE TOLD ME ..."};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE(cgi_response.has_value());
  REQUIRE_FALSE(cgi_response->GetStatus().status_code == 404);
  REQUIRE_FALSE(cgi_response->GetStatus().reason == "Page not found");
}

TEST_CASE("Status header uppercase", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Content-Type: text/plain; charset=utf-8\n"
      "STATUS: 404 Page not found\n"
      "Server: webserve/cgi\n"
      "\n"
      "SOMEBODY ONCE TOLD ME ..."};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE(cgi_response.has_value());
  REQUIRE_FALSE(cgi_response->GetStatus().status_code == 404);
  REQUIRE_FALSE(cgi_response->GetStatus().reason == "Page not found");
}

TEST_CASE("Status header as final header field", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Content-Type: text/plain; charset=utf-8\n"
      "Server: webserve/cgi\n"
      "Status: 404 Page not found\n"
      "\n"
      "SOMEBODY ONCE TOLD ME ..."};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE(cgi_response.has_value());
  REQUIRE(cgi_response->GetStatus().status_code == 404);
  REQUIRE(cgi_response->GetStatus().reason == "Page not found");
}

TEST_CASE("Status header as first header field", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Status: 404 Page not found\n"
      "Content-Type: text/plain; charset=utf-8\n"
      "Server: webserve/cgi\n"
      "\n"
      "SOMEBODY ONCE TOLD ME ..."};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE(cgi_response.has_value());
  REQUIRE(cgi_response->GetStatus().status_code == 404);
  REQUIRE(cgi_response->GetStatus().reason == "Page not found");
}

TEST_CASE("CGI parse ill formed - Lowercase content-type", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "content-type: text/plain; charset=utf-8\n"
      "Status: 200 KO\n"
      "\n"
      "SOMEBODY ONCE TOLD ME ..."};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE_FALSE(cgi_response.has_value());
}

TEST_CASE("CGI parse ill formed - Content-[t]ype", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Content-type: text/plain; charset=utf-8\n"
      "Status: 200 KO\n"
      "\n"
      "SOMEBODY ONCE TOLD ME ..."};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE_FALSE(cgi_response.has_value());
}

TEST_CASE("CGI parse ill formed - Content-Type misspelled", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "C0ntent-type: text/plain; charset=utf-8\n"
      "Status: 200 KO\n"
      "\n"
      "SOMEBODY ONCE TOLD ME ..."};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE_FALSE(cgi_response.has_value());
}

TEST_CASE("CGI parse ill formed - Content-[type]:", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Content-: text/plain; charset=utf-8\n"
      "Status: 200 KO\n"
      "\n"
      "SOMEBODY ONCE TOLD ME ..."};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE_FALSE(cgi_response.has_value());
}

TEST_CASE("CGI parse ill formed - Leading whitespace at header", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Content-Type: text/plain; charset=utf-8\n"
      "Status: 200 KO\n"
      "   Server: webserve/cgi\n"
      "\n"
      "SOMEBODY ONCE TOLD ME ..."};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE_FALSE(cgi_response.has_value());
}

TEST_CASE("CGI parse ill formed - Trailing whitespace at header", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Content-Type: text/plain; charset=utf-8\n"
      "Status: 200 KO\n"
      "Server   : webserve/cgi\n"
      "\n"
      "SOMEBODY ONCE TOLD ME ..."};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE_FALSE(cgi_response.has_value());
}

TEST_CASE("CGI parse ill formed - No newline between headers", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Server: webserve/cgi "
      "Content-Type: text/plain; charset=utf-8 "
      "Status: 204 KO"
      "\n"
      "SOMEBODY ONCE TOLD ME ..."};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE_FALSE(cgi_response.has_value());
}

TEST_CASE("CGI parse ill formed - Status with no digits", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Content-Type: text/plain; charset=utf-8\n"
      "Status: xxx KO\n"
      "Server   : webserve/cgi\n"
      "\n"
      "SOMEBODY ONCE TOLD ME ..."};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE_FALSE(cgi_response.has_value());
}

TEST_CASE("CGI parse ill formed - Missing digits for status", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Content-: text/plain; charset=utf-8\n"
      "Status: KO\n"
      "\n"
      "SOMEBODY ONCE TOLD ME ..."};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE_FALSE(cgi_response.has_value());
}

TEST_CASE("CGI parse ill formed - Single digit for status", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Content-: text/plain; charset=utf-8\n"
      "Status: 4 KO\n"
      "\n"
      "SOMEBODY ONCE TOLD ME ..."};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE_FALSE(cgi_response.has_value());
}

TEST_CASE("CGI parse ill formed - double digit for status", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Content-: text/plain; charset=utf-8\n"
      "Status: 40 KO\n"
      "\n"
      "SOMEBODY ONCE TOLD ME ..."};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE_FALSE(cgi_response.has_value());
}

TEST_CASE("CGI parse ill formed - quad digits for status", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Content-: text/plain; charset=utf-8\n"
      "Status: 4044 KO\n"
      "\n"
      "SOMEBODY ONCE TOLD ME ..."};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE_FALSE(cgi_response.has_value());
}

TEST_CASE("CGI parse ill formed - Status without separator", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Content-: text/plain; charset=utf-8\n"
      "Status: 404KO\n"
      "\n"
      "SOMEBODY ONCE TOLD ME ..."};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE_FALSE(cgi_response.has_value());
}

TEST_CASE("CGI parse ill formed - Status with non-printables", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Content-: text/plain; charset=utf-8\n"
      "Status: 400 B\ad Reqr\x06ue\ast\n"
      "\n"
      "SOMEBODY ONCE TOLD ME ..."};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE_FALSE(cgi_response.has_value());
}

TEST_CASE("CGI parse ill formed - No header-value separator", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Content-Type: text/plain; charset=utf-8\n"
      "Status: 200 KO\n"
      "Server webserve/cgi\n"
      "\n"
      "SOMEBODY ONCE TOLD ME ..."};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE_FALSE(cgi_response.has_value());
}

TEST_CASE("CGI parse ill formed - Missing newline separator", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Content-Type: text/plain; charset=utf-8"
      "Status: 204 KO\n"
      "Server: webserve/cgi\n"
      "\n"
      "SOMEBODY ONCE TOLD ME ..."};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE_FALSE(cgi_response.has_value());
}

TEST_CASE("CGI parse ill formed - No Content-Type or Location header", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Status: 201 KO\n"
      "Server: webserve/cgi\n"
      "\n"
      "SOMEBODY ONCE TOLD ME ..."};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE_FALSE(cgi_response.has_value());
}

TEST_CASE("CGI parse ill formed - Duplicate Content-Type", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Content-Type: text/plain; charset=utf-8\n"
      "Status: 201 KO\n"
      "Server: webserve/cgi\n"
      "Content-Type: text/plain; charset=utf-8\n"
      "\n"
      "SOMEBODY ONCE TOLD ME ..."};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE_FALSE(cgi_response.has_value());
}

TEST_CASE("CGI parse ill formed - Duplicate Status", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Status: 201 KO\n"
      "Server: webserve/cgi\n"
      "Content-Type: text/plain; charset=utf-8\n"
      "Status: 404 Page not found\n"
      "\n"
      "SOMEBODY ONCE TOLD ME ..."};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE_FALSE(cgi_response.has_value());
}

TEST_CASE("CGI parse ill formed - Duplicate Location", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Status: 201 KO\n"
      "Location: /index.html\n"
      "Server: webserve/cgi\n"
      "Content-Type: text/plain; charset=utf-8\n"
      "Location: http://localhost\n"
      "\n"
      "SOMEBODY ONCE TOLD ME ..."};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE_FALSE(cgi_response.has_value());
}

TEST_CASE("CGI parse ill formed - Missing ending newline", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Status: 201 KO\n"
      "Location: /index.html\n"
      "Server: webserve/cgi\n"
      "Content-Type: text/plain; charset=utf-8\n"};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE_FALSE(cgi_response.has_value());
}

TEST_CASE("CGI parse ill formed - Ending newline is not empty", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Status: 201 KO\n"
      "Location: /index.html\n"
      "Server: webserve/cgi\n"
      "Content-Type: text/plain; charset=utf-8\n"
      " \n"};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE_FALSE(cgi_response.has_value());
}

TEST_CASE("CGI parse with empty body", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Content-Type: text/plain; charset=utf-8\n"
      "Status: 202 Awesome\n"
      "\n"};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE(cgi_response.has_value());
  REQUIRE(cgi_response->GetBody().empty());
  REQUIRE(HasHeaderEntry(cgi_response->GetHeaders(), "Content-Type", "text/plain; charset=utf-8"));
  REQUIRE(cgi_response->GetStatus().status_code == 202);
  REQUIRE(cgi_response->GetStatus().reason == "Awesome");
}

TEST_CASE("CGI parse with relative Location", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Location: /index.html\n"
      "\n"};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE(cgi_response.has_value());
  REQUIRE(cgi_response->GetBody().empty());
  REQUIRE(HasHeaderEntry(cgi_response->GetHeaders(), "Location", "/index.html"));
  REQUIRE(cgi_response->GetStatus().status_code == 302);
  REQUIRE(cgi_response->GetStatus().reason == "Found");
  REQUIRE(cgi_response->IsLocalRedirect());
  REQUIRE(cgi_response->LocalTarget() == "/index.html");
}

TEST_CASE("CGI parse with absolute-uri Location", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Location: https://google.com\n"
      "\n"};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE(cgi_response.has_value());
  REQUIRE(cgi_response->GetBody().empty());
  REQUIRE(HasHeaderEntry(cgi_response->GetHeaders(), "Location", "https://google.com"));
  REQUIRE(cgi_response->GetStatus().status_code == 302);
  REQUIRE(cgi_response->GetStatus().reason == "Found");
  REQUIRE_FALSE(cgi_response->IsLocalRedirect());
}

TEST_CASE("CGI parse with \\r\\n", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Content-Type: text/plain; charset=utf-8\r\n"
      "Server: webserve/cgi\r\n"
      "Connection: Keep-Alive\r\n"
      "X-CGI-Type: Some custom data here\r\n"
      "\r\n"
      "hello, world! What a beautiful day it is for some unit tests!"};

  std::string_view raw_body{"hello, world! What a beautiful day it is for some unit tests!"};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE(cgi_response.has_value());
  REQUIRE(cgi_response->GetStatus().status_code == 200);
  REQUIRE(cgi_response->GetStatus().reason == "OK");
  REQUIRE(cgi_response->GetBody() == raw_body);

  const auto& headers = cgi_response->GetHeaders();
  REQUIRE(HasHeaderEntry(headers, "Content-Type", "text/plain; charset=utf-8"));
  REQUIRE(headers.count("Status") == 0);
  REQUIRE(HasHeaderEntry(headers, "Server", "webserve/cgi"));
  REQUIRE(HasHeaderEntry(headers, "Connection", "Keep-Alive"));
  REQUIRE(HasHeaderEntry(headers, "X-CGI-Type", "Some custom data here"));
}

TEST_CASE("CGI parse with mixed \\r\\n", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Content-Type: text/plain; charset=utf-8\n"
      "Server: webserve/cgi\r\n"
      "Connection: Keep-Alive\n"
      "X-CGI-Type: Some custom data here\r\n"
      "\n"
      "hello, world! What a beautiful day it is for some unit tests!"};

  std::string_view raw_body{"hello, world! What a beautiful day it is for some unit tests!"};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE(cgi_response.has_value());
  REQUIRE(cgi_response->GetStatus().status_code == 200);
  REQUIRE(cgi_response->GetStatus().reason == "OK");
  REQUIRE(cgi_response->GetBody() == raw_body);

  const auto& headers = cgi_response->GetHeaders();
  REQUIRE(HasHeaderEntry(headers, "Content-Type", "text/plain; charset=utf-8"));
  REQUIRE(headers.count("Status") == 0);
  REQUIRE(HasHeaderEntry(headers, "Server", "webserve/cgi"));
  REQUIRE(HasHeaderEntry(headers, "Connection", "Keep-Alive"));
  REQUIRE(HasHeaderEntry(headers, "X-CGI-Type", "Some custom data here"));
}

TEST_CASE("CGI parse with cookies", "[cgi][CGIParser]")
{
  std::string_view raw_response{
      "Content-Type: text/plain; charset=utf-8\n"
      "Server: webserve/cgi\n"
      "Connection: Keep-Alive\n"
      "X-CGI-Type: Some custom data here\n"
      "Set-Cookie: cookie1=value1\n"
      "Set-Cookie: cookie2=value2\n"
      "\n"
      "hello, world! What a beautiful day it is for some unit tests!"};

  std::string_view raw_body{"hello, world! What a beautiful day it is for some unit tests!"};

  auto cgi_response = cgi::CGIParser::Parse(raw_response);
  REQUIRE(cgi_response.has_value());
  REQUIRE(cgi_response->GetStatus().status_code == 200);
  REQUIRE(cgi_response->GetStatus().reason == "OK");
  REQUIRE(cgi_response->GetBody() == raw_body);

  const auto& headers = cgi_response->GetHeaders();
  REQUIRE(HasHeaderEntry(headers, "Content-Type", "text/plain; charset=utf-8"));
  REQUIRE(headers.count("Status") == 0);
  REQUIRE(HasHeaderEntry(headers, "Server", "webserve/cgi"));
  REQUIRE(HasHeaderEntry(headers, "Connection", "Keep-Alive"));
  REQUIRE(HasHeaderEntry(headers, "X-CGI-Type", "Some custom data here"));
  REQUIRE(cgi_response->GetCookies().size() == 2);
  REQUIRE(cgi_response->GetCookies()[0] == "cookie1=value1");
  REQUIRE(cgi_response->GetCookies()[1] == "cookie2=value2");
}
