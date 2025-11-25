#include <fcntl.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <cstring>
#include <iostream>
#include <sstream>

#include "EpollManager.hpp"
#include "catch_amalgamated.hpp"

void eventLoopOnce(EpollManager& manager, int timeout_ms = 10)
{
  constexpr int MAX_EVENTS = 64;
  struct epoll_event events[MAX_EVENTS];
  sigset_t waitMask;

  if (sigemptyset(&waitMask) < 0)
    throw std::runtime_error("sigemptyset failed");
  sigaddset(&waitMask, SIGTSTP);

  int n = epoll_pwait(manager.getEpFd(), events, MAX_EVENTS, timeout_ms, &waitMask);
  if (n < 0)
  {
    if (errno == EINTR)
      return;
    throw std::runtime_error("epoll_pwait failed");
  }

  for (int i = 0; i < n; i++)
  {
    int fd = events[i].data.fd;
    auto it = manager.getCallbacks().find(fd);
    if (it != manager.getCallbacks().end())
      it->second(events[i].events);
    else
      throw std::runtime_error("Event received for fd not registered in callbacks_");
  }
}

void drain_fd(int fd)
{
  char buf[1024];
  ssize_t n;
  while ((n = read(fd, buf, sizeof(buf))) > 0)
  {
  }
}

int makeNonBlocking(int fd)
{
  int flags = fcntl(fd, F_GETFL, 0);
  return (fcntl(fd, F_SETFL, flags | O_NONBLOCK));
}

TEST_CASE("Basic Operations", "[EpollManager]")
{
  /* SETUP */
  int n = 0;

  EpollManager manager;
  int fds[2];
  auto& callbacks = manager.getCallbacks();
  auto in_callback = [&](uint32_t i) { n = i; };
  auto mod_callback = [&](uint32_t i) { n = i * i; };

  REQUIRE(pipe(fds) == 0);
  REQUIRE(callbacks.size() == 0);

  int read_fd = fds[0];
  int write_fd = fds[1];
  /* END SETUP */

  // Each section will execute separately the SETUP and TEARDOWN (even if the test fails) part of the code
  SECTION("ADD")
  {
    REQUIRE_NOTHROW(manager.addFd(read_fd, EPOLLIN, in_callback));
    REQUIRE(callbacks.size() == 1);

    // check if a new entry has been added
    auto it = callbacks.find(read_fd);
    REQUIRE(it != callbacks.end());

    // check if entry has the assigned values
    auto [fd, callback] = *it;
    REQUIRE(fd == read_fd);
    REQUIRE(callback != nullptr);

    // check if the callback has the correct behavior
    callback(1);
    REQUIRE(n == 1);
  }

  SECTION("ADD WITH NULLPTR CALLBACK")
  {
    REQUIRE_NOTHROW(manager.addFd(read_fd, EPOLLIN, nullptr));
    REQUIRE(callbacks.size() == 1);

    auto it = callbacks.find(read_fd);
    REQUIRE(it != callbacks.end());

    auto [fd, callback] = *it;
    REQUIRE(fd == read_fd);
    REQUIRE(callback == nullptr);
  }

  SECTION("ADD DUPLICATE FD")
  {
    REQUIRE_NOTHROW(manager.addFd(read_fd, EPOLLIN, in_callback));
    REQUIRE(callbacks.size() == 1);
    REQUIRE_THROWS_AS(manager.addFd(read_fd, EPOLLIN, in_callback), std::runtime_error);
    REQUIRE(errno == EEXIST);
  }

  SECTION("MOD FD EVENT")
  {
    REQUIRE_NOTHROW(manager.addFd(read_fd, EPOLLIN, in_callback));
    REQUIRE(callbacks.size() == 1);
    REQUIRE_NOTHROW(manager.modifyFd(read_fd, EPOLLIN, mod_callback));
    REQUIRE(callbacks.size() == 1);

    auto it = callbacks.find(read_fd);
    REQUIRE(it != callbacks.end());

    auto [fd, callback] = *it;
    REQUIRE(fd == read_fd);
    REQUIRE(callback != nullptr);

    callback(2);
    REQUIRE(n == 4);
  }

  SECTION("MOD UNKNOWN FD")
  {
    REQUIRE_NOTHROW(manager.addFd(read_fd, EPOLLIN, in_callback));
    REQUIRE(callbacks.size() == 1);

    REQUIRE_THROWS_AS(manager.modifyFd(213, EPOLLIN, mod_callback), std::runtime_error);
    REQUIRE(callbacks.size() == 1);
  }

  SECTION("MOD WITH NULLPTR CALLBACK")
  {
    REQUIRE_NOTHROW(manager.addFd(read_fd, EPOLLIN, in_callback));
    REQUIRE(callbacks.size() == 1);

    REQUIRE_NOTHROW(manager.modifyFd(read_fd, EPOLLIN, in_callback));
    REQUIRE(callbacks.size() == 1);

    REQUIRE_NOTHROW(manager.modifyFd(read_fd, EPOLLIN));
    REQUIRE(callbacks.size() == 1);

    auto it = callbacks.find(read_fd);
    REQUIRE(it != callbacks.end());

    // auto [fd, callback] = *it;
    // REQUIRE(fd == read_fd);
    // REQUIRE(callback != nullptr);

    // callback(1);
    // REQUIRE(n == 1);
  }

  SECTION("REMOVE FD")
  {
    REQUIRE_NOTHROW(manager.addFd(read_fd, EPOLLIN, in_callback));
    REQUIRE(callbacks.size() == 1);

    REQUIRE_NOTHROW(manager.removeFd(read_fd));
    REQUIRE(callbacks.size() == 0);
  }

  SECTION("REMOVE NON EXISTING FD")
  {
    REQUIRE_NOTHROW(manager.addFd(read_fd, EPOLLIN, in_callback));
    REQUIRE(callbacks.size() == 1);

    REQUIRE_THROWS_AS(manager.removeFd(218), std::runtime_error);
    REQUIRE(callbacks.size() == 1);
  }

  /* TEARDOWN */
  close(read_fd);
  close(write_fd);
  /* END TEARDOWN*/
}

