// Build:
//   1. make
//   c  ++ -std=c++17 -Wall -Wextra -Werror tests/test_integration_main.cpp -o integration_test
//
// Run:
//   ./integration_test [port]

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

// --------------------------- Request Builders ---------------------------

static std::string Get(const std::string& path)
{
  std::ostringstream oss;
  oss << "GET " << path << " HTTP/1.1\r\n"
      << "Host: localhost\r\n"
      << "Connection: close\r\n"
      << "\r\n";
  return oss.str();
}

static std::string DeleteReq(const std::string& path)
{
  std::ostringstream oss;
  oss << "DELETE " << path << " HTTP/1.1\r\n"
      << "Host: localhost\r\n"
      << "Connection: close\r\n"
      << "\r\n";
  return oss.str();
}

static std::string UnknownMethodReq(const std::string& path)
{
  std::ostringstream oss;
  oss << "UNKNOWN " << path << " HTTP/1.1\r\n"
      << "Host: localhost\r\n"
      << "Connection: close\r\n"
      << "\r\n";
  return oss.str();
}

static std::string PutReq(const std::string& path)
{
  std::ostringstream oss;
  oss << "PUT " << path << " HTTP/1.1\r\n"
      << "Host: localhost\r\n"
      << "Connection: close\r\n"
      << "\r\n";
  return oss.str();
}

static std::string PostUploadRequest(const std::string& path, const std::string& body,
                                     const std::string& contentType = "text/plain")
{
  std::ostringstream oss;
  oss << "POST " << path << " HTTP/1.1\r\n"
      << "Host: localhost\r\n"
      << "Content-Type: " << contentType << "\r\n"
      << "Content-Length: " << body.size() << "\r\n"
      << "Connection: close\r\n"
      << "\r\n"
      << body;
  return oss.str();
}

static std::string PostHeaders(const std::string& path, size_t len, const std::string& contentType = "text/plain")
{
  std::ostringstream oss;
  oss << "POST " << path << " HTTP/1.1\r\n"
      << "Host: localhost\r\n"
      << "Content-Type: " << contentType << "\r\n"
      << "Content-Length: " << len << "\r\n"
      << "Connection: close\r\n"
      << "\r\n";
  return oss.str();
}

// --------------------------- Test Utilities ---------------------------

struct TestFailure : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

static void fail(const std::string& msg, const std::string& response = "")
{
  std::ostringstream oss;
  oss << "FAIL: " << msg;
  if (!response.empty())
    oss << "\n---- response ----\n" << response << "\n------------------\n";
  throw TestFailure(oss.str());
}

static int parse_status_code(const std::string& response)
{
  std::istringstream iss(response);
  std::string httpver;
  int code = 0;
  iss >> httpver >> code;
  return code;
}

static bool response_contains(const std::string& response, const std::string& needle)
{
  return response.find(needle) != std::string::npos;
}

static void expect_one_of(const std::string& response, const char* name, std::initializer_list<int> allowed)
{
  int code = parse_status_code(response);
  for (int x : allowed)
    if (code == x)
      return;

  std::ostringstream oss;
  oss << name << ": unexpected status " << code;
  fail(oss.str(), response);
}

// --------------------------- Filesystem Fixtures ---------------------------

static void rm_rf(const fs::path& p)
{
  std::error_code ec;
  fs::remove_all(p, ec);
}

static void write_file(const fs::path& p, const std::string& s)
{
  fs::create_directories(p.parent_path());
  std::ofstream out(p, std::ios::binary | std::ios::trunc);
  if (!out)
    fail("cannot write fixture file: " + p.string());
  out.write(s.data(), static_cast<std::streamsize>(s.size()));
  if (!out)
    fail("cannot write fixture file: " + p.string());
}

