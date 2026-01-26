/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPParser.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/02 15:28:21 by bewong        #+#    #+#                 */
/*   Updated: 2026/01/13 19:06:04 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

#include <cstddef>
#include <string>
#include <string_view>

#include "http/HTTPRequest.hpp"

class HTTPParser
{
  public:
    enum class ParserState
    {
      StartLine,
      Headers,
      AwaitBodyDecision,
      Body,
      Chunked,
      Done,
      Error
    };

    enum class ParseResult
    {
      NeedMoreData,
      HeadersDone,
      Done,
      Error
    };

    struct ChunkState
    {
        enum class Phase
        {
          SizeLine,
          Data,
          DataCRLF,
          Trailer,
          Done
        };
        Phase phase = Phase::SizeLine;
        size_t remaining = 0;
    };

    HTTPParser(void) = default;
    HTTPParser(const HTTPParser&) = default;
    HTTPParser& operator=(const HTTPParser&) = default;
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
    bool NeedsBodyDecision() const;

    const HTTPRequest& GetRequest(void) const;
    HTTPRequest TakeRequest(void);
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
    void Fail(void);

    HTTPRequest req_;
    ParserState state_{ParserState::StartLine};
    std::string buffer_;
    std::size_t pos_{0};

    std::size_t contentLength_{0};
    std::size_t bodyRead_{0};
    ChunkState chunk_;
};

#endif  // HTTPPARSER_HPP
