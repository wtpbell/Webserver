/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   FileDescriptorException.hpp                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/22 17:19:55 by jboon         #+#    #+#                 */
/*   Updated: 2025/11/24 18:17:34 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef FILE_DESCRIPTOR_EXCEPTION_H_
#define FILE_DESCRIPTOR_EXCEPTION_H_

#include <exception>
#include <string>

class FileDescriptorException : public std::exception
{
  private:
    std::string message_;

  public:
    FileDescriptorException(const char* pathname, int err);
    FileDescriptorException(const char* prefix, const char* msg);
    const char* what(void) const noexcept override;
};

#endif  // FILE_DESCRIPTOR_EXCEPTION_H_
