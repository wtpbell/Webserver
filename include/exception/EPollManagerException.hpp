/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   EPollManagerException.hpp                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/12/04 17:08:28 by jboon         #+#    #+#                 */
/*   Updated: 2026/04/07 10:26:50 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef EPOLLMANAGEREXCEPTION_H_
#define EPOLLMANAGEREXCEPTION_H_

#include <exception>
#include <string>
#include <system_error>

class EPollManagerException : public std::exception
{
  public:
    EPollManagerException(const char* func_name, const int err,
                          const std::error_category& cat = std::system_category());
    virtual ~EPollManagerException(void) = default;

    const char* what(void) const noexcept override;

  private:
    std::string msg_;
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
