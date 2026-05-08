/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGIError.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/03/17 00:27:38 by jboon         #+#    #+#                 */
/*   Updated: 2026/03/17 00:27:41 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_ERROR_H_
#define CGI_ERROR_H_

namespace cgi
{
  enum class CGIErrorCode
  {
    kNone,
    kNotFoundError,
    kMissingPermissionsError,
    kForkError,
    kExecveError,
    kSocketPairError,
    kForbidden,
  };
}

#endif  // CGI_ERROR_H_
