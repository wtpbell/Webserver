/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   FileDescriptorException.cpp                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/23 12:09:24 by jboon         #+#    #+#                 */
/*   Updated: 2025/11/24 18:17:36 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "exception/FileDescriptorException.hpp"

#include <cstring>

FileDescriptorException::FileDescriptorException(const char* pathname, int err)
    : FileDescriptorException(pathname, strerror(err))
{
}

FileDescriptorException::FileDescriptorException(const char* prefix, const char* msg)
{
  message_.reserve(256);
  message_ += prefix;
  message_ += ": ";
  message_ += msg;
}

const char* FileDescriptorException::what(void) const noexcept
{
  return (message_.c_str());
}
