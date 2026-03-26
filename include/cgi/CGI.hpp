/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGI.hpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/06 14:56:14 by jboon         #+#    #+#                 */
/*   Updated: 2026/02/15 14:52:02 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_H_
#define CGI_H_

#include <filesystem>
#include <string_view>

#include "Expected.hpp"
#include "cgi/CGIError.hpp"
#include "cgi/CGIProcess.hpp"
#include "http/HTTPRequest.hpp"

namespace cgi
{
  using Path = std::filesystem::path;

  struct CGIRoute
  {
      Path script_;
      Path resource_;
      Path full_resource_;
  };

  // TODO: temp struct...
  struct Route
  {
      Path root;
  };

  bool inline IsChildProcessFailure(const CGIErrorCode error_code) noexcept
  {
    return error_code == CGIErrorCode::kRedirectionError || error_code == cgi::CGIErrorCode::kScriptExecveError;
  }

  int inline ChildProcessExitCode(const CGIErrorCode error_code) noexcept
  {
    if (error_code == cgi::CGIErrorCode::kScriptExecveError)
      return (errno == ENOENT) ? 127 : 126;
    return 1;
  }

  Path ReplaceScriptRoot(std::string_view script, std::string_view mapping, const Path& root) noexcept;
  Path ExtractResourcePath(std::string_view script, std::string_view target) noexcept;
  Expected<CGIRoute, CGIErrorCode> SetupCGIRoute(const Path& url, const Path& root, std::string_view mapping) noexcept;
  Expected<CGIProcess, CGIErrorCode> ExecuteCGI(const HTTPRequest& http_request, Route route,
                                                std::string_view server_info, std::string_view client_info);
}  // namespace cgi
#endif  // CGI_H_
