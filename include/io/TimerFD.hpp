/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   TimerFD.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/03/08 23:33:59 by jboon         #+#    #+#                 */
/*   Updated: 2026/03/09 00:18:15 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <time.h>

#include "Expected.hpp"
#include "io/SharedFD.hpp"

class TimerFD : public SharedFD
{
  public:
    TimerFD(void) = default;
    TimerFD(const TimerFD& other) = default;
    TimerFD(TimerFD&& other) noexcept = default;
    ~TimerFD(void) = default;

    TimerFD& operator=(const TimerFD& rhs) = default;
    TimerFD& operator=(TimerFD&& rhs) noexcept = default;

    static Expected<TimerFD, int> CreateRealtimeClock(time_t ms_duration, time_t ms_interval);
    bool SetTime(void);

  private:
    TimerFD(int fd, clockid_t clockid, time_t ms_duration, time_t ms_interval);

    clockid_t clockid_;
    time_t ms_duration_;
    time_t ms_interval_;
};
