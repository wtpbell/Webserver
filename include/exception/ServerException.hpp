/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerException.hpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/25 11:36:51 by jboon         #+#    #+#                 */
/*   Updated: 2025/12/15 14:22:54 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVEREXCEPTION_H_
#define SERVEREXCEPTION_H_

#include <exception>
#include <string>
#include <system_error>

class ServerException : public std::exception
{
  public:
    ServerException(const char* func_name, const int err, const std::error_category& cat = std::system_category());
    ServerException(const char* prefix, const char* msg);
    ~ServerException(void) = default;
    const char* what() const noexcept override;

  private:
    std::string msg_;
};

#endif  // SERVEREXCEPTION_H_
