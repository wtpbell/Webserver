/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPParser.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/02 15:28:21 by bewong        #+#    #+#                 */
/*   Updated: 2026/04/02 16:44:32 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

#include "http/BodySink.hpp"
#include "http/HTTPRequest.hpp"
#include "http/HTTPValidator.hpp"

class HTTPParser
{
  public:
    enum class ParserState
    {
      kStartLine,
      kHeaders,
      kAwaitBodyDecision,
      kBody,
      kChunked,
      kDone,
      kError
    };

    enum class ParseResult
    {
      kNeedMoreData,
      kHeadersDone,
      kDone,
      kError
    };

    struct ChunkState
    {
        enum class Phase
        {
          kSizeLine,
          kData,
          kDataCRLF,
          kTrailer,
          kDone
        };
        Phase phase = Phase::kSizeLine;
        std::size_t remaining = 0;
    };

    HTTPParser(void) = default;
    HTTPParser(const HTTPParser&) = delete;
    HTTPParser& operator=(const HTTPParser&) = delete;
    HTTPParser(HTTPParser&&) noexcept = default;
    HTTPParser& operator=(HTTPParser&&) noexcept = default;
    ~HTTPParser(void) = default;

    ParseResult Parse(std::string_view data);
    ParseResult ExitResult(void);
    bool IsComplete(void) const;
    bool HasError(void) const;

    void SetNoBody();
    void SetContentLength(std::size_t n);
    void SetChunked();
    void SetMaxBody(std::size_t n);
    void SetBodySink(std::unique_ptr<IBodySink> sink);
    bool NeedsBodyDecision() const;

    const HTTPRequest& GetRequest(void) const;
    HTTPRequest& GetRequestMutable(void);
    ValidationResult GetError(void) const;
    HTTPRequest TakeRequest(void);
    void ResetNextRequest(void);
    void Reset(void);

  private:
    bool ParseStartLine(void);
    bool ParseHeaders(void);
    bool ParseBody(void);
    bool ParseChunked(void);

    // for Parsing Chunk
    bool ParseChunkSize(void);
    bool ParseChunkData(void);
    bool ParseChunkCRLF(void);
    bool ParseChunkTrailer(void);

    bool ConsumeLine(const std::string& buf, size_t& pos, size_t& lineEnd);
    bool ParseRequestLine(const size_t lineStart, size_t lineEnd);
    bool ParseHeaderLine(const size_t lineStart, size_t lineEnd);
    bool WriteBodyChunk(std::string_view chunk);

    void Fail(ValidationResult vr = ValidationResult::kBadRequest);

    HTTPRequest req_;
    ParserState state_{ParserState::kStartLine};
    ValidationResult error_ = ValidationResult::kOk;
    std::unique_ptr<IBodySink> bodySink_;
    std::size_t maxBody_{0};
    std::size_t bodyBytes_{0};

    std::string buffer_;
    std::size_t pos_{0};

    std::size_t headerStart_{0};
    std::size_t contentLength_{0};
    std::size_t bodyRead_{0};
    ChunkState chunk_;
};

#endif  // HTTPPARSER_HPP
