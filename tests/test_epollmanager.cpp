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
#include "webserv.hpp"

static void InitPipe(SharedFD& left, SharedFD& right)
{
  try
  {
    auto p = SharedFD::Pipe2(O_NONBLOCK);
    left = std::move(p.first);
    right = std::move(p.second);
  }
  catch (...)
  {
    FAIL("InitPipe failed");
  }
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
  SharedFD read_fd, write_fd;
  InitPipe(read_fd, write_fd);

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

    REQUIRE_NOTHROW(manager.AddFd(read_fd.GetFD(), EPOLLIN, in_callback));
    REQUIRE(callbacks.size() == 1);

    // check if a new entry has been added
    auto it = callbacks.find(read_fd.GetFD());
    REQUIRE(it != callbacks.end());

    struct epoll_event fake_ev{};
    fake_ev.data.fd = read_fd.GetFD();
    fake_ev.events = EPOLLIN;

    auto& callback = it->second;
    callback(manager, fake_ev);

    REQUIRE(called.load() == true);
    REQUIRE(n == 1);
  }

  SECTION("ADD WITH NULLPTR CALLBACK")
  {
    REQUIRE_NOTHROW(manager.AddFd(read_fd.GetFD(), EPOLLIN, nullptr));
    REQUIRE(callbacks.size() == 1);

    auto it = callbacks.find(read_fd.GetFD());
    REQUIRE(it != callbacks.end());
    REQUIRE(it->second == nullptr);
  }

  SECTION("Modify by re-adding fd to poll")
  {
    struct epoll_event fake_ev{};
    auto cb = [&](EpollManager&, const struct epoll_event&)
    {
      n = 1;
    };
    auto cb_mod = [&](EpollManager&, const struct epoll_event&)
    {
      n = 4;
    };

    REQUIRE_NOTHROW(manager.AddFd(read_fd.GetFD(), EPOLLIN, cb));
    REQUIRE(callbacks.size() == 1);

    auto it = callbacks.find(read_fd.GetFD());
    REQUIRE(it != callbacks.end());

    it->second(manager, fake_ev);
    REQUIRE(n == 1);

    REQUIRE_NOTHROW(manager.AddFd(read_fd.GetFD(), EPOLLOUT, cb_mod));
    REQUIRE(callbacks.size() == 1);

    it = callbacks.find(read_fd.GetFD());
    REQUIRE(it != callbacks.end());

    it->second(manager, fake_ev);
    REQUIRE(n == 4);
  }

  SECTION("MOD FD EVENT")
  {
    auto cb_in = [&](EpollManager&, const struct epoll_event&)
    {
      n = 2;
    };
    auto cb_mod = [&](EpollManager&, const struct epoll_event&)
    {
      n = 4;
    };

    REQUIRE_NOTHROW(manager.AddFd(read_fd.GetFD(), EPOLLIN, cb_in));
    REQUIRE(callbacks.size() == 1);
    REQUIRE_NOTHROW(manager.ModifyFd(read_fd.GetFD(), EPOLLIN, cb_mod));

    auto it = callbacks.find(read_fd.GetFD());
    REQUIRE(it != callbacks.end());

    struct epoll_event fake_ev{};
    it->second(manager, fake_ev);
    REQUIRE(n == 4);
  }

  SECTION("MOD UNKNOWN FD")
  {
    auto cb_mod = [&](EpollManager&, const struct epoll_event&)
    {
      n = 4;
    };
    REQUIRE_NOTHROW(manager.AddFd(read_fd.GetFD(), EPOLLIN, cb_mod));
    REQUIRE(callbacks.size() == 1);

    REQUIRE_THROWS_AS(manager.ModifyFd(213, EPOLLIN, cb_mod), EPollManagerException);
    REQUIRE(callbacks.size() == 1);
  }

  SECTION("MOD WITH NULLPTR CALLBACK")
  {
    auto cb_in = [&](EpollManager&, const struct epoll_event&)
    {
      n = 1;
    };
    REQUIRE_NOTHROW(manager.AddFd(read_fd.GetFD(), EPOLLIN, cb_in));
    REQUIRE(callbacks.size() == 1);

    // modify with nullptr callback
    REQUIRE_NOTHROW(manager.ModifyFd(read_fd.GetFD(), EPOLLIN));
    REQUIRE(callbacks.size() == 1);
  }

  SECTION("REMOVE FD")
  {
    auto cb_in = [&](EpollManager&, const struct epoll_event&)
    {
      n = 1;
    };
    REQUIRE_NOTHROW(manager.AddFd(read_fd.GetFD(), EPOLLIN, cb_in));
    REQUIRE(callbacks.size() == 1);

    REQUIRE_NOTHROW(manager.RemoveFd(read_fd.GetFD()));
    REQUIRE(callbacks.size() == 0);
  }

  SECTION("REMOVE NON EXISTING FD")
  {
    auto cb_in = [&](EpollManager&, const struct epoll_event&)
    {
      n = 1;
    };
    REQUIRE_NOTHROW(manager.AddFd(read_fd.GetFD(), EPOLLIN, cb_in));
    REQUIRE(callbacks.size() == 1);

    REQUIRE_THROWS_AS(manager.RemoveFd(218), EPollManagerException);
    REQUIRE(callbacks.size() == 1);
  }
}

