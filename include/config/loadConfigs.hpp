/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   loadConfigs.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/05/10 15:40:13 by jboon         #+#    #+#                 */
/*   Updated: 2026/05/10 19:40:44 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOADCONFIGS_H_
#define LOADCONFIGS_H_

#include <optional>

#include "config/ServerRegistry.hpp"

std::optional<ServerRegistry> LoadConfigs(int argc, char* argv[]);

#endif  // LOADCONFIGS_H_
