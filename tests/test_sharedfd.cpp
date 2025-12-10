#include <sys/socket.h>

#include <utility>
#include <vector>

#include "Logger.hpp"
#include "SharedFD.hpp"
#include "catch_amalgamated.hpp"
#include "exception/FileDescriptorException.hpp"
#include "fcntl.h"

void RequireAllFdsClosed(std::vector<int>& open_fds)
{
  for (std::size_t fd = 0; fd < open_fds.size(); ++fd)
  {
    CAPTURE(fd, open_fds[fd]);
    REQUIRE(open_fds[fd] == 0);
  }
}

TEST_CASE("rule-of-five", "[sharedfd]")
{
  const char* dummy_file = "tests/test_files/dummy_rw";
  auto& open_fds = SharedFD::GetSharedCountVector();

  RequireAllFdsClosed(open_fds);

  SECTION("Empty constructor")
  {
    SharedFD fd;
    REQUIRE(fd.GetFD() == -1);
    fd.Reset();
    REQUIRE(fd.GetFD() == -1);
  }

  SECTION("Copy/move/assign from and to empty sharedfd")
  {
    SharedFD empty;
    SharedFD copy_fd{empty};
    REQUIRE(empty.GetFD() == -1);
    REQUIRE(copy_fd.GetFD() == -1);

    SharedFD copyassign_fd;
    copyassign_fd = empty;
    REQUIRE(empty.GetFD() == -1);
    REQUIRE(copyassign_fd.GetFD() == -1);

    SharedFD move_fd{std::move(empty)};
    REQUIRE(empty.GetFD() == -1);
    REQUIRE(move_fd.GetFD() == -1);

    SharedFD moveassign_fd;
    moveassign_fd = std::move(empty);
    REQUIRE(empty.GetFD() == -1);
    REQUIRE(moveassign_fd.GetFD() == -1);
  }

  SECTION("Copy/move/assign from an open sharedfd")
  {
    SharedFD dummy = SharedFD::Open(dummy_file, O_RDONLY, 0);
    REQUIRE(dummy.GetFD() != -1);
    REQUIRE(dummy.SharedCount() == 1);

    SECTION("Copy")
    {
      SharedFD copy{dummy};
      REQUIRE(copy.GetFD() == dummy.GetFD());
      REQUIRE(dummy.SharedCount() == 2);
      REQUIRE(copy.SharedCount() == 2);
    }

    SECTION("Move")
    {
      SharedFD tmp{dummy};
      SharedFD move{std::move(tmp)};

      REQUIRE(tmp.GetFD() == -1);
      REQUIRE(move.GetFD() == dummy.GetFD());
      REQUIRE(tmp.SharedCount() == 0);
      REQUIRE(move.SharedCount() == 2);
    }

    SECTION("Copy assignment")
    {
      SharedFD copy;
      REQUIRE(copy.GetFD() == -1);

      copy = dummy;
      REQUIRE(copy.GetFD() == dummy.GetFD());
      REQUIRE(copy.SharedCount() == 2);
      REQUIRE(dummy.SharedCount() == 2);
    }

    SECTION("Move assignment")
    {
      SharedFD tmp{dummy};
      SharedFD move;
      REQUIRE(move.GetFD() == -1);

      move = std::move(tmp);
      REQUIRE(tmp.GetFD() == -1);
      REQUIRE(move.GetFD() == dummy.GetFD());
      REQUIRE(move.SharedCount() == 2);
      REQUIRE(dummy.SharedCount() == 2);
    }

    REQUIRE(dummy.SharedCount() == 1);
  }

  RequireAllFdsClosed(open_fds);
}

