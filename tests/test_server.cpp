#include <sys/epoll.h>
#include <unistd.h>

#include <memory>
#include <thread>

#include "Client.hpp"
#include "EpollManager.hpp"
#include "Logger.hpp"
#include "Server.hpp"
#include "catch_amalgamated.hpp"
#include "webserv.hpp"

TEST_CASE("Basic server connection", "[server]")
{
  Logger::SetLogFilter(LogLevel::NONE);
  auto endcallback = [](EpollManager&, const struct epoll_event&)
  {
    g_shutdown = true;
  };

  int pipefd[2];
  REQUIRE((pipe2(pipefd, O_NONBLOCK) == 0));
  SECTION("Basic Server connection")
  {
    Client client;
    std::unique_ptr<EpollManager> manager;
    std::unique_ptr<Server> server;

    REQUIRE_NOTHROW((manager = std::make_unique<EpollManager>()));
    REQUIRE_NOTHROW((server = std::make_unique<Server>("8080")));
    REQUIRE_NOTHROW(server->ListenAndRegister(*manager, 5));
    REQUIRE_NOTHROW(client.Connect("::1", "8080"));
    REQUIRE_NOTHROW(manager->AddFd(pipefd[0], EPOLLIN, endcallback));

    g_shutdown = false;
    close(pipefd[1]);
    pipefd[1] = -1;

    client.Ping("Hello, world!");
    std::thread loop_thread{[&manager]
                            {
                              manager->EventLoop();
                            }};
    loop_thread.join();
  }

  Logger::SetLogFilter(LogLevel::ALL);
  if (pipefd[1] != -1)
    close(pipefd[1]);
  close(pipefd[0]);
}