TEST_CASE("Epoll EventLoop full run", "[EpollManager]")
{
  const std::size_t max_buf_size = 128;
  char buf[max_buf_size] = {0};

  EpollManager manager;
  SharedFD read_fd, write_fd;
  InitPipe(read_fd, write_fd);

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
    int n = read(read_fd.GetFD(), buf, max_buf_size);
    REQUIRE(n > 0);
    read_ready = true;
    g_shutdown = true;
  };

  REQUIRE_NOTHROW(manager.AddFd(write_fd.GetFD(), EPOLLOUT, write_callback));
  REQUIRE_NOTHROW(manager.AddFd(read_fd.GetFD(), EPOLLIN, read_callback));

  SECTION("READ/WRITE USING PIPE")
  {
    g_shutdown = false;
    std::thread loop_thread(
        [&]()
        {
          manager.EventLoop();
        });

    INFO("Waiting for write-ready (EPOLLOUT)...");
    REQUIRE(waitFor(write_ready));

    REQUIRE(write(write_fd.GetFD(), "dummy", 5) == 5);

    INFO("Waiting for read-ready (EPOLLIN)...");
    REQUIRE(waitFor(read_ready));

    loop_thread.join();
    using namespace Catch::Matchers;

    REQUIRE(write_ready.load() == true);
    REQUIRE(read_ready.load() == true);
    REQUIRE_THAT(std::string(buf), Equals("dummy", Catch::CaseSensitive::Yes));
  }
}

TEST_CASE("Read byte by byte", "[EpollManager]")
{
  EpollManager manager;
  SharedFD read_fd, write_fd;
  InitPipe(read_fd, write_fd);

  g_shutdown = false;
  std::string reveived_message;

  auto write_callback = [&](EpollManager&, const struct epoll_event& ev)
  {
    REQUIRE(ev.events & EPOLLOUT);
    REQUIRE(write(write_fd.GetFD(), "Hello", 5) == 5);
  };

  auto read_callback = [&](EpollManager&, const struct epoll_event& ev)
  {
    REQUIRE(ev.events & EPOLLIN);

    char c;
    REQUIRE(read(read_fd.GetFD(), &c, 1) == 1);
    reveived_message.append(1, c);

    if (reveived_message.size() == 5)
      g_shutdown = true;
  };

  REQUIRE_NOTHROW(manager.AddFd(read_fd.GetFD(), EPOLLIN, read_callback));
  REQUIRE_NOTHROW(manager.AddFd(write_fd.GetFD(), EPOLLOUT, write_callback));

  REQUIRE_NOTHROW(manager.EventLoop());
  REQUIRE(reveived_message == "Hello");
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

        auto callback = [&](EpollManager&, const struct epoll_event&)
        {
          callback_count++;
        };
        REQUIRE_NOTHROW(manager.AddFd(read_fd, EPOLLIN, callback));
        REQUIRE_NOTHROW(manager.AddFd(write_fd, EPOLLOUT, callback));
      }  // <-- manager destructor runs here

      // Closing FDs would normally produce epoll events
      REQUIRE(close(read_fd) == 0);
      REQUIRE(close(write_fd) == 0);

      fds[0] = fds[1] = -1;
    }

    REQUIRE(callback_count.load() == 0);
  }

  // Safety cleanup in case something failed
  if (fds[0] != -1)
    close(fds[0]);
  if (fds[1] != -1)
    close(fds[1]);
}
