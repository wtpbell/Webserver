/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   SharedFD.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/22 17:20:37 by jboon         #+#    #+#                 */
/*   Updated: 2026/01/13 19:08:30 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "io/SharedFD.hpp"

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstdarg>
#include <vector>

#include "Logger.hpp"
#include "exception/FileDescriptorException.hpp"
#include "webserv.hpp"

std::vector<int> SharedFD::shared_count_fds(1024);

SharedFD SharedFD::Open(const char* pathname, int flags, mode_t mode)
{
  int fd = open(pathname, flags, mode);
  if (fd == -1)
    throw FileDescriptorException(pathname, errno);
  return (SharedFD(fd));
}

std::pair<SharedFD, SharedFD> SharedFD::Pipe(void)
{
  int pipefd[2];
  if (pipe(pipefd) == -1)
    throw FileDescriptorException("pipe", errno);
  return (std::make_pair<SharedFD, SharedFD>(SharedFD(pipefd[0]), SharedFD(pipefd[1])));
}

std::pair<SharedFD, SharedFD> SharedFD::Pipe2(int flags)
{
  int pipefd[2];
  if (pipe2(pipefd, flags) == -1)
    throw FileDescriptorException("pipe2", errno);
  return (std::make_pair<SharedFD, SharedFD>(SharedFD(pipefd[0]), SharedFD(pipefd[1])));
}

SharedFD SharedFD::Dup(const SharedFD& oldfd)
{
  int fd = dup(oldfd.GetFD());
  if (fd == -1)
    throw FileDescriptorException("dup", errno);
  return (SharedFD(fd));
}

SharedFD SharedFD::Dup2(const SharedFD& oldfd, int newfd)
{
  if (newfd < 0)
    throw FileDescriptorException("dup2", "newfd cannot be less than zero");
  else if (static_cast<std::size_t>(newfd) < shared_count_fds.size() && shared_count_fds[newfd] > 0)
    throw FileDescriptorException("dup2", "newfd cannot be referenced by more than 1 SharedFD");
  int fd = dup2(oldfd.GetFD(), newfd);
  if (fd == -1)
    throw FileDescriptorException("dup2", errno);
  return (SharedFD(fd));
}

void SharedFD::Dup2(const SharedFD& oldfd, SharedFD& newfd)
{
  if (newfd.GetFD() == -1)
    throw FileDescriptorException("dup2", "newfd is not initialized");
  if (newfd.SharedCount() > 1)
    throw FileDescriptorException("dup2", "newfd cannot be referenced by more than 1 SharedFD");
  int replacefd = newfd.GetFD();
  newfd.Reset();
  newfd = Dup2(oldfd, replacefd);
}

SharedFD::SharedFD(int fd)
{
  Initialize(fd);
}

void SharedFD::Initialize(int fd)
{
  std::size_t req_size = static_cast<std::size_t>(fd);
  if (shared_count_fds.size() < req_size)
    shared_count_fds.resize(NextPOT(req_size));
  fd_ = fd;
  ++shared_count_fds[fd];
}

SharedFD::SharedFD(const SharedFD& other)
{
  fd_ = other.fd_;
  if (fd_ != -1)
    ++shared_count_fds[fd_];
}

SharedFD& SharedFD::operator=(const SharedFD& rhs)
{
  if (this == &rhs)
    return (*this);

  Close();
  fd_ = rhs.fd_;
  if (fd_ != -1)
    ++shared_count_fds[fd_];
  return (*this);
}

bool SharedFD::operator==(const SharedFD& rhs) const
{
  return (fd_ == rhs.fd_);
}

SharedFD::operator int() const
{
  return (fd_);
}

SharedFD::SharedFD(SharedFD&& other) noexcept
{
  Swap(other);
}

/*
 * @note It needs to Release() before the assignment, but we could also swap the contents and let
 * the rvalue object clean up.
 */
SharedFD& SharedFD::operator=(SharedFD&& rhs) noexcept
{
  if (this == &rhs)
    return (*this);

  Swap(rhs);
  return (*this);
}

SharedFD::~SharedFD(void)
{
  Close();
}

void SharedFD::Close(void) noexcept
{
  if (fd_ == -1)
    return;
  if (--shared_count_fds[fd_] == 0)
    close(fd_);
  fd_ = -1;
}

int SharedFD::SharedCount(void) const
{
  if (fd_ < 0 || shared_count_fds.size() < static_cast<std::size_t>(fd_))
    return (0);
  return (shared_count_fds[fd_]);
}

int SharedFD::GetFD(void) const
{
  return (fd_);
}

void SharedFD::Swap(SharedFD& other)
{
  std::swap(fd_, other.fd_);
}

void SharedFD::Reset(void)
{
  Close();
}
