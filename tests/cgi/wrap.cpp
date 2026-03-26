#include <errno.h>
#include <unistd.h>

#ifdef FORK_COUNT
static int max_fork_count = FORK_COUNT;
#else
static int max_fork_count = 100;
#endif

extern "C" pid_t __real_fork(void);

extern "C" pid_t __wrap_fork(void)
{
  if (max_fork_count--)
    return __real_fork();
  errno = ENOMEM;
  return -1;
}

#ifdef PAIR_COUNT
static int max_pair_count = PAIR_COUNT;
#else
static int max_pair_count = 100;
#endif

extern "C" int __real_socketpair(int domain, int type, int protocol, int sv[2]);

extern "C" int __wrap_socketpair(int domain, int type, int protocol, int sv[2])
{
  if (max_pair_count--)
    return __real_socketpair(domain, type, protocol, sv);
  errno = EMFILE;
  return -1;
}

#ifdef DUP_COUNT
static int max_dup_count = DUP_COUNT;
#else
static int max_dup_count = 100;
#endif

extern "C" int __real_dup2(int oldfd, int newfd);

extern "C" int __wrap_dup2(int oldfd, int newfd)
{
  if (max_dup_count--)
    return __real_dup2(oldfd, newfd);
  errno = EMFILE;
  return -1;
}
