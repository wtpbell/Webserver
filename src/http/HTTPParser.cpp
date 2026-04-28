/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPParser.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/09 17:14:02 by bewong        #+#    #+#                 */
/*   Updated: 2026/04/02 15:48:04 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "http/HTTPParser.hpp"

#include <algorithm>
#include <cstdlib>
#include <string>
#include <string_view>

#include "Logger.hpp"
#include "http/BodySink.hpp"
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
  bool ReadTokenSP(const std::string& s, std::size_t& i, size_t end, std::size_t& tokBegin, size_t& tokEnd)
  {
    if (i >= end)
      return false;
    tokBegin = i;
    std::size_t sp = s.find(' ', i);
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

bool HTTPParser::IsComplete() const
{
  return state_ == ParserState::kDone;
}

bool HTTPParser::HasError() const
{
  return state_ == ParserState::kError;
}

void HTTPParser::Fail(ValidationResult vr)
{
  error_ = vr;
  state_ = ParserState::kError;
}

/************************************************** Getter ***********************************************************/

const HTTPRequest& HTTPParser::GetRequest() const
{
  return req_;
}

ValidationResult HTTPParser::GetError() const
{
  return error_;
}

HTTPRequest& HTTPParser::GetRequestMutable()
{
  return req_;
}

void HTTPParser::Reset()
{
  req_ = HTTPRequest();
  state_ = ParserState::kStartLine;
  buffer_.clear();
  pos_ = 0;
  contentLength_ = 0;
  bodyRead_ = 0;
  chunk_ = ChunkState{};
  error_ = ValidationResult::kOk;
  headerStart_ = 0;

  bodySink_.reset();
  maxBody_ = 0;
  bodyBytes_ = 0;
}

HTTPRequest HTTPParser::TakeRequest()
{
  HTTPRequest request = std::move(req_);
  state_ = ParserState::kDone;
  return request;
}

/************************************************** Setter ***********************************************************/

bool HTTPParser::NeedsBodyDecision() const
{
  return state_ == ParserState::kAwaitBodyDecision;
}

void HTTPParser::SetNoBody()
{
  // only valid after headers end
  if (state_ != ParserState::kAwaitBodyDecision)
    return (Fail(ValidationResult::kBadRequest), void());

  state_ = ParserState::kDone;
}

void HTTPParser::SetContentLength(std::size_t n)
{
  if (state_ != ParserState::kAwaitBodyDecision)
    return (Fail(ValidationResult::kBadRequest), void());

  if (maxBody_ > 0 && n > maxBody_)
    return (Fail(ValidationResult::kPayloadTooLarge), void());

  contentLength_ = n;
  bodyRead_ = 0;
  state_ = (contentLength_ > 0) ? ParserState::kBody : ParserState::kDone;
}

void HTTPParser::SetChunked()
{
  if (state_ != ParserState::kAwaitBodyDecision)
    return (Fail(ValidationResult::kBadRequest), void());

  chunk_ = ChunkState{};
  state_ = ParserState::kChunked;
}

void HTTPParser::SetMaxBody(std::size_t n)
{
  maxBody_ = n;
}

void HTTPParser::SetBodySink(std::unique_ptr<IBodySink> sink)
{
  bodySink_ = std::move(sink);
}

/************************************************ Parse lines *********************************************************/
/*
    @brief Consumes a line from the buffer
    @param buf The buffer contining the HTTP request
    @param pos The position to start consuming from
    @param lineEnd The position of the kCRLF
    @return true if the line was consumed, false otherwise
*/
bool HTTPParser::ConsumeLine(const std::string& buf, std::size_t& pos, std::size_t& lineEnd)
{
  lineEnd = buf.find(HTTP::kCRLF, pos);
  if (lineEnd == std::string::npos)
    return false;
  pos = lineEnd + 2;
  return true;
}
// request-line   = method SP request-target SP HTTP-version kCRLF
// GET http://example.com?foo HTTP/1.1
bool HTTPParser::ParseRequestLine(const std::size_t lineStart, std::size_t lineEnd)
{
  std::size_t mBegin, mEnd, tBegin, tEnd;
  std::size_t i = lineStart;

  if (!ReadTokenSP(buffer_, i, lineEnd, mBegin, mEnd) || !ReadTokenSP(buffer_, i, lineEnd, tBegin, tEnd) ||
      i >= lineEnd)
    return (Fail(), false);

  std::string_view method(buffer_.data() + mBegin, mEnd - mBegin);
  std::string_view target(buffer_.data() + tBegin, tEnd - tBegin);
  std::string_view version(buffer_.data() + i, lineEnd - i);
  if (version.find(' ') != std::string::npos)
    return (Fail(), false);
  // TODO(VALIDATE): method support checks (501 vs 400)
  // if (!HTTPValidation::Method(method)) return Fail();
  req_.SetMethod(method);
  if (!req_.SetTarget(target))
    return (Fail(), false);
  req_.SetVersion(std::string(version));
  return true;
}

