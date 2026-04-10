/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   test_TimerFD.cpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/03/09 13:45:27 by jboon         #+#    #+#                 */
/*   Updated: 2026/03/09 13:45:28 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <sys/time.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <chrono>
#include <ctime>
#include <thread>

#include "Expected.hpp"
#include "catch_amalgamated.hpp"
#include "io/TimerFD.hpp"

itimerspec GetTime(const TimerFD& timerfd)
{
  itimerspec curr_value;

  REQUIRE(timerfd_gettime(timerfd.GetFD(), &curr_value) == 0);
  return curr_value;
}

timespec GetNow(void)
{
  timespec curr_value;

  REQUIRE(clock_gettime(CLOCK_MONOTONIC, &curr_value) == 0);
  return curr_value;
}

TEST_CASE("Create TimerFD with 1500ms duration", "[TimerFD]")
{
  int delay = 1500;
  Expected<TimerFD, int> timerfd = TimerFD::CreateRealtimeClock(delay, 0);

  if (!timerfd.HasValue())
  {
    CAPTURE(timerfd.GetError());
  }
  REQUIRE(timerfd.HasValue());

  itimerspec curr_value = GetTime(timerfd.GetValue());

  REQUIRE(curr_value.it_value.tv_sec == 1);
  REQUIRE(curr_value.it_value.tv_nsec > 499980000);
  REQUIRE(curr_value.it_value.tv_nsec < 500010000);

  REQUIRE(curr_value.it_interval.tv_sec == 0);
  REQUIRE(curr_value.it_interval.tv_nsec == 0);
}

TEST_CASE("Create TimerFD with 250ms duration", "[TimerFD]")
{
  int delay = 250;
  Expected<TimerFD, int> timerfd = TimerFD::CreateRealtimeClock(delay, 0);

  if (!timerfd.HasValue())
  {
    CAPTURE(timerfd.GetError());
  }
  REQUIRE(timerfd.HasValue());

  itimerspec curr_value = GetTime(timerfd.GetValue());

  REQUIRE(curr_value.it_value.tv_sec == 0);
  REQUIRE(curr_value.it_value.tv_nsec >= 249900000);
  REQUIRE(curr_value.it_value.tv_nsec <= 250010000);

  REQUIRE(curr_value.it_interval.tv_sec == 0);
  REQUIRE(curr_value.it_interval.tv_nsec == 0);
}

TEST_CASE("Create TimerFD with 25847ms duration", "[TimerFD]")
{
  int delay = 25847;
  Expected<TimerFD, int> timerfd = TimerFD::CreateRealtimeClock(delay, 0);

  if (!timerfd.HasValue())
  {
    CAPTURE(timerfd.GetError());
  }
  REQUIRE(timerfd.HasValue());

  itimerspec curr_value = GetTime(timerfd.GetValue());

  REQUIRE(curr_value.it_value.tv_sec == 25);
  REQUIRE(curr_value.it_value.tv_nsec > 846980000);
  REQUIRE(curr_value.it_value.tv_nsec < 847010000);

  REQUIRE(curr_value.it_interval.tv_sec == 0);
  REQUIRE(curr_value.it_interval.tv_nsec == 0);
}

TEST_CASE("Create TimerFD with 0ms duration", "[TimerFD]")
{
  int delay = 0;
  Expected<TimerFD, int> timerfd = TimerFD::CreateRealtimeClock(delay, 0);

  if (!timerfd.HasValue())
  {
    CAPTURE(timerfd.GetError());
  }
  REQUIRE(timerfd.HasValue());

  itimerspec curr_value = GetTime(timerfd.GetValue());

  REQUIRE(curr_value.it_value.tv_sec == 0);
  REQUIRE(curr_value.it_value.tv_nsec == 0);

  REQUIRE(curr_value.it_interval.tv_sec == 0);
  REQUIRE(curr_value.it_interval.tv_nsec == 0);
}

TEST_CASE("Create TimerFD with 12435ms duration and 11734ms interval", "[TimerFD]")
{
  int delay = 12435;
  int interval = 11734;
  Expected<TimerFD, int> timerfd = TimerFD::CreateRealtimeClock(delay, interval);

  if (!timerfd.HasValue())
  {
    CAPTURE(timerfd.GetError());
  }
  REQUIRE(timerfd.HasValue());

  itimerspec curr_value = GetTime(timerfd.GetValue());

  REQUIRE(curr_value.it_value.tv_sec == 12);
  REQUIRE(curr_value.it_value.tv_nsec > 434980000);
  REQUIRE(curr_value.it_value.tv_nsec < 435010000);

  REQUIRE(curr_value.it_interval.tv_sec == 11);
  REQUIRE(curr_value.it_interval.tv_nsec == 734000000);
}

TEST_CASE("SetTime multiple times", "[TimerFD]")
{
  int delay = 12435;
  int interval = 11734;
  Expected<TimerFD, int> timerfd = TimerFD::CreateRealtimeClock(delay, interval);

  if (!timerfd.HasValue())
  {
    CAPTURE(timerfd.GetError());
  }
  REQUIRE(timerfd.HasValue());

  for (int i = 0; i < 5; ++i)
  {
    itimerspec curr_value = GetTime(timerfd.GetValue());

    REQUIRE(curr_value.it_value.tv_sec == 12);
    REQUIRE(curr_value.it_value.tv_nsec > 434950000);
    REQUIRE(curr_value.it_value.tv_nsec < 435010000);

    REQUIRE(curr_value.it_interval.tv_sec == 11);
    REQUIRE(curr_value.it_interval.tv_nsec == 734000000);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    REQUIRE(timerfd->SetTime());
  }
}
