/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   BodySink.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/03/17 10:03:50 by bewong        #+#    #+#                 */
/*   Updated: 2026/03/17 10:03:50 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef BODYSINK_HPP
#define BODYSINK_HPP

#include <filesystem>
#include <fstream>
#include <string_view>

#include "http/HTTPRequest.hpp"

struct IBodySink
{
    virtual ~IBodySink() = default;
    virtual bool Write(std::string_view chunk) = 0;
};

class MemorySink : public IBodySink
{
  public:
    explicit MemorySink(HTTPRequest& request) : req_(request) {}

    bool Write(std::string_view chunk) override
    {
      req_.AppendBody(chunk);
      return true;
    }

  private:
    HTTPRequest& req_;
};

class FileSink : public IBodySink
{
  public:
    using Path = std::filesystem::path;

    explicit FileSink(const Path& path) : path_(path), ofs_(path, std::ios::binary | std::ios::trunc) {}

    bool IsOpen() const
    {
      return ofs_.is_open();
    }

    bool Write(std::string_view chunk) override
    {
      ofs_.write(chunk.data(), static_cast<std::streamsize>(chunk.size()));
      return static_cast<bool>(ofs_);
    }

    const Path& GetPath() const
    {
      return path_;
    }

  private:
    Path path_;
    std::ofstream ofs_;
};
#endif
