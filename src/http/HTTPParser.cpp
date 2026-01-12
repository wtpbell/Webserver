/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPParser.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/09 17:14:02 by bewong        #+#    #+#                 */
/*   Updated: 2026/01/08 19:57:47 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "http/HTTPParser.hpp"

#include <algorithm>
#include <charconv>
#include <cstdlib>

#include "http/HTTPRequest.hpp"
#include "http/HTTPTypes.hpp"
#include "string.hpp"

/************************************************** Helper ***********************************************************/

namespace
{
  // token 1*<any char except SP/HTAB/CR/LF> is handled by caller; here we just split by delim
  // based on whitespace rules.

  // Read a token until the next SP, require exactly one SP, and position i at start of next token.
  // Grammar: token SP
  bool ReadTokenSP(const std::string& s, size_t& i, size_t end, size_t& tokBegin, size_t& tokEnd)
  {
    if (i >= end)
      return false;
    tokBegin = i;
    size_t sp = s.find(' ', i);
    if (sp == std::string::npos || sp >= end)
      return false;
    tokEnd = sp;
    if (tokEnd <= tokBegin)
      return false;
    i = sp + 1;
    if (i >= end || s[i] == ' ' || s[i] == '\t')
      return false;
    return true;
  }
}  // namespace

/************************************************ Validation *********************************************************/

bool HTTPParser::IsComplete(void) const
{
  return state_ == ParserState::Done;
}

bool HTTPParser::HasError(void) const
{
  return state_ == ParserState::Error;
}

void HTTPParser::Fail(void)
{
  state_ = ParserState::Error;
}

/************************************************** Getter ***********************************************************/

const HTTPRequest& HTTPParser::GetRequest(void) const
{
  return req_;
}

HTTPParser::ParserState HTTPParser::GetState(void) const
{
  return state_;
}

void HTTPParser::Reset(void)
{
  req_ = HTTPRequest();
  state_ = ParserState::StartLine;
  buffer_.clear();
  pos_ = 0;
  contentLength_ = 0;
  bodyRead_ = 0;
  chunk_ = ChunkState{};
}

HTTPRequest HTTPParser::TakeRequest(void)
{
  HTTPRequest req = std::move(req_);
  state_ = ParserState::Done;
  // clear parser only after request destroyed
  return req;
}

/************************************************ Parse lines *********************************************************/
/*
    @brief Consumes a line from the buffer
    @param buf The buffer contining the HTTP request
    @param pos The position to start consuming from
    @param lineEnd The position of the kCRLF
    @return true if the line was consumed, false otherwise
*/
bool HTTPParser::ConsumeLine(const std::string& buf, size_t& pos, size_t& lineEnd)
{
  lineEnd = buf.find(HTTP::kCRLF, pos);
  if (lineEnd == std::string::npos)
    return false;
  pos = lineEnd + 2;
  return true;
}
// request-line   = method SP request-target SP HTTP-version kCRLF
// GET http://example.com?foo HTTP/1.1
bool HTTPParser::ParseRequestLine(const size_t lineStart, size_t lineEnd)
{
  size_t mBegin, mEnd, tBegin, tEnd;
  size_t i = lineStart;

  if (!ReadTokenSP(buffer_, i, lineEnd, mBegin, mEnd) || !ReadTokenSP(buffer_, i, lineEnd, tBegin, tEnd) ||
      i >= lineEnd)
    return (Fail(), false);

  std::string_view method(buffer_.data() + mBegin, mEnd - mBegin);
  std::string_view target(buffer_.data() + tBegin, tEnd - tBegin);
  std::string_view version(buffer_.data() + i, lineEnd - i);
  if (version.find(' ') != std::string::npos || version != HTTP::kVERSION)
    return (Fail(), false);
  // TODO(VALIDATE): method support checks (501 vs 400)
  // if (!HTTPValidation::IsValidMethod(method)) return Fail();
  req_.SetMethod(method);
  req_.SetTarget(target);
  req_.SetVersion(std::string(version));
  return true;
}

