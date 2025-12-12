/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   webserv.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/18 17:18:13 by jboon         #+#    #+#                 */
/*   Updated: 2025/12/02 15:58:51 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_H_
#define WEBSERV_H_

#include <atomic>
#include <cstddef>

extern std::atomic<bool> g_shutdown;

void setupSignals(void);
int LoadConfigs(int argc, char* argv[]);
std::size_t NextPOT(std::size_t n);

#endif  // WEBSERV_H_