// Header-Name: value\r\n
// Host: example.com \r\n
// Content-Type: text/html \r\n
// Content-Length: 42 \r\n
// \r\n
bool HTTPParser::ParseHeaderLine(const std::size_t lineStart, std::size_t lineEnd)
{
  std::size_t colon = buffer_.find(':', lineStart);
  if (colon == std::string::npos || colon >= lineEnd)
    return (Fail(), false);

  for (std::size_t i = lineStart; i < colon; ++i)
  {
    if (buffer_[i] == ' ' || buffer_[i] == '\t')
      return (Fail(), false);
  }

  std::string_view name(buffer_.data() + lineStart, colon - lineStart);
  std::string_view value(buffer_.data() + (colon + 1), lineEnd - (colon + 1));
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
bool HTTPParser::ParseChunkSize()
{
  std::size_t lineEnd;
  std::size_t start = pos_;

  if (!ConsumeLine(buffer_, pos_, lineEnd))
    return (false);
  std::string_view line(buffer_.data() + start, lineEnd - start);
  // can accept chunk extentsions A;foo=bar\r\n
  auto semi = line.find(';');
  if (semi != std::string_view::npos)
    line.remove_suffix(line.size() - semi);
  line = String::Trim(line);
  if (line.empty())
    return false;
  // parse hex
  std::size_t size{0};
  if (!String::ConvertToNumber(line, size, 16))
    return (Fail(), false);  // invalid chunk size

  chunk_.remaining = size;
  chunk_.phase = (size == 0) ? ChunkState::Phase::kTrailer : ChunkState::Phase::kData;

  return true;
}

bool HTTPParser::ParseChunkData()
{
  if (buffer_.size() - pos_ < chunk_.remaining)
    return false;

  if (!WriteBodyChunk(std::string_view(buffer_.data() + pos_, chunk_.remaining)))
    return false;

  pos_ += chunk_.remaining;
  chunk_.remaining = 0;
  chunk_.phase = ChunkState::Phase::kDataCRLF;
  return true;
}

bool HTTPParser::ParseChunkCRLF()
{
  if (buffer_.size() - pos_ < 2)
    return false;
  if (buffer_[pos_] != '\r' || buffer_[pos_ + 1] != '\n')
    return (Fail(ValidationResult::kBadRequest), false);

  pos_ += 2;
  return true;
}

bool HTTPParser::ParseChunkTrailer()
{
  // We are at the start of trailer section after "0\r\n".
  // Trailer section ends with an empty line "\r\n".

  while (true)
  {
    std::size_t lineEnd = 0;
    std::size_t lineStart = pos_;

    if (!ConsumeLine(buffer_, pos_, lineEnd))
      return false;  // need more data

    if (lineEnd == lineStart)
    {
      // empty line => end of trailers
      chunk_.phase = ChunkState::Phase::kDone;
      return true;
    }

    return (Fail(ValidationResult::kBadRequest), false);
  }
}

/************************************************* Parser ************************************************************/

/*
    @brief Parses the start line of the HTTP request
    @return ParseProgress::kNeedMoreData if need more data,
    ParseProgress::Advance if made progress or terminal state(kDone / Error)
    @note It is RECOMMENDED that all HTTP senders and recipients support, at a minimum, request-line lengths of 8000
   octets.
    @note nginx default is ~8k–16k depending on build
    @note if there is no kCRLF, check if the request line is too long
*/
bool HTTPParser::ParseStartLine()
{
  while (true)
  {
    std::size_t lineEnd = 0;
    const std::size_t lineStart = pos_;

    if (!ConsumeLine(buffer_, pos_, lineEnd))  // if there is no kCRLF
    {
      if (buffer_.size() - lineStart > HTTP::kMaxRequestLine)  // if the request line is too long
      {
        Fail(ValidationResult::kURITooLong);
        return false;
      }
      return false;  // is incomplete, need more data
    }
    // skip leading empty lines (RFC-compliant)
    if (lineEnd == lineStart)
      continue;
    if (!ParseRequestLine(lineStart, lineEnd))
      return false;
    state_ = ParserState::kHeaders;  // move to next state
    headerStart_ = pos_;
    return true;
  }
}

/*
    @brief Parses the headers of the HTTP request
    @return ParseProgress::NeedMoreData if need more data,
    ParseProgress::Advance if made progress or terminal state(kDone / Error)
    @note A server MUST reject any received request message that contains
    whitespace between a header field-name and colon with a response code of 400 (Bad Request).
*/
bool HTTPParser::ParseHeaders()
{
  while (true)
  {
    std::size_t lineStart = pos_;
    std::size_t lineEnd = 0;

    if (!ConsumeLine(buffer_, pos_, lineEnd))
    {
      if (buffer_.size() - headerStart_ > HTTP::kMaxHeaderSize)
        Fail(ValidationResult::kPayloadTooLarge);
      return false;
    }
    if (lineEnd - headerStart_ > HTTP::kMaxHeaderSize)
    {
      Fail(ValidationResult::kPayloadTooLarge);
      return false;
    }
    if (lineEnd == lineStart)
    {
      state_ = ParserState::kAwaitBodyDecision;
      return true;
    }

    if (!ParseHeaderLine(lineStart, lineEnd))
      return false;
  }
}

bool HTTPParser::WriteBodyChunk(std::string_view chunk)
{
  if (maxBody_ > 0 && bodyBytes_ + chunk.size() > maxBody_)
    return (Fail(ValidationResult::kPayloadTooLarge), false);

  if (bodySink_)
  {
    if (!bodySink_->Write(chunk))
    {
      state_ = ParserState::kError;
      return false;
    }
  }
  else
    req_.AppendBody(chunk);

  bodyBytes_ += chunk.size();
  return true;
}

// read exactly contentLength bytes from the stream
bool HTTPParser::ParseBody()
{
  const std::size_t available = buffer_.size() - pos_;
  const std::size_t remaining = contentLength_ - bodyRead_;

  if (remaining == 0)
  {
    state_ = ParserState::kDone;
    return true;
  }
  if (available == 0)
    return false;

  const std::size_t take = std::min(available, remaining);
  if (!WriteBodyChunk(std::string_view(buffer_.data() + pos_, take)))
    return false;

  pos_ += take;
  bodyRead_ += take;

  if (bodyRead_ == contentLength_)
    state_ = ParserState::kDone;

  return true;
}

bool HTTPParser::ParseChunked()
{
  while (true)
  {
    switch (chunk_.phase)
    {
      case ChunkState::Phase::kSizeLine:
        if (!ParseChunkSize())
          return false;
        break;

      case ChunkState::Phase::kData:
        if (!ParseChunkData())
          return false;
        break;

      case ChunkState::Phase::kDataCRLF:
        if (!ParseChunkCRLF())
          return false;
        chunk_.phase = ChunkState::Phase::kSizeLine;
        break;

      case ChunkState::Phase::kTrailer:
        if (!ParseChunkTrailer())
          return false;
        break;

      case ChunkState::Phase::kDone:
        state_ = ParserState::kDone;
        return true;
    }
  }
}

HTTPParser::ParseResult HTTPParser::ExitResult()
{
  if (state_ == ParserState::kDone)
  {
    req_.SetComplete(true);
    return ParseResult::kDone;
  }
  if (state_ == ParserState::kError)
    return ParseResult::kError;
  if (state_ == ParserState::kAwaitBodyDecision)
    return ParseResult::kHeadersDone;
  return ParseResult::kNeedMoreData;
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
*/
HTTPParser::ParseResult HTTPParser::Parse(std::string_view input)
{
  if (state_ == ParserState::kDone || state_ == ParserState::kError)
    return ExitResult();
  buffer_.append(input);
  if (state_ == ParserState::kStartLine && !ParseStartLine())
    return ExitResult();
  if (state_ == ParserState::kHeaders && !ParseHeaders())
    return ExitResult();
  if (state_ == ParserState::kAwaitBodyDecision)
    return ExitResult();
  if (state_ == ParserState::kBody && !ParseBody())
    return ExitResult();
  if (state_ == ParserState::kChunked && !ParseChunked())
    return ExitResult();
  return ExitResult();
}

void HTTPParser::ResetNextRequest()
{
  if (pos_ > 0)
    buffer_.erase(0, pos_);
  pos_ = 0;

  req_.HTTPRequest::Clear();

  state_ = ParserState::kStartLine;
  contentLength_ = 0;
  bodyRead_ = 0;
  chunk_ = ChunkState{};
  error_ = ValidationResult::kOk;
  headerStart_ = 0;

  bodySink_.reset();
  maxBody_ = 0;
  bodyBytes_ = 0;
}
