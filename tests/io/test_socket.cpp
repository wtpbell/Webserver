
#include <sys/socket.h>
#include <unistd.h>

#include <type_traits>

#include "utils/Logger.hpp"
#include "catch_amalgamated.hpp"
#include "exception/FileDescriptorException.hpp"
#include "io/Socket.hpp"

static void RequireAllFdsClosed(void)
{
  auto& shared_count{SharedFD::GetSharedCountVector()};

  for (std::size_t fd{}; fd < shared_count.size(); ++fd)
  {
    CAPTURE(fd, shared_count[fd]);
    REQUIRE(shared_count[fd] == 0);
  }
}

TEST_CASE("rule-of-five:type-traits", "[socket]")
{
  REQUIRE(std::is_copy_constructible<Socket>::value);
  REQUIRE(std::is_copy_assignable<Socket>::value);
  REQUIRE(std::is_move_constructible<Socket>::value);
  REQUIRE(std::is_move_assignable<Socket>::value);
  REQUIRE(std::is_destructible<Socket>::value);
}

TEST_CASE("rule-of-five:runtime-move", "[socket]")
{
  int original_fd = -1;

  {
    Socket s1 = Socket::CreateSocket(AF_INET, SOCK_STREAM, 0);
    original_fd = s1.GetFD();
    REQUIRE(original_fd >= 0);

    Socket s2(std::move(s1));
    REQUIRE(s2.GetFD() == original_fd);
    REQUIRE(s2.SharedCount() == 1);
    REQUIRE(s1.GetFD() == -1);
    REQUIRE(s1.SharedCount() == 0);
    // s2 will close the fd on destruction; do not close it here to avoid double-close
  }

  // After scope exit, s2 destroyed; check that original fd is closed
  int rc = fcntl(original_fd, F_GETFD);
  CAPTURE(original_fd, rc, errno);
  REQUIRE(rc == -1);
  REQUIRE(errno == EBADF);

  errno = 0;
  RequireAllFdsClosed();
}

TEST_CASE("rule-of-five:runtime-copy", "[socket]")
{
  int original_fd = -1;
  {
    Socket s1 = Socket::CreateSocket(AF_INET, SOCK_STREAM, 0);
    original_fd = s1.GetFD();
    REQUIRE(original_fd >= 0);

    int before_count = s1.SharedCount();
    Socket s2 = s1;  // copy ctor
    REQUIRE(s2.GetFD() == original_fd);
    REQUIRE(s1.GetFD() == original_fd);
    REQUIRE(s1.SharedCount() == before_count + 1);
    REQUIRE(s2.SharedCount() == s1.SharedCount());
    // both s1 and s2 will be destroyed at scope exit
  }

  // After scope exit, copied sockets destroyed; original fd should be closed
  int rc = fcntl(original_fd, F_GETFD);
  CAPTURE(original_fd, rc, errno);
  REQUIRE(rc == -1);
  REQUIRE(errno == EBADF);

  errno = 0;
  RequireAllFdsClosed();
}

TEST_CASE("CreateSocket(int, int, int)", "[socket]")
{
  SECTION("invalid-family")
  {
    REQUIRE_THROWS_AS(Socket::CreateSocket(-1, SOCK_STREAM, 0), FileDescriptorException);
  }

  SECTION("valid-ipv4-stream")
  {
    Socket s = Socket::CreateSocket(AF_INET, SOCK_STREAM, 0);
    REQUIRE(s.GetFD() >= 0);
    REQUIRE(s.SharedCount() == 1);
  }

  SECTION("valid-ipv4-datagram")
  {
    Socket s = Socket::CreateSocket(AF_INET, SOCK_DGRAM, 0);
    REQUIRE(s.GetFD() >= 0);
    REQUIRE(s.SharedCount() == 1);
  }

  SECTION("valid-ipv6-stream")
  {
    Socket s = Socket::CreateSocket(AF_INET6, SOCK_STREAM, 0);
    REQUIRE(s.GetFD() >= 0);
    REQUIRE(s.SharedCount() == 1);
  }

  SECTION("valid-ipv6-datagram")
  {
    Socket s = Socket::CreateSocket(AF_INET6, SOCK_DGRAM, 0);
    REQUIRE(s.GetFD() >= 0);
    REQUIRE(s.SharedCount() == 1);
  }

  SECTION("valid-unix-stream")
  {
    Socket s = Socket::CreateSocket(AF_UNIX, SOCK_STREAM, 0);
    REQUIRE(s.GetFD() >= 0);
    REQUIRE(s.SharedCount() == 1);
  }

  SECTION("valid-unix-datagram")
  {
    Socket s = Socket::CreateSocket(AF_UNIX, SOCK_DGRAM, 0);
    REQUIRE(s.GetFD() >= 0);
    REQUIRE(s.SharedCount() == 1);
  }

  errno = 0;
  RequireAllFdsClosed();
}

