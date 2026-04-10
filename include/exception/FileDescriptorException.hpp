/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   FileDescriptorException.hpp                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/22 17:19:55 by jboon         #+#    #+#                 */
/*   Updated: 2026/04/07 10:27:24 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef FILE_DESCRIPTOR_EXCEPTION_H_
#define FILE_DESCRIPTOR_EXCEPTION_H_

#include <exception>
#include <string>

class FileDescriptorException : public std::exception
{
  public:
    // TODO: Add file descriptor number to the message
    // TODO: Support gaierror
    FileDescriptorException(const char* pathname, int err);
    FileDescriptorException(const char* prefix, const char* msg);
    const char* what(void) const noexcept override;

  private:
    std::string message_;
};

#endif  // FILE_DESCRIPTOR_EXCEPTION_H_
