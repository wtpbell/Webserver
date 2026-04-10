#include <iterator>

#include "catch_amalgamated.hpp"
#include "http/BodySink.hpp"
#include "http/HTTPRequest.hpp"

namespace fs = std::filesystem;

TEST_CASE("BodySink - MemorySink", "[BodySink][MemorySink]")
{
  HTTPRequest request;
  MemorySink sink(request);

  REQUIRE(sink.Write("Write into memory sink. "));
  REQUIRE(sink.Write("Write more into memory sink."));

  REQUIRE(request.GetBody() == "Write into memory sink. Write more into memory sink.");
}

TEST_CASE("BodySink - FileSink", "[BodySink][FileSink]")
{
  fs::path path = "test_upload.tmp";

  {
    FileSink sink(path);
    REQUIRE(sink.IsOpen());
    REQUIRE(sink.Write("Hello, "));
    REQUIRE(sink.Write("world!"));
  }

  REQUIRE(fs::exists(path));
  REQUIRE(fs::file_size(path) == 13);

  std::ifstream ifs(path, std::ios::binary);
  REQUIRE(ifs.is_open());

  std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

  REQUIRE(content == "Hello, world!");

  fs::remove(path);
}

TEST_CASE("FileSink fails to open invalid path")
{
  fs::path invalid = "/this/path/should/not/exist/file.tmp";

  FileSink sink(invalid);

  REQUIRE_FALSE(sink.IsOpen());
}

TEST_CASE("BodySink - FileSink truncates existing file", "[BodySink][FileSink]")
{
  fs::path path = "test_upload_trunc.tmp";

  {
    std::ofstream ofs(path, std::ios::binary);
    ofs << "OLD";
  }

  {
    FileSink sink(path);
    REQUIRE(sink.IsOpen());
    REQUIRE(sink.Write("NEW"));
  }

  std::ifstream ifs(path, std::ios::binary);
  std::string content((std::istreambuf_iterator<char>(ifs)), {});
  REQUIRE(content == "NEW");

  fs::remove(path);
}

TEST_CASE("BodySink - FileSink writes binary including null bytes", "[BodySink][FileSink]")
{
  fs::path path = "test_upload_bin.tmp";

  const std::string data = std::string("A\0B\0C", 5);

  {
    FileSink sink(path);
    REQUIRE(sink.IsOpen());
    REQUIRE(sink.Write(std::string_view(data.data(), data.size())));
  }

  std::ifstream ifs(path, std::ios::binary);
  std::string content((std::istreambuf_iterator<char>(ifs)), {});
  REQUIRE(content.size() == data.size());
  REQUIRE(content == data);

  fs::remove(path);
}

TEST_CASE("BodySink - FileSink accepts empty chunk", "[BodySink][FileSink]")
{
  fs::path path = "test_upload_empty.tmp";

  {
    FileSink sink(path);
    REQUIRE(sink.IsOpen());
    REQUIRE(sink.Write(""));
  }

  std::ifstream ifs(path, std::ios::binary);
  std::string content((std::istreambuf_iterator<char>(ifs)), {});
  REQUIRE(content.empty());

  fs::remove(path);
}

TEST_CASE("BodySink - MemorySink supports binary including null bytes", "[BodySink][MemorySink]")
{
  HTTPRequest request;
  MemorySink sink(request);

  const std::string data = std::string("X\0Y", 3);
  REQUIRE(sink.Write(std::string_view(data.data(), data.size())));

  REQUIRE(request.GetBody().size() == data.size());
  REQUIRE(request.GetBody() == data);
}

TEST_CASE("FileSink fails if parent directory doesn't exist", "[BodySink][FileSink]")
{
  fs::path invalid = "no_such_dir/file.tmp";
  FileSink sink(invalid);
  REQUIRE_FALSE(sink.IsOpen());
}
