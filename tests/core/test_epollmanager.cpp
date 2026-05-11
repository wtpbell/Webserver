#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstring>
#include <string>
#include <thread>
#include <unordered_map>

#include "catch_amalgamated.hpp"
#include "core/EpollManager.hpp"
#include "utils/signal.hpp"

static void InitPipe(SharedFD& left, SharedFD& right)
{
  try
  {
    std::pair<SharedFD, SharedFD> p = SharedFD::Pipe2(O_NONBLOCK);
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
  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
  while (!flag.load() &&
         std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() < ms)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return flag.load();
}

TEST_CASE("Basic Operations", "[EpollManager]")
{
  std::atomic<int> n{0};

  EpollManager manager;
  REQUIRE(manager.Init() == EpollManager::Result::kOk);

  SharedFD read_fd, write_fd;
  InitPipe(read_fd, write_fd);

  std::unordered_map<int, EpollManager::EventCallback>& callbacks = manager.GetCallbacks();
  REQUIRE(callbacks.size() == 0);

  SECTION("ADD")
  {
    std::atomic<bool> called{false};
    auto in_callback = [&](EpollManager&, const struct epoll_event& ev)
    {
      REQUIRE((ev.events & EPOLLIN) == EPOLLIN);
      called = true;
      n = 1;
    };

    REQUIRE(manager.AddFd(read_fd.GetFD(), EPOLLIN, in_callback) == EpollManager::Result::kOk);
    REQUIRE(callbacks.size() == 1);

    std::unordered_map<int, EpollManager::EventCallback>::iterator it = callbacks.find(read_fd.GetFD());
    REQUIRE(it != callbacks.end());

    struct epoll_event fake_ev{};
    fake_ev.data.fd = read_fd.GetFD();
    fake_ev.events = EPOLLIN;

    EpollManager::EventCallback& callback = it->second;
    REQUIRE(static_cast<bool>(callback) == true);
    callback(manager, fake_ev);

    REQUIRE(called.load() == true);
    REQUIRE(n.load() == 1);
  }

  SECTION("ADD WITH EMPTY CALLBACK")
  {
    EpollManager::EventCallback empty_cb;
    REQUIRE(manager.AddFd(read_fd.GetFD(), EPOLLIN, empty_cb) == EpollManager::Result::kOk);
    REQUIRE(callbacks.size() == 1);

    std::unordered_map<int, EpollManager::EventCallback>::iterator it = callbacks.find(read_fd.GetFD());
    REQUIRE(it != callbacks.end());
    REQUIRE(static_cast<bool>(it->second) == false);
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

    REQUIRE(manager.AddFd(read_fd.GetFD(), EPOLLIN, cb) == EpollManager::Result::kOk);
    REQUIRE(callbacks.size() == 1);

    std::unordered_map<int, EpollManager::EventCallback>::iterator it = callbacks.find(read_fd.GetFD());
    REQUIRE(it != callbacks.end());

    it->second(manager, fake_ev);
    REQUIRE(n.load() == 1);

    REQUIRE(manager.AddFd(read_fd.GetFD(), EPOLLOUT, cb_mod) == EpollManager::Result::kOk);
    REQUIRE(callbacks.size() == 1);

    it = callbacks.find(read_fd.GetFD());
    REQUIRE(it != callbacks.end());

    it->second(manager, fake_ev);
    REQUIRE(n.load() == 4);
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

    REQUIRE(manager.AddFd(read_fd.GetFD(), EPOLLIN, cb_in) == EpollManager::Result::kOk);
    REQUIRE(callbacks.size() == 1);
    REQUIRE(manager.ModifyFd(read_fd.GetFD(), EPOLLIN, cb_mod) == EpollManager::Result::kOk);

    std::unordered_map<int, EpollManager::EventCallback>::iterator it = callbacks.find(read_fd.GetFD());
    REQUIRE(it != callbacks.end());

    struct epoll_event fake_ev{};
    it->second(manager, fake_ev);
    REQUIRE(n.load() == 4);
  }

  SECTION("MOD UNKNOWN FD")
  {
    auto cb_mod = [&](EpollManager&, const struct epoll_event&)
    {
      n = 4;
    };

    SharedFD extra_read, extra_write;
    InitPipe(extra_read, extra_write);

    REQUIRE(manager.AddFd(read_fd.GetFD(), EPOLLIN, cb_mod) == EpollManager::Result::kOk);
    REQUIRE(callbacks.size() == 1);

    REQUIRE(manager.ModifyFd(extra_read.GetFD(), EPOLLIN, cb_mod) == EpollManager::Result::kNotFound);
    REQUIRE(callbacks.size() == 1);
  }

  SECTION("MOD WITHOUT REPLACING CALLBACK")
  {
    auto cb_in = [&](EpollManager&, const struct epoll_event&)
    {
      n = 1;
    };

    REQUIRE(manager.AddFd(read_fd.GetFD(), EPOLLIN, cb_in) == EpollManager::Result::kOk);
    REQUIRE(callbacks.size() == 1);

    REQUIRE(manager.ModifyFd(read_fd.GetFD(), EPOLLIN) == EpollManager::Result::kOk);
    REQUIRE(callbacks.size() == 1);

    std::unordered_map<int, EpollManager::EventCallback>::iterator it = callbacks.find(read_fd.GetFD());
    REQUIRE(it != callbacks.end());
    REQUIRE(static_cast<bool>(it->second) == true);

    struct epoll_event fake_ev{};
    it->second(manager, fake_ev);
    REQUIRE(n.load() == 1);
  }

  SECTION("REMOVE FD")
  {
    auto cb_in = [&](EpollManager&, const struct epoll_event&)
    {
      n = 1;
    };

    REQUIRE(manager.AddFd(read_fd.GetFD(), EPOLLIN, cb_in) == EpollManager::Result::kOk);
    REQUIRE(callbacks.size() == 1);

    REQUIRE(manager.RemoveFd(read_fd.GetFD()) == EpollManager::Result::kOk);
    REQUIRE(callbacks.size() == 0);
  }

  SECTION("REMOVE NON EXISTING FD")
  {
    auto cb_in = [&](EpollManager&, const struct epoll_event&)
    {
      n = 1;
    };

    SharedFD extra_read, extra_write;
    InitPipe(extra_read, extra_write);

    REQUIRE(manager.AddFd(read_fd.GetFD(), EPOLLIN, cb_in) == EpollManager::Result::kOk);
    REQUIRE(callbacks.size() == 1);

    REQUIRE(manager.RemoveFd(extra_read.GetFD()) == EpollManager::Result::kNotFound);
    REQUIRE(callbacks.size() == 1);
  }

  SECTION("INIT TWICE RETURNS INVALID STATE")
  {
    REQUIRE(manager.Init() == EpollManager::Result::kInvalidState);
  }
}

TEST_CASE("Init succeeds once", "[EpollManager]")
{
  EpollManager manager;
  REQUIRE(manager.Init() == EpollManager::Result::kOk);
}

TEST_CASE("Init twice returns invalid state", "[EpollManager]")
{
  EpollManager manager;
  REQUIRE(manager.Init() == EpollManager::Result::kOk);
  REQUIRE(manager.Init() == EpollManager::Result::kInvalidState);
}

TEST_CASE("Epoll EventLoop full run", "[EpollManager]")
{
  const std::size_t max_buf_size = 128;
  char buf[max_buf_size] = {0};

  EpollManager manager;
  REQUIRE(manager.Init() == EpollManager::Result::kOk);

  SharedFD read_fd, write_fd;
  InitPipe(read_fd, write_fd);

  std::atomic<bool> write_ready{false};
  std::atomic<bool> read_ready{false};

  auto write_callback = [&](EpollManager&, const struct epoll_event& ev)
  {
    REQUIRE((ev.events & EPOLLOUT) == EPOLLOUT);
    write_ready = true;
  };

  auto read_callback = [&](EpollManager&, const struct epoll_event& ev)
  {
    REQUIRE((ev.events & EPOLLIN) == EPOLLIN);
    ssize_t nread = read(read_fd.GetFD(), buf, max_buf_size);
    REQUIRE(nread > 0);
    read_ready = true;
    g_shutdown = true;
  };

  REQUIRE(manager.AddFd(write_fd.GetFD(), EPOLLOUT, write_callback) == EpollManager::Result::kOk);
  REQUIRE(manager.AddFd(read_fd.GetFD(), EPOLLIN, read_callback) == EpollManager::Result::kOk);

  SECTION("READ/WRITE USING PIPE")
  {
    g_shutdown = false;

    EpollManager::Result loop_result = EpollManager::Result::kOk;
    std::thread loop_thread(
        [&]()
        {
          loop_result = manager.EventLoop();
        });

    INFO("Waiting for write-ready (EPOLLOUT)...");
    REQUIRE(waitFor(write_ready));

    REQUIRE(write(write_fd.GetFD(), "dummy", 5) == 5);

    INFO("Waiting for read-ready (EPOLLIN)...");
    REQUIRE(waitFor(read_ready));

    loop_thread.join();

    using namespace Catch::Matchers;
    REQUIRE(loop_result == EpollManager::Result::kOk);
    REQUIRE(write_ready.load() == true);
    REQUIRE(read_ready.load() == true);
    REQUIRE_THAT(std::string(buf), Equals("dummy", Catch::CaseSensitive::Yes));
  }
}

TEST_CASE("Read byte by byte", "[EpollManager]")
{
  EpollManager manager;
  REQUIRE(manager.Init() == EpollManager::Result::kOk);

  SharedFD read_fd, write_fd;
  InitPipe(read_fd, write_fd);

  g_shutdown = false;
  std::string received_message;

  auto write_callback = [&](EpollManager&, const struct epoll_event& ev)
  {
    REQUIRE((ev.events & EPOLLOUT) == EPOLLOUT);
    REQUIRE(write(write_fd.GetFD(), "Hello", 5) == 5);
  };

  auto read_callback = [&](EpollManager&, const struct epoll_event& ev)
  {
    REQUIRE((ev.events & EPOLLIN) == EPOLLIN);

    char c;
    REQUIRE(read(read_fd.GetFD(), &c, 1) == 1);
    received_message.append(1, c);

    if (received_message.size() == 5)
      g_shutdown = true;
  };

  REQUIRE(manager.AddFd(read_fd.GetFD(), EPOLLIN, read_callback) == EpollManager::Result::kOk);
  REQUIRE(manager.AddFd(write_fd.GetFD(), EPOLLOUT, write_callback) == EpollManager::Result::kOk);

  REQUIRE(manager.EventLoop() == EpollManager::Result::kOk);
  REQUIRE(received_message == "Hello");
}

TEST_CASE("EventLoop invalid state before Init", "[EpollManager]")
{
  EpollManager manager;
  REQUIRE(manager.EventLoop() == EpollManager::Result::kInvalidState);
}

TEST_CASE("Add/Modify/Remove error state before Init", "[EpollManager]")
{
  EpollManager manager;

  SharedFD read_fd, write_fd;
  InitPipe(read_fd, write_fd);

  auto cb = [&](EpollManager&, const struct epoll_event&) {};

  REQUIRE(manager.AddFd(read_fd.GetFD(), EPOLLIN, cb) == EpollManager::Result::kSyscallError);
  REQUIRE(manager.ModifyFd(read_fd.GetFD(), EPOLLIN) == EpollManager::Result::kSyscallError);
  REQUIRE(manager.ModifyFd(read_fd.GetFD(), EPOLLIN, cb) == EpollManager::Result::kSyscallError);
  REQUIRE(manager.RemoveFd(read_fd.GetFD()) == EpollManager::Result::kSyscallError);
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
        REQUIRE(manager.Init() == EpollManager::Result::kOk);

        auto callback = [&](EpollManager&, const struct epoll_event&)
        {
          callback_count++;
        };

        REQUIRE(manager.AddFd(read_fd, EPOLLIN, callback) == EpollManager::Result::kOk);
        REQUIRE(manager.AddFd(write_fd, EPOLLOUT, callback) == EpollManager::Result::kOk);
      }

      REQUIRE(close(read_fd) == 0);
      REQUIRE(close(write_fd) == 0);

      fds[0] = -1;
      fds[1] = -1;
    }

    REQUIRE(callback_count.load() == 0);
  }

  if (fds[0] != -1)
    close(fds[0]);
  if (fds[1] != -1)
    close(fds[1]);
}
