/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   webserv.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/18 17:18:13 by jboon         #+#    #+#                 */
/*   Updated: 2026/01/13 19:07:03 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_H_
#define WEBSERV_H_

#include <atomic>
#include <cstddef>
#include <string>
#include <vector>

extern std::atomic<bool> g_shutdown;

void setupSignals(void);
int LoadConfigs(int argc, char* argv[]);
std::size_t NextPOT(std::size_t n);
std::vector<char*> ConvertToCstrVector(const std::vector<std::string>& v);

#endif  // WEBSERV_H_