// Header-Name: value\r\n
// Host: example.com \r\n
// Content-Type: text/html \r\n
// Content-Length: 42 \r\n
// \r\n
bool HTTPParser::ParseHeaderLine(const size_t lineStart, size_t lineEnd)
{
  size_t colon = buffer_.find(':', lineStart);
  if (colon == std::string::npos || colon >= lineEnd)
    return (Fail(), false);

  for (size_t i = lineStart; i < colon; ++i)
  {
    if (buffer_[i] == ' ' || buffer_[i] == '\t')
      return (Fail(), false);
  }

  std::string_view name(buffer_.data() + lineStart, colon - lineStart);

  size_t vBegin = colon + 1;
  while (vBegin < lineEnd && (buffer_[vBegin] == ' ' || buffer_[vBegin] == '\t'))
    ++vBegin;

  std::string_view value(buffer_.data() + vBegin, lineEnd - vBegin);
  req_.AddHeader(name, value);
  return true;
}

/********************************************** Parse Chunked *********************************************************/
/*
3\r\n 3(hex) = 3 bytes -> Cod
Cod\r\n
2\r\n 2(hex) = 2 bytes -> am
am\r\n
0\r\n 0 -> end
\r\n trailer section
we get Codam at the end
*/
// TODO: check the kMAxBodySize -> return 413
bool HTTPParser::ParseChunkSize(void)
{
  size_t lineEnd;
  size_t start = pos_;

  if (!ConsumeLine(buffer_, pos_, lineEnd))
    return false;
  std::string_view line(buffer_.data() + start, lineEnd - start);
  // can accept chunk extentsions A;foo=bar\r\n
  auto semi = line.find(';');
  if (semi != std::string_view::npos)
    line.remove_suffix(line.size() - semi);
  line = String::Trim(line);
  if (line.empty())
    return (Fail(), false);
  // parse hex
  std::size_t size = 0;
  auto [ptr, ec] = std::from_chars(line.data(), line.data() + line.size(), size, 16);
  // Validation
  if (ec != std::errc{} || ptr != line.data() + line.size())
    return (Fail(), false);  // invalid chunk size

  chunk_.remaining = size;
  chunk_.phase = (size == 0) ? ChunkState::Phase::Trailer : ChunkState::Phase::Data;

  return true;
}

bool HTTPParser::ParseChunkData(void)
{
  if (buffer_.size() - pos_ < chunk_.remaining)
    return false;
  req_.AppendBody(std::string_view(buffer_.data() + pos_, chunk_.remaining));
  pos_ += chunk_.remaining;
  chunk_.remaining = 0;
  chunk_.phase = ChunkState::Phase::DataCRLF;
  return true;
}

bool HTTPParser::ParseChunkCRLF(void)
{
  if (buffer_.size() - pos_ < 2)
    return false;
  if (buffer_[pos_] != '\r' || buffer_[pos_ + 1] != '\n')
    return (Fail(), false);

  pos_ += 2;
  return true;
}

bool HTTPParser::ParseChunkTrailer(void)
{
  if (!ParseChunkCRLF())
    return false;

  chunk_.phase = ChunkState::Phase::Done;
  return true;
}

/************************************************* Parser ************************************************************/

/*
    @brief Parses the start line of the HTTP request
    @return ParseProgress::NeedMoreData if need more data,
    ParseProgress::Advance if made progress or terminal state(Done / Error)
    @note It is RECOMMENDED that all HTTP senders and recipients support, at a minimum, request-line lengths of 8000
   octets.
    @note nginx default is ~8k–16k depending on build
    @note if there is no kCRLF, check if the request line is too long
*/
bool HTTPParser::ParseStartLine(void)
{
  while (true)
  {
    size_t lineEnd = 0;
    const size_t lineStart = pos_;

    if (!ConsumeLine(buffer_, pos_, lineEnd))  // if there is no kCRLF
    {
      if (buffer_.size() - lineStart > HTTP::kMaxRequestLine)  // if the request line is too long
      {
        Fail();  // set state to error
        return false;
      }
      return false;  // is incomplete, need more data
    }
    // skip leading empty lines (RFC-compliant)
    if (lineEnd == lineStart)
      continue;
    if (!ParseRequestLine(lineStart, lineEnd))
      return false;
    state_ = ParserState::Headers;  // move to next state
    return true;
  }
}

