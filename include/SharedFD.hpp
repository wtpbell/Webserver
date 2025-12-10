/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   SharedFD.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/22 13:59:33 by jboon         #+#    #+#                 */
/*   Updated: 2025/12/09 14:26:33 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SHARED_FD_H_
#define SHARED_FD_H_

#include <fcntl.h>

#include <memory>
#include <utility>
#include <vector>

class SharedFD
{
  public:
    SharedFD(void) = default;
    SharedFD(const SharedFD& other);
    SharedFD(SharedFD&& other) noexcept;
    SharedFD& operator=(const SharedFD& rhs);
    SharedFD& operator=(SharedFD&& rhs) noexcept;
    explicit operator int() const;
    ~SharedFD(void);

    static SharedFD Open(const char* pathname, int flags, mode_t mode = (S_IRWXU & S_IRWXG));
    static SharedFD Socket(int domain, int type, int protocol);
    static std::pair<SharedFD, SharedFD> SocketPair(int domain, int type, int protocol);
    static std::pair<SharedFD, SharedFD> Pipe(void);
    static std::pair<SharedFD, SharedFD> Pipe2(int flags);
    static SharedFD Dup(const SharedFD& oldfd);
    static SharedFD Dup2(const SharedFD& oldfd, int newfd);
    static void Dup2(const SharedFD& oldfd, SharedFD& newfd);

    int SharedCount(void) const;
    int GetFD(void) const;
    void Swap(SharedFD& other);
    void Reset(void);

  private:
    static std::vector<int> shared_count_fds;

    explicit SharedFD(int fd);

    void Close(void) noexcept;

    int fd_ = -1;

#ifdef UNIT_TEST
  public:
    static std::vector<int>& GetSharedCountVector(void)
    {
      return (shared_count_fds);
    }
#endif
};

#endif  // SHARED_FD_H_
