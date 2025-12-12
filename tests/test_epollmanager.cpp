#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <atomic>
#include <csignal>
#include <cstring>
#include <stdexcept>
#include <thread>

#include "EpollManager.hpp"
#include "catch_amalgamated.hpp"
#include "exception/EPollManagerException.hpp"

static int makeNonBlocking(int fd)
{
  int flags = fcntl(fd, F_GETFL, 0);
  return (fcntl(fd, F_SETFL, flags | O_NONBLOCK));
}

static bool waitFor(std::atomic<bool>& flag, int ms = 200)
{
  auto start = std::chrono::steady_clock::now();
  while (!flag.load() &&
         std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() < ms)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return flag.load();
}

TEST_CASE("Basic Operations", "[EpollManager]")
{
  /* SETUP */
  std::atomic<int> n{0};

  EpollManager manager;
  int fds[2];
  REQUIRE(pipe(fds) == 0);

  int read_fd = fds[0];

  auto& callbacks = manager.GetCallbacks();
  REQUIRE(callbacks.size() == 0);
  /* END SETUP */

  SECTION("ADD")
  {
    std::atomic<bool> called{false};
    auto in_callback = [&](EpollManager&, const struct epoll_event& ev)
    {
      REQUIRE(ev.events & EPOLLIN);
      called = true;
      n = 1;
    };

    REQUIRE_NOTHROW(manager.AddFd(read_fd, EPOLLIN, in_callback));
    REQUIRE(callbacks.size() == 1);

    // check if a new entry has been added
    auto it = callbacks.find(read_fd);
    REQUIRE(it != callbacks.end());

    struct epoll_event fake_ev{};
    fake_ev.data.fd = read_fd;
    fake_ev.events = EPOLLIN;

    auto& callback = it->second;
    callback(manager, fake_ev);

    REQUIRE(called.load() == true);
    REQUIRE(n == 1);
  }

  SECTION("ADD WITH NULLPTR CALLBACK")
  {
    REQUIRE_NOTHROW(manager.AddFd(read_fd, EPOLLIN, nullptr));
    REQUIRE(callbacks.size() == 1);

    auto it = callbacks.find(read_fd);
    REQUIRE(it != callbacks.end());
    REQUIRE(it->second == nullptr);
  }

  SECTION("ADD DUPLICATE FD")
  {
    auto cb = [&](EpollManager&, const struct epoll_event&) { n = 1; };
    REQUIRE_NOTHROW(manager.AddFd(read_fd, EPOLLIN, cb));
    REQUIRE(callbacks.size() == 1);

    REQUIRE_THROWS_AS(manager.AddFd(read_fd, EPOLLIN, cb), ExistInEPollException);
    REQUIRE(errno == EEXIST);
  }

  SECTION("MOD FD EVENT")
  {
    auto cb_in = [&](EpollManager&, const struct epoll_event&) { n = 2; };
    auto cb_mod = [&](EpollManager&, const struct epoll_event&) { n = 4; };

    REQUIRE_NOTHROW(manager.AddFd(read_fd, EPOLLIN, cb_in));
    REQUIRE(callbacks.size() == 1);
    REQUIRE_NOTHROW(manager.ModifyFd(read_fd, EPOLLIN, cb_mod));

    auto it = callbacks.find(read_fd);
    REQUIRE(it != callbacks.end());

    struct epoll_event fake_ev{};
    fake_ev.data.fd = read_fd;
    fake_ev.events = EPOLLOUT;

    it->second(manager, fake_ev);
    REQUIRE(n == 4);
  }

  SECTION("MOD UNKNOWN FD")
  {
    auto cb_mod = [&](EpollManager&, const struct epoll_event&) { n = 4; };
    REQUIRE_NOTHROW(manager.AddFd(read_fd, EPOLLIN, cb_mod));
    REQUIRE(callbacks.size() == 1);

    REQUIRE_THROWS_AS(manager.ModifyFd(213, EPOLLIN, cb_mod), EPollManagerException);
    REQUIRE(callbacks.size() == 1);
  }

  SECTION("MOD WITH NULLPTR CALLBACK")
  {
    auto cb_in = [&](EpollManager&, const struct epoll_event&) { n = 1; };
    REQUIRE_NOTHROW(manager.AddFd(read_fd, EPOLLIN, cb_in));
    REQUIRE(callbacks.size() == 1);

    // modify with nullptr callback
    REQUIRE_NOTHROW(manager.ModifyFd(read_fd, EPOLLIN));
    REQUIRE(callbacks.size() == 1);
  }

  SECTION("REMOVE FD")
  {
    auto cb_in = [&](EpollManager&, const struct epoll_event&) { n = 1; };
    REQUIRE_NOTHROW(manager.AddFd(read_fd, EPOLLIN, cb_in));
    REQUIRE(callbacks.size() == 1);

    REQUIRE_NOTHROW(manager.RemoveFd(read_fd));
    REQUIRE(callbacks.size() == 0);
  }

  SECTION("REMOVE NON EXISTING FD")
  {
    auto cb_in = [&](EpollManager&, const struct epoll_event&) { n = 1; };
    REQUIRE_NOTHROW(manager.AddFd(read_fd, EPOLLIN, cb_in));
    REQUIRE(callbacks.size() == 1);

    REQUIRE_THROWS_AS(manager.RemoveFd(218), EPollManagerException);
    REQUIRE(callbacks.size() == 1);
  }

  close(read_fd);
}