TEST_CASE("CreateSocket(const char* host, const char* service, int flags)", "[socket]")
{
  SECTION("valid-hostname")
  {
    Socket s = Socket::CreateSocket("::1", "8080");
    REQUIRE(s.GetFD() >= 0);
    REQUIRE(s.SharedCount() == 1);
  }

  SECTION("invalid-hostname")
  {
    REQUIRE_THROWS_AS(Socket::CreateSocket("localhost.invalid", "8080"), FileDescriptorException);
  }

  SECTION("valid-port-number")
  {
    Socket s = Socket::CreateSocket(nullptr, "8080");
    REQUIRE(s.GetFD() >= 0);
    REQUIRE(s.SharedCount() == 1);
  }

  SECTION("valid-service-name")
  {
    Socket s = Socket::CreateSocket(nullptr, "http");
    REQUIRE(s.GetFD() >= 0);
    REQUIRE(s.SharedCount() == 1);
  }

  SECTION("port-zero")
  {
    Socket s = Socket::CreateSocket(nullptr, "0");
    REQUIRE(s.GetFD() >= 0);
    REQUIRE(s.SharedCount() == 1);
  }

  SECTION("invalid-port-number")
  {
    REQUIRE_THROWS_AS(Socket::CreateSocket(nullptr, "999999"), FileDescriptorException);
  }

  SECTION("invalid-service-name")
  {
    REQUIRE_THROWS_AS(Socket::CreateSocket(nullptr, "nonexistent_service_xyz"), FileDescriptorException);
  }

  SECTION("nullptr-service-name")
  {
    REQUIRE_THROWS_AS(Socket::CreateSocket(nullptr, nullptr), FileDescriptorException);
  }

  SECTION("invalid-flag")
  {
    REQUIRE_THROWS_AS(Socket::CreateSocket(nullptr, "8080", -1), FileDescriptorException);
  }

  errno = 0;
  RequireAllFdsClosed();
}