TEST_CASE("Epoll event loop once", "[EpollManager]")
{
  const std::size_t max_buf_size = 128;
  char buf[max_buf_size];
  std::memset(buf, 0, max_buf_size);

  EpollManager manager;

  int fds[2];
  REQUIRE(pipe(fds) == 0);

  int read_fd = fds[0];
  int write_fd = fds[1];

  REQUIRE(makeNonBlocking(read_fd) != -1);
  REQUIRE(makeNonBlocking(write_fd) != -1);

  int status = 0;
  bool read_called = false;
  bool write_called = false;

  auto read_callback = [&](uint32_t events)
  {
    REQUIRE(events & EPOLLIN);
    status = read(read_fd, buf, max_buf_size);
    read_called = true;
  };

  auto write_callback = [&](uint32_t events)
  {
    REQUIRE(events & EPOLLOUT);
    status = write(write_fd, "Hello, World!", 14);
    write_called = true;
  };

  REQUIRE_NOTHROW(manager.addFd(read_fd, EPOLLIN, read_callback));
  REQUIRE_NOTHROW(manager.addFd(write_fd, EPOLLOUT, write_callback));

  SECTION("READ/WRITE USING PIPE")
  {
    // 1. Test EPOLLOUT (write callback fires -> write 14 bytes)
    eventLoopOnce(manager, 50);
    REQUIRE(write_called == true);
    REQUIRE(status == 14);

    // Remove write callback so it cannot fire again
    manager.removeFd(write_fd);
    write_called = false;

    // drain pipe so no leftover data remains
    while (read(read_fd, buf, max_buf_size) > 0)
    {
    }
    std::memset(buf, 0, max_buf_size);
    read_called = false;
    status = 0;

    // 2. Prepare read test: write data manually
    const char* msg = "hello";
    REQUIRE(write(write_fd, msg, 6) == 6);

    // Now only read_fd is registered and readable
    eventLoopOnce(manager, 50);

    REQUIRE(read_called == true);
    REQUIRE(status == 6);
    REQUIRE(std::strncmp(buf, "hello", 5) == 0);
  }

  close(read_fd);
  close(write_fd);
}

// TEST_CASE("EpollManager basic ADD/DEL", "[EpollManager]")
// {
//   EpollManager manager;

//   int fds[2];
//   auto callbacks = manager.getCallbacks();
//   auto in_callback = [&](uint32_t) { std::cout << "[CALLBACK] EPOLLIN triggered\n"; };
//   auto mod_callback = [&](uint32_t) { std::cout << "[CALLBACK] EPOLLOUT triggered\n"; };

//   REQUIRE(pipe(fds) == 0);
//   REQUIRE(callbacks.size() == 0);
//   int read_fd = fds[0];
//   int write_fd = fds[1];

//   REQUIRE(makeNonBlocking(read_fd) != -1);
//   REQUIRE(makeNonBlocking(write_fd) != -1);

//   SECTION("Adding the fd")
//   {
//     REQUIRE_NOTHROW(manager.addFd(read_fd, EPOLLIN, in_callback));
//     REQUIRE(callbacks.size() == 1);
//     auto curr = callbacks.find(read_fd);
//     REQUIRE(curr != callbacks.end());
//     REQUIRE(curr->first == read_fd);
//     // REQUIRE(*(curr->second.target) == in_callback);
//   }

//   // std::cout << "[TEST] Adding read_fd " << read_fd << " with EPOLLIN\n";

//   // std::cout << "[TEST] Trying to add same fd again (should throw)\n";
//   // try
//   // {
//   //   manager.addFd(read_fd, EPOLLIN, nullptr);
//   // }
//   // catch (const std::exception& e)
//   // {
//   //   std::cout << "[TEST] Caught exception: " << e.what() << "\n";
//   // }
//   // REQUIRE_THROWS(manager.addFd(read_fd, EPOLLIN, nullptr));

