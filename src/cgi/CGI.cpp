/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGI.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/08 19:15:05 by jboon         #+#    #+#                 */
/*   Updated: 2026/05/10 19:41:51 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "cgi/CGI.hpp"

#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string_view>
#include <system_error>
#include <utility>

#include "cgi/CGIError.hpp"
#include "cgi/CGIProcess.hpp"
#include "cgi/CGIRequest.hpp"
#include "config/RouteView.hpp"
#include "config/ServerRegistry.hpp"
#include "http/HTTPRequest.hpp"
#include "http/HTTPStatus.hpp"
#include "router/RequestHandler.hpp"
#include "utils/Expected.hpp"
#include "utils/Logger.hpp"
#include "utils/string.hpp"

/* =============================== PUBLIC =================================== */
namespace cgi
{
  namespace
  {
    std::pair<SharedFD, SharedFD> RedirectStandardIO(const Socket& socket)
    {
      try
      {
        return {SharedFD::Dup2(socket, STDIN_FILENO), SharedFD::Dup2(socket, STDOUT_FILENO)};
      }
      catch (const std::exception& ex)
      {
        Logger::Log(LogLevel::ERROR, "Failed to redirect {} to stdin and stdout", socket);
      }
      return {};
    }

    Expected<CGIProcess, CGIErrorCode> Fork(CGIRequest&& cgi_request) noexcept
    {
      std::pair<Socket, Socket> pair;
      try
      {
        pair = Socket::SocketPair(AF_UNIX, SOCK_STREAM, 0);
      }
      catch (const std::exception& ex)
      {
        return CGIErrorCode::kSocketPairError;
      }

      CGIProcess process{::fork(), std::move(cgi_request), std::move(pair)};
      if (process.IsError())
        return CGIErrorCode::kForkError;
      return process;
    }

    std::filesystem::path GetExtensionScript(
        const std::map<std::string, std::filesystem::path, std::less<>>& extensionPaths, std::string_view target)
    {
      std::size_t delim{target.rfind(".")};
      if (delim == target.npos)
      {
        return {};
      }
      target.remove_prefix(delim);

      auto it = extensionPaths.find(target);
      if (it == extensionPaths.end())
      {
        return {};
      }
      return it->second;
    }

    Expected<CGIProcess, CGIErrorCode> ExecuteCgiExtension(const HTTPRequest& httpRequest, const RouteView& route,
                                                           const IpPort& ipPort, std::string_view clientInfo)
    {
      std::filesystem::path executablePath{GetExtensionScript(route.cgiExePaths.value(), httpRequest.GetPath())};
      if (executablePath.empty())
      {
        return CGIErrorCode::kNone;
      }

      std::optional<std::filesystem::path> fullPath = request_handler::ResolvePath(route, httpRequest.GetPath());
      if (!fullPath.has_value())
      {
        return CGIErrorCode::kForbidden;
      }
      request_handler::PathInfo pathInfo = request_handler::InspectPath(*fullPath);
      if (pathInfo.statError)
      {
        return CGIErrorCode::kForbidden;
      }
      else if (!pathInfo.exists)
      {
        return CGIErrorCode::kNotFoundError;
      }
      else if (!pathInfo.isFile || !pathInfo.canRead || !pathInfo.canExecute)
      {
        return CGIErrorCode::kForbidden;
      }
      CGIRoute cgiRoute{executablePath.generic_string(), *fullPath, "", ""};
      return ExecuteCGI(CGIRequest(httpRequest, cgiRoute, ipPort, clientInfo, route));
    }

    Expected<std::string, CGIErrorCode> GetFullResourcePath(const std::string& resource, const IpPort& ipPort,
                                                            const std::string_view hostname,
                                                            const ServerRegistry& serverRegistery) noexcept
    {
      const RouteView* route = serverRegistery.GetRouteView(ipPort.ip, ipPort.port, hostname, resource);
      if (route == nullptr)
      {
        return CGIErrorCode::kNotFoundError;
      }

      std::optional<Path> fullResourcePath = request_handler::ResolvePath(*route, resource);
      if (!fullResourcePath.has_value())
      {
        return CGIErrorCode::kForbidden;
      }
      return fullResourcePath->generic_string();
    }

    Expected<CGIProcess, CGIErrorCode> ExecuteCgiEnabled(const HTTPRequest& httpRequest, const RouteView& route,
                                                         const ServerRegistry& serverRegistry, const IpPort& ipPort,
                                                         std::string_view clientInfo)
    {
      auto expCgiRoute = SetupCGIRoute(httpRequest.GetPath(), route);
      if (!expCgiRoute.HasValue())
      {
        return expCgiRoute.ExtractError();
      }
      if (!expCgiRoute->resource_.empty() && expCgiRoute->resource_ != "/")
      {
        auto fullResourcePath =
            GetFullResourcePath(expCgiRoute->resource_, ipPort, httpRequest.GetHost(), serverRegistry);
        if (!fullResourcePath.HasValue())
        {
          return fullResourcePath.ExtractError();
        }
        expCgiRoute->fullResource_ = fullResourcePath.GetValue();
      }
      return ExecuteCGI(CGIRequest(httpRequest, expCgiRoute.GetValue(), ipPort, clientInfo, route));
    }
  }  // namespace

