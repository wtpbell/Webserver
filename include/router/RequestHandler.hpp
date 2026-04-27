/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   RequestHandler.hpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/29 13:10:28 by bewong        #+#    #+#                 */
/*   Updated: 2026/01/29 13:10:28 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include <filesystem>
#include <optional>
#include <string_view>

#include "config/RouteView.hpp"
#include "http/HTTPRequest.hpp"
#include "http/HTTPResponse.hpp"

/*
    - method dispatch (GET/POST/DELETE)
    - static file serving
    - DELETE unlink
    - POST body acceptance
    - build correct HTTPResponse
*/

namespace request_handler
{
  namespace fs = std::filesystem;
  using Method = RouteView::MethodMask;
  using Status = HTTP::Status;

  enum class FileStatusResult
  {
    kOk,
    kPermissionDenied,
    kIOError
  };

  struct PathInfo
  {
      bool exists = false;
      bool isDir = false;
      bool isFile = false;
      bool statError = false;
      std::error_code ec{};
  };

  bool HasAnyPerm(const fs::file_status& status, fs::perms owner, fs::perms group, fs::perms others);
  bool IsPermDenied(const std::error_code& ec);
  Status MapStatError(const std::error_code& ec);
  Method MethodToMask(std::string_view m);

  FileStatusResult ReadFile(const fs::path& path, std::string& out);
  FileStatusResult WriteFile(const fs::path& path, std::string_view data);
  FileStatusResult MoveTempIntoPlace(const fs::path& tmp, const fs::path& target, bool targetExisted);
  FileStatusResult FinalizeBodyToPath(const HTTPRequest& request, const fs::path& target, bool existed);
  PathInfo InspectPath(const fs::path& path);
  std::string_view ComputeRouteTail(std::string_view path, std::string_view locationPref);

  std::optional<HTTPResponse> ValidatePostTarget(const fs::path& path, const PathInfo& info);
  std::optional<HTTPResponse> FileStatusToHTTP(FileStatusResult result);
  void CleanupTempOnFailure(const HTTPRequest& request);
  std::string BuildAllowHeader(RouteView::MethodMask mask);

  std::optional<HTTPResponse> TryServeIndex(const RouteView& route, const fs::path& dirFsPath);
  HTTPResponse MakeAutoIndex(const RouteView& route, const fs::path& dirFsPath, std::string_view urlPath);
  HTTPResponse HandleDirectoryGet(const RouteView& route, const fs::path& dirFsPath, std::string_view urlPath);

  HTTPResponse HandleMethods(const HTTPRequest& request, const RouteView& route, std::string_view remainder);
  HTTPResponse HandleCgi(const HTTPRequest& request, const RouteView& route, std::string_view remainder);

  HTTPResponse HandleGet(const HTTPRequest& request, const RouteView& route, std::string_view remainder);
  HTTPResponse HandlePost(const HTTPRequest& request, const RouteView& route, std::string_view remainder);
  HTTPResponse HandleDelete(const HTTPRequest& request, const RouteView& route, std::string_view remainder);

  std::optional<fs::path> ResolvePath(const RouteView& route, const std::string_view path,
                                                   std::string_view remainder);

};  // namespace request_handler

#endif
