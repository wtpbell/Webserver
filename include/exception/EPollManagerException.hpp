/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   EPollManagerException.hpp                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/04 17:08:28 by jboon         #+#    #+#                 */
/*   Updated: 2025/12/04 17:27:19 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef EPOLLMANAGEREXCEPTION_H_
#define EPOLLMANAGEREXCEPTION_H_

#include <exception>
#include <string>
#include <system_error>

class EPollManagerException : public std::exception
{
  private:
    std::string msg_;

  public:
    EPollManagerException(const char* func_name, const int err,
                          const std::error_category& cat = std::system_category());
    ~EPollManagerException(void) = default;

    const char* what(void) const noexcept override;
};

class ExistInEPollException : public EPollManagerException
{
  public:
    ExistInEPollException(const char* func_name, const int err, const std::error_category& cat = std::system_category())
        : EPollManagerException(func_name, err, cat)
    {
    }
    ~ExistInEPollException(void) = default;
};

class NotRegisteredInEPollException : public EPollManagerException
{
  public:
    NotRegisteredInEPollException(const char* func_name, const int err,
                                  const std::error_category& cat = std::system_category())
        : EPollManagerException(func_name, err, cat)
    {
    }
    ~NotRegisteredInEPollException(void) = default;
};

#endif  // EPOLLMANAGEREXCEPTION_H_