TEST_CASE("SocketPair(int, int, int)", "[socket]")
{
  SECTION("invalid-family")
  {
    REQUIRE_THROWS_AS(Socket::SocketPair(-1, SOCK_STREAM, 0), FileDescriptorException);
  }

  SECTION("valid-unix-stream")
  {
    auto [s1, s2] = Socket::SocketPair(AF_UNIX, SOCK_STREAM, 0);
    REQUIRE(s1.GetFD() >= 0);
    REQUIRE(s2.GetFD() >= 0);
    REQUIRE(s1.GetFD() != s2.GetFD());
  }

  SECTION("valid-unix-datagram")
  {
    auto [s1, s2] = Socket::SocketPair(AF_UNIX, SOCK_DGRAM, 0);
    REQUIRE(s1.GetFD() >= 0);
    REQUIRE(s2.GetFD() >= 0);
    REQUIRE(s1.GetFD() != s2.GetFD());
  }

  SECTION("data-communication-unix-stream")
  {
    auto [s1, s2] = Socket::SocketPair(AF_UNIX, SOCK_STREAM, 0);

    // Send data from s1 to s2
    std::string sent_msg = "Hello from socket 1";
    std::size_t leftover = sent_msg.size();
    REQUIRE(leftover == sent_msg.size());

    ssize_t bytes_sent = s1.Send(sent_msg, leftover);
    REQUIRE(bytes_sent > 0);
    REQUIRE(leftover == 0);

    // Receive data on s2
    std::string received_msg;
    ssize_t bytes_recv = s2.Recv(received_msg, 0, bytes_sent);
    REQUIRE(bytes_recv == bytes_sent);
    REQUIRE(received_msg == sent_msg);
  }

  SECTION("data-communication-closed-receiver")
  {
    auto [s1, s2] = Socket::SocketPair(AF_UNIX, SOCK_STREAM, 0);

    // Close s2 before sending
    s2.Reset();

    // Try to send data from s1 to closed s2
    std::string sent_msg = "Hello from socket 1";
    std::size_t leftover = sent_msg.size();

    // Sending to a closed socket should return 0
    ssize_t bytes_sent = s1.Send(sent_msg, leftover, MSG_NOSIGNAL);
    REQUIRE(bytes_sent == -1);
  }

  SECTION("partial-send-non-blocking")
  {
    auto [s1, s2] = Socket::SocketPair(AF_UNIX, SOCK_STREAM, 0);

    int buffer_size = 2048;
    socklen_t optlen = sizeof(buffer_size);

    REQUIRE_NOTHROW(s1.SetSockOpt(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<void*>(&buffer_size), optlen));
    if (getsockopt(s1.GetFD(), SOL_SOCKET, SO_SNDBUF, reinterpret_cast<void*>(&buffer_size), &optlen) == -1)
      FAIL("Test was unable to set the send buffer size to 2048");

    // Fill the receive buffer by sending without reading
    std::string large_msg(buffer_size + 64, 'X');
    std::size_t leftover = large_msg.size();

    // Keep sending until buffer is full and we get EWOULDBLOCK
    ssize_t send_result = 0;
    while (send_result != -1 && leftover > 0)
      send_result = s1.Send(large_msg, leftover, MSG_DONTWAIT);

    // When buffer is full, Send should return -1 and set errno to EWOULDBLOCK
    REQUIRE(send_result == -1);
    REQUIRE(errno == EWOULDBLOCK);

    // Try sending another message on already-full buffer
    errno = 0;
    std::string another_msg = "More data";
    std::size_t another_leftover = another_msg.size();
    ssize_t another_result = s1.Send(another_msg, another_leftover, MSG_DONTWAIT);
    REQUIRE(another_result == -1);
    REQUIRE(errno == EWOULDBLOCK);
  }

  SECTION("max-chunk-size-respected-small")
  {
    auto [s1, s2] = Socket::SocketPair(AF_UNIX, SOCK_STREAM, 0);

    // Send a message with a small max_chunk_size
    std::string sent_msg = "Hello from socket 1";
    std::size_t leftover = sent_msg.size();

    // Limit max_chunk_size to 5 bytes
    ssize_t bytes_sent = s1.Send(sent_msg, leftover, 0, 5);
    REQUIRE(bytes_sent == 5);
    REQUIRE(leftover == sent_msg.size() - 5);
  }

  SECTION("max-chunk-size-respected-large")
  {
    auto [s1, s2] = Socket::SocketPair(AF_UNIX, SOCK_STREAM, 0);

    // Send a message with a large max_chunk_size
    std::string sent_msg = "Hello";
    std::size_t leftover = sent_msg.size();

    // Set max_chunk_size larger than message
    ssize_t bytes_sent = s1.Send(sent_msg, leftover, 0, 1024);
    REQUIRE(bytes_sent == static_cast<ssize_t>(sent_msg.size()));
    REQUIRE(leftover == 0);
  }

  SECTION("recv-max-chunk-size-respected-small")
  {
    auto [s1, s2] = Socket::SocketPair(AF_UNIX, SOCK_STREAM, 0);

    // Send a message
    std::string sent_msg = "Hello from socket 1";
    std::size_t leftover = sent_msg.size();
    s1.Send(sent_msg, leftover);

    // Receive with small max_chunk_size
    std::string received_msg;
    ssize_t bytes_recv = s2.Recv(received_msg, 0, 5);
    REQUIRE(bytes_recv == 5);
    REQUIRE(received_msg.size() == 5);
    REQUIRE(received_msg == "Hello");
  }

  SECTION("recv-max-chunk-size-respected-large")
  {
    auto [s1, s2] = Socket::SocketPair(AF_UNIX, SOCK_STREAM, 0);

    // Send a message
    std::string sent_msg = "Hello";
    std::size_t leftover = sent_msg.size();
    s1.Send(sent_msg, leftover);

    // Receive with large max_chunk_size
    std::string received_msg;
    ssize_t bytes_recv = s2.Recv(received_msg, MSG_DONTWAIT, 1024);
    REQUIRE(bytes_recv == 5);
    REQUIRE(received_msg == sent_msg);
  }

  errno = 0;
  RequireAllFdsClosed();
}