  HTTP::Status CGIErrorCodeToHTTPCode(const CGIErrorCode errorCode) noexcept
  {
    switch (errorCode)
    {
      case CGIErrorCode::kForkError:
      case CGIErrorCode::kExecveError:
      case CGIErrorCode::kSocketPairError:
        return HTTP::Status::kInternalServerError;
      case CGIErrorCode::kForbidden:
      case CGIErrorCode::kMissingPermissionsError:
      case CGIErrorCode::kNotFoundError:
        return HTTP::Status::kForbidden;
      case CGIErrorCode::kNone:
        return HTTP::Status::kOk;
    }
    assert(false && "Invalid CGI error code");
    __builtin_unreachable();
  }

  Expected<CGIRoute, CGIErrorCode> SetupCGIRoute(std::string_view target, const RouteView& route) noexcept
  {
    namespace fs = std::filesystem;
    namespace reqHandler = request_handler;

    std::optional<Path> fullScriptPath = reqHandler::ResolvePath(route, target);
    if (!fullScriptPath.has_value())
    {
      return CGIErrorCode::kForbidden;
    }

    std::error_code errorCode;
    const Path root = route.alias ? fs::weakly_canonical(*route.alias, errorCode)
                                  : fs::weakly_canonical(route.root, errorCode).concat(route.locationPrefix);
    if (errorCode)
    {
      return CGIErrorCode::kForbidden;
    }
    errorCode.clear();

    const fs::perms readExecuteOwnerPerms = (fs::perms::owner_exec | fs::perms::owner_read);
    const fs::perms readExecuteGroupPerms = (fs::perms::group_exec | fs::perms::group_read);
    Path script = *fullScriptPath;
    while (script != root && script.has_parent_path())
    {
      fs::file_status scriptStatus{fs::symlink_status(script, errorCode)};
      if (!errorCode && fs::exists(scriptStatus) && fs::is_regular_file(scriptStatus))
      {
        if ((scriptStatus.permissions() & readExecuteOwnerPerms) == readExecuteOwnerPerms ||
            (scriptStatus.permissions() & readExecuteGroupPerms) == readExecuteGroupPerms)
        {
          std::string scriptTarget = script.generic_string();
          std::string fullScriptTarget = fullScriptPath->generic_string();
          return CGIRoute{scriptTarget, scriptTarget,
                          std::string(request_handler::ComputeRouteTail(fullScriptTarget, scriptTarget)), ""};
        }
        return CGIErrorCode::kMissingPermissionsError;
      }
      script = script.parent_path();
    }
    return CGIErrorCode::kNotFoundError;
  }

  Expected<CGIProcess, CGIErrorCode> DispatchCGIHandler(const HTTPRequest& httpRequest, const RouteView& route,
                                                        const ServerRegistry& serverRegistry, IpPort ipPort,
                                                        std::string_view clientInfo)
  {
    if (route.cgiExePaths.has_value())
    {
      Expected<CGIProcess, CGIErrorCode> process = ExecuteCgiExtension(httpRequest, route, ipPort, clientInfo);
      if (process.HasValue())
      {
        return process.ExtractValue();
      }
      if (process.GetError() != CGIErrorCode::kNone)
      {
        return process.ExtractError();
      }
    }

    if (route.cgi)
    {
      return ExecuteCgiEnabled(httpRequest, route, serverRegistry, ipPort, clientInfo);
    }

    return CGIErrorCode::kNone;
  }

  /// @brief Execute script in a new process
  Expected<CGIProcess, CGIErrorCode> ExecuteCGI(CGIRequest&& cgiRequest)
  {
    auto process = Fork(std::move(cgiRequest));
    if (!process.HasValue())
    {
      return process.ExtractError();
    }
    if (process->IsParent())
    {
      return process.ExtractValue();
    }

    auto std_io = RedirectStandardIO(process->GetSocket());
    if (std_io.first.GetFD() < 0 || std_io.second.GetFD() < 0)
    {
      std::_Exit(EXIT_FAILURE);
    }

    const char* executable = process->GetRequest().GetExecutable().data();
    const std::vector<char*> argv{String::ConvertToCstrVector(process->GetRequest().GetArgv())};
    const std::vector<char*> envp{String::ConvertToCstrVector(process->GetRequest().GetEnvp())};
    const char* path{*argv.data()};

    if (setpgid(0, 0) != 0)  // Create own process group for cgi script
    {
      Logger::Log(LogLevel::ERROR, "Child: setpgid({}) failed: {}", path, errno);
      std::_Exit(EXIT_FAILURE);
    }

    execve(executable, argv.data(), envp.data());
    Logger::Log(LogLevel::ERROR, "Child: execve({}) failed: {}", path, errno);
    std::_Exit(126);
  }
}  // namespace cgi
