/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   webserv.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/18 17:18:13 by jboon         #+#    #+#                 */
/*   Updated: 2025/11/20 11:50:09 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_H_
#define WEBSERV_H_

#include <atomic>

extern std::atomic<bool> g_shutdown;

void setupSignals(void);

#endif  // WEBSERV_H_
