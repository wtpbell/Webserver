/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   webserv.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/18 17:18:13 by jboon         #+#    #+#                 */
/*   Updated: 2025/12/11 10:28:14 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_H_
#define WEBSERV_H_

#include <sys/socket.h>

#include <atomic>
#include <cstddef>
#include <string>

extern std::atomic<bool> g_shutdown;

void setupSignals(void);
int LoadConfigs(int argc, char* argv[]);
std::size_t NextPOT(std::size_t n);
std::string GetNameInfo(struct sockaddr* addr, socklen_t addrlen);
std::string GetSocketInfo(int sfd);

#endif  // WEBSERV_H_
