/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   signal.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/05/10 15:23:58 by jboon         #+#    #+#                 */
/*   Updated: 2026/05/10 19:40:22 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIGNAL_H_
#define SIGNAL_H_

#include <atomic>

extern std::atomic<bool> g_shutdown;

void setupSignals(void);

#endif  // SIGNAL_H_