/*If the message has Transfer-Encoding (and it includes chunked),
then you must not use Content-Length to delimit the body.
ignore Content-Length for body-length decisions.*/
void HTTPParser::UpdateState(void)
{
  if (req_.IsChunked())
  {
    state_ = ParserState::Chunked;
    return;
  }
  else if (auto len = req_.GetContentLength())
  {
    contentLength_ = *len;
    // TODO(VALIDATE): content length checks (413 vs 400)
    state_ = (contentLength_ > 0) ? ParserState::Body : ParserState::Done;
    return;
  }
  state_ = ParserState::Done;
}

/*
    @brief Parses the headers of the HTTP request
    @return ParseProgress::NeedMoreData if need more data,
    ParseProgress::Advance if made progress or terminal state(Done / Error)
    @note A server MUST reject any received request message that contains
    whitespace between a header field-name and colon with a response code of 400 (Bad Request).
*/
bool HTTPParser::ParseHeaders(void)
{
  while (true)
  {
    size_t lineStart = pos_;
    size_t lineEnd = 0;

    if (!ConsumeLine(buffer_, pos_, lineEnd))
      return false;

    if (lineEnd == lineStart)
    {
      UpdateState();
      return true;
    }

    if (!ParseHeaderLine(lineStart, lineEnd))
      return false;
  }
}

// read exactly contentLength bytes from the stream
// TODO(VALIDATE): max body size -> 413
// TODO(VALIDATE): reject Content-Length for GET/HEAD?
// TODO(VALIDATE): handle multiple Content-Length
bool HTTPParser::ParseBody(void)
{
  const size_t available = buffer_.size() - pos_;
  const size_t remaining = contentLength_ - bodyRead_;

  if (remaining == 0)
  {
    state_ = ParserState::Done;
    return true;
  }

  if (available == 0)
    return false;
  size_t take = std::min(available, remaining);
  req_.AppendBody(std::string_view(buffer_.data() + pos_, take));
  pos_ += take;
  bodyRead_ += take;
  if (bodyRead_ == contentLength_)
    state_ = ParserState::Done;

  return true;
}

bool HTTPParser::ParseChunked(void)
{
  while (true)
  {
    switch (chunk_.phase)
    {
      case ChunkState::Phase::SizeLine:
        if (!ParseChunkSize())
          return false;
        break;

      case ChunkState::Phase::Data:
        if (!ParseChunkData())
          return false;
        break;

      case ChunkState::Phase::DataCRLF:
        if (!ParseChunkCRLF())
          return false;
        chunk_.phase = ChunkState::Phase::SizeLine;
        break;

      case ChunkState::Phase::Trailer:
        if (!ParseChunkTrailer())
          return false;
        break;

      case ChunkState::Phase::Done:
        state_ = ParserState::Done;
        return true;
    }
  }
}

HTTPParser::ParseResult HTTPParser::ExitResult(void) const
{
  if (state_ == ParserState::Done)
    return ParseResult::Done;

  if (state_ == ParserState::Error)
    return ParseResult::Error;

  return ParseResult::NeedMoreData;
}

/*
    FSM: https://github.com/nodejs/http-parser
    FSM (finite state machine) with chunk-based parsing, not a byte-by-byte FSM
    HSM:https://natsys-lab.blogspot.com/2014/11/the-fast-finite-state-machine-for-http.html (state within state)
    My approach: Top-level FSM(parent) -> ParserState has semantic meaning
    Nested FSM inside Chunked (child) -> only exists while ParserState::Chunked is active. When it is done, control
    returns to the parent FSM.
    inspired by https://www.boost.org/doc/libs/latest/libs/beast/doc/html/beast/using_http/parser_stream_operations.html
    using incremental parsers, parse incrementally and can stopo when buffer dont contain enough data
    keep the states, transition decisions are based on line boundaries not per byte
    read start-line -> headers -> empty line -> body
    ParserState: the current state of the parser, keep track of the boundaries
    ParseProgress: advance vs need more data, indicate what actions to be taken next
*/
HTTPParser::ParseResult HTTPParser::Parse(std::string_view input)
{
  buffer_.append(input);

  if (state_ == ParserState::StartLine && !ParseStartLine())
    return ExitResult();
  if (state_ == ParserState::Headers && !ParseHeaders())
    return ExitResult();
  if (state_ == ParserState::Body && !ParseBody())
    return ExitResult();
  if (state_ == ParserState::Chunked && !ParseChunked())
    return ExitResult();
  return ExitResult();
}