static std::string read_file(const fs::path& p)
{
  std::ifstream in(p, std::ios::binary);
  if (!in)
    fail("cannot read file: " + p.string());
  std::string s((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  return s;
}

static void expect_file_equals(const fs::path& p, const std::string& want)
{
  if (!fs::exists(p))
    fail("expected file does not exist: " + p.string());

  std::string got = read_file(p);
  if (got != want)
    fail("file content mismatch: " + p.string());
}

static void ensure_clean_www()
{
  rm_rf("./www");
  fs::create_directories("./www");
}

// --------------------------- Process Control ---------------------------

static pid_t spawn_server(const std::string& exe, const std::vector<std::string>& args)
{
  pid_t pid = ::fork();
  if (pid < 0)
    fail("fork failed");

  if (pid == 0)
  {
    ::setpgid(0, 0);
    std::vector<char*> av;
    av.reserve(args.size() + 2);
    av.push_back(const_cast<char*>(exe.c_str()));
    for (const std::string& arg : args)
      av.push_back(const_cast<char*>(arg.c_str()));
    av.push_back(nullptr);

    ::execv(exe.c_str(), av.data());
    std::perror("execv");
    std::_Exit(127);
  }
  return pid;
}

static void stop_server(pid_t pid)
{
  if (pid <= 0)
    return;
  ::kill(-pid, SIGTERM);  // kill the whole group
  int status = 0;
  ::waitpid(pid, &status, 0);
}

struct ServerGuard
{
    pid_t pid;
    ~ServerGuard()
    {
      stop_server(pid);
    }
};

// --------------------------- TCP Client ---------------------------

static int connect_with_retry(const std::string& host, int port, int timeout_ms)
{
  auto start = std::chrono::steady_clock::now();

  while (true)
  {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
      fail("socket failed");

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    if (::inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1)
      fail("inet_pton failed");

    if (::connect(fd, (sockaddr*)&addr, sizeof(addr)) == 0)
      return fd;

    ::close(fd);

    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
    if (elapsed > timeout_ms)
      fail("server did not become ready in time");
  }
}

static std::string send_and_recv_close(const std::string& host, int port, const std::string& request)
{
  int fd = connect_with_retry(host, port, 2500);

  // SEND ALL
  size_t total = 0;
  while (total < request.size())
  {
    ssize_t sent = ::send(fd, request.data() + total, request.size() - total, 0);
    if (sent > 0)
    {
      total += static_cast<size_t>(sent);
      continue;
    }
    if (total > 0)
      break;  // stop sending and try to recv whatever response we got
    fail("send failed before any bytes were sent");
  }

  ::shutdown(fd, SHUT_WR);

  // RECV UNTIL CLOSE
  std::string response;
  char buf[4096];

  while (true)
  {
    ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
    if (n > 0)
    {
      response.append(buf, (size_t)n);
      continue;
    }
    if (n == 0)
      break;
    if (!response.empty())
      break;  // error after partial response → accept partial
    fail("recv failed before any response");
  }

  ::close(fd);
  return response;
}

static std::string post_big_streaming(const std::string& host, int port, const std::string& path, size_t totalBytes)
{
  int fd = connect_with_retry(host, port, 2500);

  std::string headers = PostHeaders(path, totalBytes);
  // send headers (must succeed)
  size_t off = 0;
  while (off < headers.size())
  {
    ssize_t n = ::send(fd, headers.data() + off, headers.size() - off, 0);
    if (n <= 0)
      fail("send headers failed");
    off += (size_t)n;
  }

  // stream body
  std::string chunk(8192, 'a');
  size_t sentBody = 0;
  while (sentBody < totalBytes)
  {
    size_t toSend = std::min(chunk.size(), totalBytes - sentBody);
    ssize_t n = ::send(fd, chunk.data(), toSend, 0);
    if (n <= 0)
      break;
    sentBody += (size_t)n;
  }

  // recv response
  std::string response;
  char buf[4096];
  while (true)
  {
    ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
    if (n > 0)
    {
      response.append(buf, (size_t)n);
      continue;
    }
    if (n == 0)
      break;
    if (!response.empty())
      break;
    fail("recv failed before any response");
  }

  ::close(fd);
  return response;
}
// --------------------------- Main Tests ---------------------------

int main(int argc, char** argv)
{
  const std::string host = "127.0.0.1";
  int port = 8080;
  if (argc > 1)
    port = std::stoi(argv[1]);

  ensure_clean_www();

  pid_t pid = spawn_server("./webserv", {std::to_string(port)});
  ServerGuard guard{pid};

  {
    int fd = connect_with_retry(host, port, 2500);
    ::close(fd);
  }

  // -------------------- GET tests --------------------

  // GET missing -> 404
  {
    std::string response = send_and_recv_close(host, port, Get("/nofile"));
    expect_one_of(response, "GET missing", {404});
  }

  // GET file -> 200 + body contains "hello"
  write_file("./www/file.txt", "hello");
  {
    std::string response = send_and_recv_close(host, port, Get("/file.txt"));
    expect_one_of(response, "GET file", {200});
    if (!response_contains(response, "hello"))
      fail("GET file body mismatch", response);
  }

  // Directory redirect (no trailing slash) -> 301
  fs::create_directories("./www/dir");
  {
    std::string response = send_and_recv_close(host, port, Get("/dir"));
    expect_one_of(response, "GET dir redirect", {301});
  }

  // Directory with missing index and autoindex OFF -> 403
  {
    std::string response = send_and_recv_close(host, port, Get("/dir/"));
    expect_one_of(response, "GET dir no index (autoindex off)", {403});
  }

  // Directory with index -> 200 and contains "IDX"
  write_file("./www/dir/index.html", "<h1>IDX</h1>");
  {
    std::string response = send_and_recv_close(host, port, Get("/dir/"));
    expect_one_of(response, "GET dir index", {200});
    if (!response_contains(response, "IDX"))
      fail("GET dir index body mismatch", response);
  }

  // -------------------- Method tests --------------------

  // UNKNOWN method -> 501
  {
    std::string response = send_and_recv_close(host, port, UnknownMethodReq("/"));
    expect_one_of(response, "UNKNOWN method", {501});
  }

  // PUT -> 501 (your MethodToMask returns None for PUT)
  {
    std::string response = send_and_recv_close(host, port, PutReq("/"));
    expect_one_of(response, "PUT method", {501});
  }

  // -------------------- POST tests --------------------

  // POST / -> 400
  {
    std::string response = send_and_recv_close(host, port, PostUploadRequest("/", "x"));
    expect_one_of(response, "POST /", {400});
  }

  // POST trailing slash -> 400 (checked in HandleMethods)
  {
    std::string response = send_and_recv_close(host, port, PostUploadRequest("/dir/", "x"));
    expect_one_of(response, "POST trailing slash", {400});
  }

  // POST create file -> 201
  {
    fs::create_directories("./www/.upload_tmp");
    std::error_code ec;
    fs::remove("./www/.upload_tmp/test.txt", ec);
    const std::string body = "hello webserv\nline2\n";
    std::string response = send_and_recv_close(host, port, PostUploadRequest("/.upload_tmp/test.txt", body));
    expect_one_of(response, "POST create file", {201});
    expect_file_equals("./www/.upload_tmp/test.txt", body);
  }

  //   POST overwrite file -> 204
  {
    const std::string body = "NEW";
    std::string response = send_and_recv_close(host, port, PostUploadRequest("/.upload_tmp/test.txt", body));
    expect_one_of(response, "POST overwrite file", {204});
    expect_file_equals("./www/.upload_tmp/test.txt", body);
  }

  // POST parent missing -> 404 (ValidatePostTarget)
  {
    std::string response = send_and_recv_close(host, port, PostUploadRequest("/no_such_dir/a.txt", "x"));
    expect_one_of(response, "POST parent missing", {404});
  }

  // POST too big -> ideally 413.
  {
    std::string big((1u << 20) + 1, 'a');
    std::string response = send_and_recv_close(host, port, PostUploadRequest("/.upload_tmp/big.txt", big));
    if (response.empty())
      fail("POST too big: server closed without response; implement 413 check at header-stage (Content-Length)");

    expect_one_of(response, "POST too big", {413});
  }

  {
    std::string response = post_big_streaming(host, port, "/.upload_tmp/big.txt", (1u << 20) + 1);
    expect_one_of(response, "POST too big", {413});
  }

  // -------------------- DELETE tests --------------------

  // DELETE missing -> 404
  {
    std::string response = send_and_recv_close(host, port, DeleteReq("/nope.txt"));
    expect_one_of(response, "DELETE missing", {404});
  }

  // DELETE file -> 204 and file removed
  {
    write_file("./www/todel.txt", "x");
    std::string response = send_and_recv_close(host, port, DeleteReq("/todel.txt"));
    expect_one_of(response, "DELETE file", {204});
    if (fs::exists("./www/todel.txt"))
      fail("DELETE file: still exists");
  }

  // DELETE directory -> 403
  {
    fs::create_directories("./www/deldir");
    std::string response = send_and_recv_close(host, port, DeleteReq("/deldir"));
    expect_one_of(response, "DELETE directory forbidden", {403});
  }

  std::cerr << "kOk: integration tests passed\n";
  return 0;  // ServerGuard will stop the server
}