TEST_CASE("Epoll EventLoop full run", "[EpollManager]")
{
  const std::size_t max_buf_size = 128;
  char buf[max_buf_size] = {0};

  EpollManager manager;

  int fds[2];
  REQUIRE(pipe(fds) == 0);

  int read_fd = fds[0];
  int write_fd = fds[1];

  REQUIRE(makeNonBlocking(read_fd) != -1);
  REQUIRE(makeNonBlocking(write_fd) != -1);

  std::atomic<bool> write_ready{false};
  std::atomic<bool> read_ready{false};

  auto write_callback = [&](EpollManager&, const struct epoll_event& ev)
  {
    REQUIRE(ev.events & EPOLLOUT);
    write_ready = true;
  };

  auto read_callback = [&](EpollManager&, const struct epoll_event& ev)
  {
    REQUIRE(ev.events & EPOLLIN);
    int n = read(read_fd, buf, max_buf_size);
    REQUIRE(n > 0);
    read_ready = true;
    g_shutdown = true;
  };

  REQUIRE_NOTHROW(manager.AddFd(write_fd, EPOLLOUT, write_callback));
  REQUIRE_NOTHROW(manager.AddFd(read_fd, EPOLLIN, read_callback));

  SECTION("READ/WRITE USING PIPE")
  {
    g_shutdown = false;
    std::thread loop_thread([&]() { manager.EventLoop(); });

    INFO("Waiting for write-ready (EPOLLOUT)...");
    REQUIRE(waitFor(write_ready));

    REQUIRE(write(write_fd, "dummy", 5) == 5);

    INFO("Waiting for read-ready (EPOLLIN)...");
    REQUIRE(waitFor(read_ready));

    loop_thread.join();
    using namespace Catch::Matchers;

    REQUIRE(write_ready.load() == true);
    REQUIRE(read_ready.load() == true);
    REQUIRE_THAT(std::string(buf), Equals("dummy", Catch::CaseSensitive::Yes));
  }

  close(read_fd);
  close(write_fd);
}

TEST_CASE("EpollManager destructor behavior", "[EpollManager]")
{
  std::atomic<int> callback_count{0};
  int fds[2] = {-1, -1};

  SECTION("Destroy manager after adding FDs")
  {
    {
      REQUIRE(pipe(fds) == 0);

      int read_fd = fds[0];
      int write_fd = fds[1];
      {
        EpollManager manager;

        auto callback = [&](EpollManager&, const struct epoll_event&) { callback_count++; };
        REQUIRE_NOTHROW(manager.AddFd(read_fd, EPOLLIN, callback));
        REQUIRE_NOTHROW(manager.AddFd(write_fd, EPOLLOUT, callback));
      }
      // Closing FDs would normally produce epoll events
      REQUIRE(close(read_fd) == -1);
      REQUIRE(close(write_fd) == -1);

      fds[0] = fds[1] = -1;
    }  // <-- manager destructor runs here

    REQUIRE(callback_count.load() == 0);
  }

  // Safety cleanup in case something failed
  if (fds[0] != -1)
    close(fds[0]);
  if (fds[1] != -1)
    close(fds[1]);
}