TEST_CASE("GetAddrInfo(addrinfo&, const char*, const char*)", "[socket]")
{
  SECTION("valid-hostname-and-service")
  {
    Socket::addrinfo_t hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    Socket::addrinfo_t* result = Socket::GetAddrInfo(hints, "localhost", "http");
    REQUIRE(result != nullptr);
    REQUIRE(result->ai_addr != nullptr);
    freeaddrinfo(result);
  }

  SECTION("valid-ipv4-loopback")
  {
    Socket::addrinfo_t hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    Socket::addrinfo_t* result = Socket::GetAddrInfo(hints, "127.0.0.1", "8080");
    REQUIRE(result != nullptr);
    REQUIRE(result->ai_family == AF_INET);
    REQUIRE(result->ai_addr != nullptr);
    freeaddrinfo(result);
  }

  SECTION("valid-ipv6-loopback")
  {
    Socket::addrinfo_t hints{};
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;

    Socket::addrinfo_t* result = Socket::GetAddrInfo(hints, "::1", "8080");
    REQUIRE(result != nullptr);
    REQUIRE(result->ai_family == AF_INET6);
    REQUIRE(result->ai_addr != nullptr);
    freeaddrinfo(result);
  }

  SECTION("invalid-hostname")
  {
    Socket::addrinfo_t hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // The `.invalid` suffix is important here (see https://www.rfc-editor.org/rfc/rfc2606.html)
    REQUIRE_THROWS_AS(Socket::GetAddrInfo(hints, "nonexistent.hostname.invalid", "http"), FileDescriptorException);
  }

  SECTION("af-inet-filter")
  {
    Socket::addrinfo_t hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    Socket::addrinfo_t* result = Socket::GetAddrInfo(hints, "localhost", "http");
    REQUIRE(result != nullptr);
    REQUIRE(result->ai_family == AF_INET);
    freeaddrinfo(result);
  }

  SECTION("af-inet6-filter")
  {
    Socket::addrinfo_t hints{};
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;

    Socket::addrinfo_t* result = Socket::GetAddrInfo(hints, "::1", "http");
    REQUIRE(result != nullptr);
    REQUIRE(result->ai_family == AF_INET6);
    freeaddrinfo(result);
  }

  SECTION("sock-stream-filter")
  {
    Socket::addrinfo_t hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    Socket::addrinfo_t* result = Socket::GetAddrInfo(hints, "localhost", "http");
    REQUIRE(result != nullptr);
    REQUIRE(result->ai_socktype == SOCK_STREAM);
    freeaddrinfo(result);
  }

  SECTION("sock-dgram-filter")
  {
    Socket::addrinfo_t hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    Socket::addrinfo_t* result = Socket::GetAddrInfo(hints, "localhost", "domain");
    REQUIRE(result != nullptr);
    REQUIRE(result->ai_socktype == SOCK_DGRAM);
    freeaddrinfo(result);
  }

  errno = 0;
}