//   // std::cout << "[TEST] Removing read_fd " << read_fd << "\n";
//   // REQUIRE_NOTHROW(manager.removeFd(read_fd));

//   // std::cout << "[TEST] Removing same fd again (should throw)\n";
//   // try
//   // {
//   //   manager.removeFd(read_fd);
//   // }
//   // catch (const std::exception& e)
//   // {
//   //   std::cout << "[TEST] Caught exception: " << e.what() << "\n";
//   // }
//   // REQUIRE_THROWS(manager.removeFd(read_fd));

//   // How to ensure these will be closed (TEARDOWN)
//   close(read_fd);
//   close(write_fd);
// }

// TEST_CASE("EpollManager callback execution", "[verbose]")
// {
//   EpollManager manager;

//   int fds[2];
//   REQUIRE(pipe(fds) == 0);
//   int read_fd = fds[0];
//   int write_fd = fds[1];

//   makeNonBlocking(read_fd);
//   makeNonBlocking(write_fd);

//   std::atomic<int> callback_count{0};

//   std::cout << "[TEST] Adding read_fd " << read_fd << " with EPOLLIN callback\n";
//   REQUIRE_NOTHROW(manager.addFd(read_fd, EPOLLIN,
//                                 [&](uint32_t)
//                                 {
//                                   char buf[128];
//                                   ssize_t n = read(read_fd, buf, sizeof(buf));  // read one message at a time
//                                   if (n > 0)
//                                   {
//                                     std::cout << "[CALLBACK] EPOLLIN triggered, read " << n << " bytes: '"
//                                               << std::string(buf, n) << "'\n";
//                                     callback_count.fetch_add(1);
//                                   }
//                                 }));

//   const char* msgs[] = {"HELLO1", "HELLO2", "HELLO3"};
//   for (const auto& msg : msgs)
//   {
//     std::cout << "[TEST] Writing '" << msg << "' to pipe\n";
//     write(write_fd, msg, strlen(msg));

//     int tries = 0;
//     int prev_count = callback_count.load();
//     while (callback_count.load() == prev_count && tries++ < 200)
//       eventLoopOnce(manager, 10);

//     if (callback_count.load() == prev_count)
//       std::cout << "[TEST] Warning: callback did not fire within timeout\n";
//   }

//   REQUIRE(callback_count.load() == 3);

//   close(read_fd);
//   close(write_fd);
// }

// TEST_CASE("EpollManager modifyFd", "[verbose]")
// {
//   EpollManager manager;

//   int fds[2];
//   REQUIRE(pipe(fds) == 0);
//   int read_fd = fds[0];
//   int write_fd = fds[1];

//   makeNonBlocking(read_fd);
//   makeNonBlocking(write_fd);

//   std::atomic<int> callback_count{0};

//   std::cout << "[TEST] Adding read_fd " << read_fd << " with EPOLLIN\n";
//   REQUIRE_NOTHROW(manager.addFd(read_fd, EPOLLIN,
//                                 [&](uint32_t)
//                                 {
//                                   char buf[128];
//                                   ssize_t n = read(read_fd, buf, sizeof(buf));  // read one message
//                                   if (n > 0)
//                                   {
//                                     std::cout << "[CALLBACK] Original EPOLLIN triggered, read " << n << " bytes: '"
//                                               << std::string(buf, n) << "'\n";
//                                     callback_count.fetch_add(1);
//                                   }
//                                 }));

//   std::cout << "[TEST] Modifying read_fd callback\n";
//   REQUIRE_NOTHROW(manager.modifyFd(read_fd, EPOLLIN,
//                                    [&](uint32_t)
//                                    {
//                                      char buf[128];
//                                      ssize_t n = read(read_fd, buf, sizeof(buf));
//                                      if (n > 0)
//                                      {
//                                        std::cout << "[CALLBACK] Modified EPOLLIN triggered, read " << n << " bytes:
//                                        '"
//                                                  << std::string(buf, n) << "'\n";
//                                        callback_count.fetch_add(1);
//                                      }
//                                    }));

//   const char* msgs[] = {"HELLO1", "HELLO2"};
//   for (const auto& msg : msgs)
//   {
//     std::cout << "[TEST] Writing '" << msg << "' to write_fd to trigger modified callback\n";
//     write(write_fd, msg, strlen(msg));

//     int tries = 0;
//     int prev_count = callback_count.load();
//     while (callback_count.load() == prev_count && tries++ < 200)
//       eventLoopOnce(manager, 10);

//     if (callback_count.load() == prev_count)
//       std::cout << "[TEST] Warning: modified callback did not fire within timeout\n";
//   }

//   REQUIRE(callback_count.load() == 2);

//   close(read_fd);
//   close(write_fd);
// }
