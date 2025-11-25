/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   signal.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/20 11:45:58 by jboon         #+#    #+#                 */
/*   Updated: 2025/11/20 16:47:45 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <atomic>
#include <csignal>
#include <stdexcept>

std::atomic<bool> g_shutdown(false);

static void signalHandler(int sig)
{
  if (sig == SIGINT || sig == SIGTERM)
    g_shutdown.store(true);
}

/*
SIGPIPE: This signal is generated when a process tries to write to a pipe, a FIFO, or a
socket for which there is no corresponding reader process. This normally
occurs because the reading process has closed its file descriptor for the
IPC channel. See Section 44.2 for further details.

Keep ignoring SIGPIPE (prevent crashes on closed sockets)
https://stackoverflow.com/questions/21687695/getting-sigpipe-with-non-blocking-sockets-is-this-normal
*/
void setupSignals(void)
{
  struct sigaction sa{};
  sa.sa_handler = signalHandler;
  sa.sa_flags = 0;
  if (sigemptyset(&sa.sa_mask) < 0)
    throw std::runtime_error("sigemptyset safailed");
  if (sigaction(SIGINT, &sa, nullptr) < 0 || sigaction(SIGTERM, &sa, nullptr) < 0)
    throw std::runtime_error("SIGINT and SIGTERM caught");

  struct sigaction ign{};
  ign.sa_handler = SIG_IGN;
  ign.sa_flags = 0;
  if (sigemptyset(&ign.sa_mask) < 0)
    throw std::runtime_error("sigemptyset ign failed");
  if (sigaction(SIGPIPE, &ign, nullptr) < 0)
    throw std::runtime_error("SIGPIPE caught");
}
