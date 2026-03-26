/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGI.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: jboon <jboon@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/01/08 19:15:05 by jboon         #+#    #+#                 */
/*   Updated: 2026/02/24 10:54:44 by jboon         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "cgi/CGI.hpp"

#include <cerrno>
#include <filesystem>
#include <string_view>
#include <system_error>
#include <utility>

#include "Expected.hpp"
#include "Logger.hpp"
#include "cgi/CGIError.hpp"
#include "cgi/CGIProcess.hpp"
#include "cgi/CGIRequest.hpp"
#include "http/HTTPRequest.hpp"
#include "string.hpp"
#include "webserv.hpp"

/*
  Currently any url that starts with cgi-bin is considered a cgi request
  if the cgi does not resolve to a valid script than HTTP 404 should be returned.
*/

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
  }  // namespace

  Path ReplaceScriptRoot(std::string_view script, std::string_view mapping, const Path& root) noexcept
  {
    if (!mapping.empty())
      mapping.remove_prefix(mapping[0] == '/');
    if (!script.empty())
      script.remove_prefix(script[0] == '/');

    if (script.compare(0, mapping.length(), mapping) != 0)
      return {};

    if (script.length() > mapping.length())
      script.remove_prefix(mapping.length() + (script[mapping.length()] == '/'));
    else
      script.remove_prefix(mapping.length());
    return root / script;
  }

  Path ExtractResourcePath(std::string_view script, std::string_view target) noexcept
  {
    if (script.empty() || target.empty())
    {
      return {};
    }

    script.remove_prefix(script[0] == '/');
    target.remove_prefix(target[0] == '/');
    while (!script.empty() && target.compare(0, script.length(), script) != 0)
    {
      std::size_t slash = script.find_first_of('/');
      if (slash == std::string_view::npos)
      {
        slash = script.length();
      }
      else
      {
        slash += 1;
      }
      script.remove_prefix(slash);
    }

    if (!script.empty())
    {
      target.remove_prefix(script.length());
      return Path{target};
    }
    return {};
  }

  Expected<CGIRoute, CGIErrorCode> SetupCGIRoute(const Path& target, const Path& root,
                                                 std::string_view mapping) noexcept
  {
    namespace fs = std::filesystem;

    Path script{ReplaceScriptRoot(target.generic_string(), mapping, root)};
    std::error_code error_code;
    while (script != "/" && !script.empty())
    {
      if (fs::is_symlink(script, error_code))
      {
        return CGIErrorCode::kScriptIsSymlinkError;  // TODO: allow for symlinks that are still within the root
      }

      fs::file_status script_status{fs::status(script, error_code)};
      if (!error_code && fs::exists(script_status))
      {
        if (fs::is_regular_file(script_status))
        {
          if ((script_status.permissions() & fs::perms::owner_exec) == fs::perms::owner_exec)
          {
            return CGIRoute{script, ExtractResourcePath(script.generic_string(), target.generic_string()), ""};
          }
          return CGIErrorCode::kScriptMissingPermissionsError;
        }
      }

      script = script.parent_path();
    }
    return CGIErrorCode::kScriptNotFoundError;
  }

  /// @brief Execute script in a new process
  Expected<CGIProcess, CGIErrorCode> ExecuteCGI(const HTTPRequest& http_request, Route route,
                                                std::string_view server_info, std::string_view client_info)
  {
    // TODO: ROOOOOOOT and MAPPING
    auto cgi_route = SetupCGIRoute(http_request.GetPath(), route.root, "/cgi-bin");
    if (!cgi_route.HasValue())
      return std::move(cgi_route.GetError());

    // TODO: Find resource location block here or before ExecuteCGI is called

    auto process = Fork(CGIRequest{http_request, cgi_route.GetValue(), server_info, client_info});
    if (!process.HasValue())
      return std::move(process.GetError());
    if (process->IsParent())
      return std::move(process.GetValue());

    auto std_io = RedirectStandardIO(process->GetSocket());
    if (std_io.first.GetFD() < 0 || std_io.second.GetFD() < 0)
      return CGIErrorCode::kRedirectionError;

    const std::vector<char*> argv{ConvertToCstrVector(process->GetRequest().GetArgv())};
    const std::vector<char*> envp{ConvertToCstrVector(process->GetRequest().GetEnvp())};
    const char* path{*argv.data()};

    if (setpgid(0, 0) == 0)  // Create own process group for cgi script
    {
      execve(path, argv.data(), envp.data());
    }
    Logger::Log(LogLevel::ERROR, "execve({}) failed: {}", path, errno);
    return CGIErrorCode::kScriptExecveError;
  }
}  // namespace cgi
