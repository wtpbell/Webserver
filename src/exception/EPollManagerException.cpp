/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   EPollManagerException.cpp                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/04 17:16:07 by jboon         #+#    #+#                 */
/*   Updated: 2026/04/07 10:27:07 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "exception/EPollManagerException.hpp"

#include <cstring>

EPollManagerException::EPollManagerException(const char* func_name, const int err, const std::error_category& cat)
{
  std::error_code error{err, cat};

  msg_ += func_name;
  msg_ += "() - ";
  msg_ += strerrorname_np(error.value());
  msg_ += "(";
  msg_ += std::to_string(err);
  msg_ += ")";
  msg_ += ": ";
  msg_ += error.message();
}

const char* EPollManagerException::what(void) const noexcept
{
  return (msg_.c_str());
}
