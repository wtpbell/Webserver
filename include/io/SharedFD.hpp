/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   SharedFD.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/22 13:59:33 by jboon         #+#    #+#                 */
/*   Updated: 2026/01/08 19:57:07 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SHARED_FD_H_
#define SHARED_FD_H_

#include <fcntl.h>
#include <netdb.h>

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

    bool operator==(const SharedFD& rhs) const;
    explicit operator int() const;
    virtual ~SharedFD(void);

    static SharedFD Open(const char* pathname, int flags, mode_t mode = (S_IRWXU & S_IRWXG));
    static std::pair<SharedFD, SharedFD> Pipe(void);
    static std::pair<SharedFD, SharedFD> Pipe2(int flags);
    static SharedFD Dup(const SharedFD& oldfd);
    static SharedFD Dup2(const SharedFD& oldfd, int newfd);
    static void Dup2(const SharedFD& oldfd, SharedFD& newfd);

    int SharedCount(void) const;
    int GetFD(void) const;
    void Swap(SharedFD& other);
    void Reset(void);

  protected:
    explicit SharedFD(int fd);
    void Initialize(int fd);

    int fd_ = -1;

  private:
    void Close(void) noexcept;

    static std::vector<int> shared_count_fds;

#ifdef UNIT_TEST
  public:
    static std::vector<int>& GetSharedCountVector(void)
    {
      return (shared_count_fds);
    }
#endif
};

#endif  // SHARED_FD_H_
