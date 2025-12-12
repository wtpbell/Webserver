/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/13 17:24:28 by jboon         #+#    #+#                 */
/*   Updated: 2025/12/02 15:59:34 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <cstdlib>

#include "webserv.hpp"

int main(int argc, char* argv[])
{
  setupSignals();

  if (LoadConfigs(argc, argv) != EXIT_SUCCESS)
    return (EXIT_FAILURE);

  return (EXIT_SUCCESS);
}