TEST_CASE("Bind, Listen, Accept4", "[socket]")
{
  SECTION("bind-socket-already-bound")
  {
    Socket socket{Socket::CreateSocket(nullptr, "8080")};

    REQUIRE_NOTHROW(socket.Bind());
    REQUIRE_THROWS_AS(socket.Bind(), FileDescriptorException);
    REQUIRE(errno == EINVAL);  // Socket is already bound
  }

  SECTION("bind-address-already-in-use")
  {
    Socket socket_one{Socket::CreateSocket(nullptr, "8080")};
    Socket socket_two{Socket::CreateSocket(nullptr, "8080")};

    REQUIRE_NOTHROW(socket_one.Bind());
    REQUIRE_THROWS_AS(socket_two.Bind(), FileDescriptorException);
    REQUIRE(errno == EADDRINUSE);  // Address already in use
  }

  SECTION("bind-bad-file-descriptor")
  {
    Socket socket;

    REQUIRE_THROWS_AS(socket.Bind(), FileDescriptorException);
    REQUIRE(errno == EBADF);  // Bad file descriptor
  }

  SECTION("listen-on-unbound-socket")
  {
    Socket socket{Socket::CreateSocket(nullptr, "80")};

    // This causes an implicit bind to a suitable local address and ephemeral port (49152-65535)
    REQUIRE_NOTHROW(socket.Listen());
  }

  SECTION("listen-unitialized-socket")
  {
    Socket socket;

    REQUIRE_THROWS_AS(socket.Listen(), FileDescriptorException);
    REQUIRE(errno == EBADF);  // Bad file descriptor
  }

  SECTION("listen-change-backlog")
  {
    Socket socket{Socket::CreateSocket(nullptr, "8080")};

    REQUIRE_NOTHROW(socket.Bind());
    REQUIRE_NOTHROW(socket.Listen(5));
    REQUIRE_NOTHROW(socket.Listen(15));
  }

  SECTION("bind-after-listen-on-socket")
  {
    Socket socket{Socket::CreateSocket(nullptr, "8080")};

    REQUIRE_NOTHROW(socket.Listen());
    REQUIRE_THROWS_AS(socket.Bind(), FileDescriptorException);
    REQUIRE(errno == EINVAL);  // Socket is already bound
  }

  SECTION("connect-after-listen-on-socket")
  {
    Socket socket{Socket::CreateSocket(nullptr, "8080")};

    REQUIRE_NOTHROW(socket.Listen());
    REQUIRE_THROWS_AS(socket.Connect(), FileDescriptorException);
    REQUIRE(errno == EISCONN);  // Transport endpoint is already connected
  }

  SECTION("accept-basic-test")
  {
    Socket socket{Socket::CreateSocket(nullptr, "8080")};
    socket.SetNonBlocking(true);

    REQUIRE_THROWS_AS(((void)socket.Accept4()), FileDescriptorException);
    REQUIRE(errno == EINVAL);  // socket is not listing

    errno = 0;
    REQUIRE_NOTHROW(socket.Bind());
    REQUIRE_THROWS_AS(((void)socket.Accept4()), FileDescriptorException);
    REQUIRE(errno == EINVAL);  // socket is not listing

    errno = 0;
    REQUIRE_NOTHROW(socket.Listen());
    REQUIRE_NOTHROW(((void)socket.Accept4()));
    REQUIRE(errno == EWOULDBLOCK);  // nothing in queue
  }

  SECTION("accept-a-connection")
  {
    Socket server{Socket::CreateSocket(nullptr, "8080")};
    Socket client{Socket::CreateSocket("::1", "8080")};

    server.Bind();
    server.Listen();

    // Client opens connection
    client.Connect();
    Socket incoming = server.Accept4();
    REQUIRE(incoming.GetFD() >= 0);
    REQUIRE(server.SharedCount() == 1);
    REQUIRE(client.SharedCount() == 1);
    REQUIRE(incoming.SharedCount() == 1);

    // Send message to client
    std::string server_message{"Hello, World!"};
    std::size_t leftover{server_message.length()};

    ssize_t bytes = incoming.Send(server_message, leftover);
    REQUIRE(leftover == 0);
    REQUIRE(bytes == static_cast<ssize_t>(server_message.length()));

    std::string client_message;
    bytes = client.Recv(client_message, 0, server_message.length());
    REQUIRE(bytes == static_cast<ssize_t>(server_message.length()));
    REQUIRE(client_message == server_message);

    // Client closes connection
    client.Reset();

    REQUIRE(client.SharedCount() == 0);
    REQUIRE(incoming.SharedCount() == 1);

    server_message.clear();
    bytes = incoming.Recv(server_message);
    REQUIRE(bytes == 0);
  }

  errno = 0;
  RequireAllFdsClosed();
}

TEST_CASE("Toggle non-blocking through SetNonBlocking", "[socket]")
{
  Socket socket{Socket::CreateSocket(AF_UNIX, SOCK_STREAM, 0)};

  int flags = fcntl(socket.GetFD(), F_GETFL);
  if (flags == -1)
    FAIL("Failed to retrieve socket flags");
  REQUIRE((flags & O_NONBLOCK) == 0);

  int old_flags = flags;
  socket.SetNonBlocking(true);
  flags = fcntl(socket.GetFD(), F_GETFL);
  if (flags == -1)
    FAIL("Failed to retrieve socket flags");
  REQUIRE((flags & O_NONBLOCK) == O_NONBLOCK);

  socket.SetNonBlocking(false);
  flags = fcntl(socket.GetFD(), F_GETFL);
  if (flags == -1)
    FAIL("Failed to retrieve socket flags");
  REQUIRE((flags & O_NONBLOCK) == 0);
  REQUIRE(flags == old_flags);
}