TEST_CASE("SharedFD creation", "[sharedfd]")
{
  const char* dummy_file = "tests/test_files/dummy_rw";
  auto& open_fds = SharedFD::GetSharedCountVector();

  RequireAllFdsClosed(open_fds);

  SECTION("Open")
  {
    SharedFD dummy;
    REQUIRE_NOTHROW((dummy = SharedFD::Open(dummy_file, O_RDONLY, 0)));
    REQUIRE_FALSE(dummy.GetFD() == -1);

    SharedFD nofd;
    REQUIRE_THROWS_AS((nofd = SharedFD::Open("nothing_to_see_here", O_RDONLY, 0)), FileDescriptorException);
  }

  SECTION("Socket")
  {
    SharedFD socket;
    REQUIRE_NOTHROW((socket = SharedFD::Socket(AF_UNIX, SOCK_DGRAM, 0)));
    REQUIRE_FALSE(socket.GetFD() == -1);

    SharedFD nosocket;
    REQUIRE_THROWS_AS((nosocket = SharedFD::Socket(999, SOCK_DGRAM, 0)), FileDescriptorException);
  }

  SECTION("SocketPair")
  {
    std::pair<SharedFD, SharedFD> socketpair;
    REQUIRE_NOTHROW((socketpair = SharedFD::SocketPair(AF_UNIX, SOCK_DGRAM, 0)));
    REQUIRE_FALSE(socketpair.first.GetFD() == -1);
    REQUIRE_FALSE(socketpair.second.GetFD() == -1);
    REQUIRE_FALSE(socketpair.first.GetFD() == socketpair.second.GetFD());

    std::pair<SharedFD, SharedFD> nosocketpair;
    REQUIRE_THROWS_AS((nosocketpair = SharedFD::SocketPair(999, SOCK_DGRAM, 0)), FileDescriptorException);
  }

  SECTION("Pipe")
  {
    std::pair<SharedFD, SharedFD> pipe;
    REQUIRE_NOTHROW((pipe = SharedFD::Pipe()));
    REQUIRE_FALSE(pipe.first.GetFD() == -1);
    REQUIRE_FALSE(pipe.second.GetFD() == -1);
    REQUIRE_FALSE(pipe.first.GetFD() == pipe.second.GetFD());
  }

  SECTION("Pipe2")
  {
    std::pair<SharedFD, SharedFD> pipe2;
    REQUIRE_NOTHROW((pipe2 = SharedFD::Pipe2(O_NONBLOCK)));
    REQUIRE_FALSE(pipe2.first.GetFD() == -1);
    REQUIRE_FALSE(pipe2.second.GetFD() == -1);
    REQUIRE_FALSE(pipe2.first.GetFD() == pipe2.second.GetFD());
  }

  SECTION("Dup")
  {
    SharedFD dup;
    SharedFD dummy = SharedFD::Open(dummy_file, O_RDONLY);
    REQUIRE_NOTHROW((dup = SharedFD::Dup(dummy)));
    REQUIRE_FALSE(dup.GetFD() == dummy.GetFD());
    REQUIRE(dup.SharedCount() == 1);
    REQUIRE(dummy.SharedCount() == 1);

    SharedFD empty;
    SharedFD another_dup;
    REQUIRE_THROWS_AS((another_dup = SharedFD::Dup(empty)), FileDescriptorException);
  }

  SECTION("Dup2")
  {
    SharedFD dup2;
    SharedFD dummy{SharedFD::Open(dummy_file, O_RDONLY)};

    REQUIRE_NOTHROW((dup2 = SharedFD::Dup2(dummy, 99)));
    REQUIRE_FALSE(dummy.GetFD() == dup2.GetFD());
    REQUIRE(dup2.GetFD() == 99);
    REQUIRE(dummy.SharedCount() == 1);
    REQUIRE(dup2.SharedCount() == 1);

    SharedFD empty;
    SharedFD nodup2;
    REQUIRE_THROWS_AS(SharedFD::Dup2(dummy, empty), FileDescriptorException);
    REQUIRE_FALSE(dummy.GetFD() == -1);
    REQUIRE(dummy.SharedCount() == 1);

    SharedFD copy_dummy{dummy};
    REQUIRE_THROWS_AS(SharedFD::Dup2(dummy, copy_dummy), FileDescriptorException);
    REQUIRE(copy_dummy.GetFD() == dummy.GetFD());
    REQUIRE(copy_dummy.SharedCount() == dummy.SharedCount());

    SharedFD second_dummy{SharedFD::Open(dummy_file, O_RDONLY)};
    int cached_fd = second_dummy.GetFD();
    REQUIRE_NOTHROW(SharedFD::Dup2(dummy, second_dummy));
    REQUIRE(second_dummy.GetFD() == cached_fd);
    REQUIRE(second_dummy.SharedCount() == 1);
    REQUIRE(dummy.SharedCount() == 2);

    SharedFD third_dummy;
    REQUIRE_THROWS_AS((third_dummy = SharedFD::Dup2(dummy, dummy.GetFD())), FileDescriptorException);
    REQUIRE(third_dummy.GetFD() == -1);
    REQUIRE_FALSE(dummy.GetFD() == -1);
    REQUIRE(dummy.SharedCount() == 2);
  }

  RequireAllFdsClosed(open_fds);
}

TEST_CASE("SharedFD counter/Clean up", "[sharedfd]")
{
  const char* dummy_file = "tests/test_files/dummy_rw";
  auto& open_fds = SharedFD::GetSharedCountVector();

  RequireAllFdsClosed(open_fds);

  SECTION("Create copies out of dummy")
  {
    SharedFD dummy{SharedFD::Open(dummy_file, O_RDONLY)};
    SECTION("n copies")
    {
      int count = GENERATE(1, 5, 10, 20, 40, 80, 120, 240);
      std::vector<SharedFD> fds;
      fds.reserve(count);

      for (int i = 0; i < count; ++i)
        fds.emplace_back(SharedFD(dummy));
      CAPTURE(dummy.SharedCount(), count);
      REQUIRE(dummy.SharedCount() == (count + 1));

      fds.erase(fds.begin());
      REQUIRE(dummy.SharedCount() == count);

      int removal = count >> 1;
      fds.erase(fds.begin(), fds.begin() + removal);
      REQUIRE(dummy.SharedCount() == (count - removal));
    }
  }

  SECTION("Open many fds")
  {
    int count = GENERATE(1, 5, 10, 20, 40, 80, 120, 240);
    std::vector<SharedFD> fds;
    fds.reserve(count);

    for (int i = 0; i < count; ++i)
      fds.emplace_back(SharedFD::Open(dummy_file, O_RDONLY));

    int open_file_count{};
    for (int n : open_fds)
      if (n > 0)
        ++open_file_count;

    REQUIRE(open_file_count == count);
  }

  RequireAllFdsClosed(open_fds);
}

TEST_CASE("Error Handling", "[sharedfd]")
{
  auto& open_fds = SharedFD::GetSharedCountVector();
  RequireAllFdsClosed(open_fds);

  // Test opening non-existent file
  SECTION("Open non-existent file")
  {
    REQUIRE_THROWS_AS(SharedFD::Open("non_existent_file.txt", O_RDONLY), FileDescriptorException);
  }

  // Test invalid socket parameters
  SECTION("Invalid socket parameters")
  {
    REQUIRE_THROWS_AS(SharedFD::Socket(-1, -1, -1),  // Invalid parameters
                      FileDescriptorException);
  }

  // Test invalid pipe creation
  SECTION("Invalid pipe creation")
  {
    // Test with invalid flags
    REQUIRE_THROWS_AS(SharedFD::Pipe2(-1),  // Invalid flags
                      FileDescriptorException);
  }

  RequireAllFdsClosed(open_fds);
}
