/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerException.cpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.ccodam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/11/25 11:43:18 by jboon         #+#    #+#                 */
/*   Updated: 2025/12/02 15:01:17 by jboon         ########   codam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "exception/ServerException.hpp"

#include <cstring>
#include <string>

static std::string ServerErrorMessage(const std::error_code& code)
{
  if (code == std::errc::operation_would_block || code == std::errc::resource_unavailable_try_again)
    return (std::string("No pending connections to accept (non-blocking)"));
  else if (code == std::errc::invalid_argument)
    return (std::string("Socket is not listening for connections, addrlen is invalid, or invalid value in flags"));
  else if (code == std::errc::operation_not_permitted)
    return (std::string("Filewall rules forbid connection"));
  else if (code == std::errc::bad_address)
    return (std::string("The addr argument is not in a writable part of the user address space"));
  else if (code == std::errc::address_in_use)
    return (std::string("Another socket is already listening on the same port"));
  else if (code == std::errc::operation_not_supported)
    return (std::string("The socket is not of a type that supports the listen() operation"));
  return (code.message());
}

ServerException::ServerException(const char* func_name, const int err, const std::error_category& cat)
{
  std::error_code error{err, cat};

  msg_ += func_name;
  msg_ += "() - ";
  msg_ += strerrorname_np(error.value());
  msg_ += "(";
  msg_ += std::to_string(err);
  msg_ += ")";
  msg_ += ": ";
  msg_ += ServerErrorMessage(error);
}

ServerException::ServerException(const char* prefix, const char* msg)
{
  msg_ += prefix;
  msg_ += ": ";
  msg_ += msg;
}

const char* ServerException::what(void) const noexcept
{
  return (msg_.c_str());
}
