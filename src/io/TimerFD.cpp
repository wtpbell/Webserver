/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   TimerFD.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/03/08 23:33:59 by jboon         #+#    #+#                 */
/*   Updated: 2026/03/09 00:18:55 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "io/TimerFD.hpp"

#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

#include <cerrno>
#include <ctime>

#include "Expected.hpp"

#define MS_TO_S(ms) ((ms) / 1000)
#define S_TO_MS(s) ((s)*1000)
#define MS_TO_NS(ms) ((ms)*1000000)
#define NS_TO_S(ns) ((ns) / 1000000000)
#define S_TO_NS(s) ((s)*1000000000)

Expected<TimerFD, int> TimerFD::CreateRealtimeClock(time_t ms_duration, time_t ms_interval)
{
  int fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
  if (fd == -1)
    return static_cast<int>(errno);

  TimerFD timerfd{fd, CLOCK_REALTIME, ms_duration, ms_interval};
  if (!timerfd.SetTime())
    return static_cast<int>(errno);

  return timerfd;
}

bool TimerFD::SetTime(void)
{
  timespec now;
  if (clock_gettime(clockid_, &now) == -1)
    return false;

  int seconds = MS_TO_S(ms_duration_);
  int nanoseconds = MS_TO_NS(ms_duration_ - S_TO_MS(seconds));
  int remain_seconds = NS_TO_S(now.tv_nsec + nanoseconds);
  nanoseconds = (now.tv_nsec + nanoseconds) - S_TO_NS(remain_seconds);

  itimerspec time_spec;
  // When the clock will expire
  time_spec.it_value.tv_sec = now.tv_sec + seconds + remain_seconds;
  time_spec.it_value.tv_nsec = nanoseconds;

  // Time between ticks after expiration
  seconds = MS_TO_S(ms_interval_);
  time_spec.it_interval.tv_sec = seconds;
  time_spec.it_interval.tv_nsec = MS_TO_NS(ms_interval_ - S_TO_MS(seconds));

  return timerfd_settime(fd_, TFD_TIMER_ABSTIME, &time_spec, NULL) == 0;
}

TimerFD::TimerFD(int fd, clockid_t clockid, time_t ms_duration, time_t ms_interval)
    : SharedFD(fd), clockid_(clockid), ms_duration_(ms_duration), ms_interval_(ms_interval)
{
}
