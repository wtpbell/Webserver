/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   RequestHandler.cpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: bewong <bewong@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/02/13 10:05:04 by bewong        #+#    #+#                 */
/*   Updated: 2026/02/13 10:05:04 by bewong        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "router/RequestHandler.hpp"

#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>

#include "Logger.hpp"
#include "config/RouteView.hpp"
#include "http/ResponseFactory.hpp"

/*************************************************** Helpers **********************************************************/
namespace request_handler
{
  namespace Response = HTTP::response;
  using Method = RouteView::MethodMask;
  using Status = HTTP::Status;
  using FileStatusResult = request_handler::FileStatusResult;
  using PathInfo = request_handler::PathInfo;

  bool HasAnyPerm(const fs::file_status& status, fs::perms owner, fs::perms group, fs::perms others)
  {
    const fs::perms permissions = status.permissions();
    return (permissions & (owner | group | others)) != fs::perms::none;
  }

  bool IsPermDenied(const std::error_code& ec)
  {
    return ec == std::errc::permission_denied || ec == std::errc::operation_not_permitted;
  }

  Status MapStatError(const std::error_code& ec)
  {
    return IsPermDenied(ec) ? Status::kForbidden : Status::kInternalServerError;
  }

  std::string_view ComputeRouteTail(std::string_view path, std::string_view locationPrefix)
  {
    if (locationPrefix.size() > path.size())
      return {};
    std::string_view tail = path.substr(locationPrefix.size());
    return tail.empty() ? std::string_view("/") : tail;
  }

  Method MethodToMask(HTTP::Method m)
  {
    switch (m)
    {
      case HTTP::Method::kGet:
        return Method::kGet;
      case HTTP::Method::kPost:
        return Method::kPost;
      case HTTP::Method::kDelete:
        return Method::kDelete;
      default:
        return Method::kNone;
    }
  }

  // https://en.cppreference.com/w/cpp/io/basic_istream/seekg -> moves the read position (get pointer) inside the input
  // stream.
  // https://en.cppreference.com/w/cpp/io/basic_istream/tellg -> returns the current position of the read pointer.
  // avoid whitespace issues of >> and single allocation

  FileStatusResult ReadFile(const fs::path& path, std::string& out)
  {
    out.clear();
    std::error_code ec;

    fs::file_status status = fs::status(path, ec);
    if (ec)
      return IsPermDenied(ec) ? FileStatusResult::kPermissionDenied : FileStatusResult::kIOError;

    if (!HasAnyPerm(status, fs::perms::owner_read, fs::perms::group_read, fs::perms::others_read))
      return FileStatusResult::kPermissionDenied;

    std::uintmax_t file_size = fs::file_size(path, ec);
    if (ec)
      return IsPermDenied(ec) ? FileStatusResult::kPermissionDenied : FileStatusResult::kIOError;

    if (file_size > static_cast<std::uintmax_t>(std::numeric_limits<std::size_t>::max()))
      return FileStatusResult::kIOError;

    std::ifstream input(path, std::ios::binary);
    if (!input)
      return FileStatusResult::kIOError;

    std::size_t size = static_cast<std::size_t>(file_size);
    out.resize(size);
    if (size > 0 && !input.read(&out[0], static_cast<std::streamsize>(size)))
      return FileStatusResult::kIOError;

    return FileStatusResult::kOk;
  }

  FileStatusResult WriteFile(const fs::path& path, std::string_view data)
  {
    std::error_code ec;
    fs::path parent = path.parent_path();

    if (!parent.empty())
    {
      fs::file_status parentStatus = fs::status(parent, ec);
      if (ec)
        return IsPermDenied(ec) ? FileStatusResult::kPermissionDenied : FileStatusResult::kIOError;

      if (!HasAnyPerm(parentStatus, fs::perms::owner_write, fs::perms::group_write, fs::perms::others_write) ||
          !HasAnyPerm(parentStatus, fs::perms::owner_exec, fs::perms::group_exec, fs::perms::others_exec))
        return FileStatusResult::kPermissionDenied;
    }

    const bool exists = fs::exists(path, ec);
    if (ec)
      return IsPermDenied(ec) ? FileStatusResult::kPermissionDenied : FileStatusResult::kIOError;

    if (exists)
    {
      fs::file_status fileStatus = fs::status(path, ec);
      if (ec)
        return IsPermDenied(ec) ? FileStatusResult::kPermissionDenied : FileStatusResult::kIOError;

      if (!HasAnyPerm(fileStatus, fs::perms::owner_write, fs::perms::group_write, fs::perms::others_write))
        return FileStatusResult::kPermissionDenied;
    }

    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output)
      return FileStatusResult::kIOError;

    output.write(data.data(), static_cast<std::streamsize>(data.size()));
    if (!output)
      return FileStatusResult::kIOError;

    return FileStatusResult::kOk;
  }

  /*
    https://man7.org/linux/man-pages/man2/rename.2.html
    https://lwn.net/Articles/457667/
    https://queue.acm.org/detail.cfm?id=2801719
  */
  FileStatusResult MoveTempIntoPlace(const fs::path& tmp, const fs::path& target, bool /*targetExisted*/)
  {
    std::error_code ec;
    fs::rename(tmp, target, ec);
    if (!ec)
      return FileStatusResult::kOk;

    std::error_code ec2;
    fs::remove(tmp, ec2);

    return IsPermDenied(ec) ? FileStatusResult::kPermissionDenied : FileStatusResult::kIOError;
  }

  /*
  GET /images/
    ↓
    Resolve path → is directory?
    ↓ YES
    TryServeIndex
        ↓
        Exists? → serve file
        Missing? → nullopt
    ↓
    MakeAutoIndex
        ↓
        autoindex ON? → generate HTML
        autoindex OFF? → 403
  */
  FileStatusResult FinalizeBodyToPath(const HTTPRequest& request, const fs::path& target, bool existed)
  {
    if (const std::optional<fs::path>& tmpOpt = request.GetBodyFilePath(); tmpOpt)
      return MoveTempIntoPlace(*tmpOpt, target, existed);

    return WriteFile(target, request.GetBody());
  }

  PathInfo InspectPath(const fs::path& path)
  {
    PathInfo info;

    std::error_code ec;
    fs::file_status file_status = fs::status(path, ec);

    if (file_status.type() == fs::file_type::not_found)
      return info;

    if (ec)
    {
      info.ec = ec;
      info.statError = true;
      return info;
    }

    info.exists = fs::exists(file_status);
    info.isDir = fs::is_directory(file_status);
    info.isFile = fs::is_regular_file(file_status);
    return info;
  }

  std::optional<HTTPResponse> ValidatePostTarget(const fs::path& path, const PathInfo& info)
  {
    if (info.statError)
      return Response::MakeError(MapStatError(info.ec));
    if (info.exists && info.isDir)
      return Response::MakeError(Status::kBadRequest);
    if (info.exists && !info.isFile && !info.isDir)
      return Response::MakeError(Status::kForbidden);

    const fs::path parent = path.parent_path();
    if (!parent.empty())
    {
      const PathInfo parentInfo = InspectPath(parent);
      if (parentInfo.statError)
        return Response::MakeError(MapStatError(parentInfo.ec));
      if (!parentInfo.exists || !parentInfo.isDir)
        return Response::MakeError(Status::kNotFound);
    }

    return std::nullopt;
  }

  std::optional<HTTPResponse> FileStatusToHTTP(FileStatusResult r)
  {
    switch (r)
    {
      case FileStatusResult::kOk:
        return std::nullopt;
      case FileStatusResult::kPermissionDenied:
        return Response::MakeError(Status::kForbidden);
      case FileStatusResult::kIOError:
      default:
        return Response::MakeError(Status::kInternalServerError);
    }
  }

  void CleanupTempOnFailure(const HTTPRequest& request)
  {
    if (const auto& tmp = request.GetBodyFilePath(); tmp)
    {
      std::error_code ec;
      fs::remove(*tmp, ec);
    }
  }

  std::string BuildAllowHeader(RouteView::MethodMask mask)
  {
    std::string out;
    auto add = [&](const char* s)
    {
      if (!out.empty())
        out += ", ";
      out += s;
    };

    if (Has(mask, Method::kGet))
      add("GET");
    if (Has(mask, Method::kPost))
      add("POST");
    if (Has(mask, Method::kDelete))
      add("DELETE");
    return out;
  }

  // Directory request -> check index; index missing + autoindex enabled -> listing; index missing + autoindex disabled
  // -> 403
  std::optional<HTTPResponse> TryServeIndex(const RouteView& route, const fs::path& dirFsPath)
  {
    fs::path idx = dirFsPath / route.index;
    PathInfo idxInfo = InspectPath(idx);

    if (idxInfo.statError)
      return Response::MakeError(MapStatError(idxInfo.ec));
    if (!idxInfo.exists || !idxInfo.isFile)
      return std::nullopt;

    std::string body;
    switch (ReadFile(idx, body))
    {
      case FileStatusResult::kOk:
        return Response::MakeFile(Status::kOk, idx.string(), std::move(body));
      case FileStatusResult::kPermissionDenied:
        return Response::MakeError(Status::kForbidden);
      default:
        return Response::MakeError(Status::kInternalServerError);
    }
  }

  /*
  iterate over directory and make a listing --> build HTML --> return response
  e.g. dirFsPath = "/var/www/html/files/"; urlPath = "/files/"

  <!doctype html>
    <title>Index of /files/</title>
    <h1>Index of /files/</h1>
    <ul>
    <li><a href='../'>../</a></li>  //                      --> /files/ is not '/'
    <li><a href='/files/report.pdf'>report.pdf</a></li>  //  /files/report.pdf
    <li><a href='/files/image.png'>image.png</a></li>    //  /files/image.png
    <li><a href='/files/docs/'>docs/</a></li>            //  /files/docs/
  </ul></body></html>
  */
  HTTPResponse MakeAutoIndex(const RouteView& route, const fs::path& dirFsPath, std::string_view urlPath)
  {
    if (!route.autoindex)
      return Response::MakeError(Status::kForbidden);

    std::error_code ec;
    std::ostringstream html;

    html << "<!doctype html><html><head><meta charset='utf-8'>"
         << "<title>Index of " << urlPath << "</title></head><body>"
         << "<h1>Index of " << urlPath << "</h1><ul>";

    if (urlPath != "/")
      html << "<li><a href='../'>../</a></li>";

    fs::directory_iterator it(dirFsPath, ec);
    if (ec)
      return Response::MakeError(IsPermDenied(ec) ? Status::kForbidden : Status::kInternalServerError);

    for (; it != fs::directory_iterator(); it.increment(ec))
    {
      if (ec)
        break;
      const std::string name = it->path().filename().string();
      bool isDir = it->is_directory(ec);
      if (ec)
      {
        ec.clear();
        isDir = false;
      }
      std::string href(urlPath);
      if (!href.empty() && href.back() != '/')  // If directory URL doesn't end with /, browser treats it like file.
        href.push_back('/');
      href += name;
      if (isDir)
        href.push_back('/');
      html << "<li><a href='" << href << "'>" << name << (isDir ? "/" : "") << "</a></li>";
    }
    if (ec)
      return Response::MakeError(IsPermDenied(ec) ? Status::kForbidden : Status::kInternalServerError);
    html << "</ul></body></html>";
    return Response::MakeHTML(Status::kOk, html.str());
  }

  HTTPResponse HandleDirectoryGet(const RouteView& route, const fs::path& dirFsPath, std::string_view urlPath)
  {
    if (std::optional<HTTPResponse> idx = TryServeIndex(route, dirFsPath))
      return std::move(*idx);

    return MakeAutoIndex(route, dirFsPath, urlPath);
  }

  /************************************************** Dispatch
   * **********************************************************/

  HTTPResponse HandleMethods(const HTTPRequest& request, const RouteView& route, std::string_view remainder)
  {
    const Method method = MethodToMask(request.GetMethod());

    if (method == Method::kNone)
      return Response::MakeError(Status::kNotImplemented);

    if (!Has(route.allowedMask, method))
    {
      HTTPResponse res = Response::MakeError(Status::kMethodNotAllowed);
      res.SetHeader("Allow", BuildAllowHeader(route.allowedMask));
      return res;
    }

    const std::string_view urlPath = request.GetPath();
    if ((method == Method::kPost || method == Method::kDelete) && urlPath.size() > 1 && urlPath.back() == '/')
      return Response::MakeError(Status::kBadRequest);

    if (method == Method::kGet)
      return request_handler::HandleGet(request, route, remainder);
    if (method == Method::kPost)
      return request_handler::HandlePost(request, route, remainder);
    if (method == Method::kDelete)
      return request_handler::HandleDelete(request, route, remainder);

    return Response::MakeError(Status::kInternalServerError);
  }

  /************************************************ Path helper
   * *********************************************************/

  /*
  There are two mapping modes:

  1. Root location "/"
     - use the full normalized request path
     - example:
         request path = /images/logo.png
         route.root   = /srv/www
         result       = /srv/www/images/logo.png

  2. Non-root location or alias
     - use only the matched-location remainder
     - example:
         locationPrefix = /static
         request path   = /static/logo.png
         remainder      = /logo.png
         route.root     = /srv/www/static
         result         = /srv/www/static/logo.png

     - alias behaves the same way: it maps the remainder under the alias base.
*/

  std::optional<fs::path> ResolvePath(const RouteView& route, std::string_view path, std::string_view remainder)
  {
    std::string_view tail = route.alias.has_value() ? remainder : path;

    while (!tail.empty() && tail.front() == '/')
      tail.remove_prefix(1);

    const fs::path base = route.alias ? fs::path(*route.alias) : fs::path(route.root);

    std::error_code ec;
    fs::path canonBase = fs::weakly_canonical(base, ec);
    if (ec)
      return std::nullopt;

    fs::path joined = fs::weakly_canonical(canonBase / fs::path(tail), ec);
    if (ec)
      return std::nullopt;

    if (!std::equal(canonBase.begin(), canonBase.end(), joined.begin()))
      return std::nullopt;

    return joined;
  }

  /**************************************************** GET
   * *************************************************************/

  // trailing slash is valid for directories, missing slash for directory gets 301
  HTTPResponse HandleGet(const HTTPRequest& request, const RouteView& route, std::string_view remainder)
  {
    const std::string_view urlPath = request.GetPath();

    std::optional<fs::path> resolved = ResolvePath(route, urlPath, remainder);
    if (!resolved)
      return Response::MakeError(Status::kForbidden);

    const fs::path& path = *resolved;
    PathInfo info = InspectPath(path);
    if (info.statError)
      return Response::MakeError(MapStatError(info.ec));

    if (!info.exists)
      return Response::MakeError(Status::kNotFound);

    // ---- directory ----
    if (info.isDir)
    {
      if (!urlPath.empty() && urlPath.back() != '/')
        return Response::MakeRedirect(Status::kMovedPermanently, std::string(urlPath) + "/");
      return HandleDirectoryGet(route, path, urlPath);
    }

    // ---- file ----
    if (!info.isFile)
      return Response::MakeError(Status::kForbidden);

    std::string body;
    FileStatusResult result = ReadFile(path, body);
    std::optional<HTTPResponse> errorResponse = FileStatusToHTTP(result);
    if (errorResponse)
      return *errorResponse;

    return Response::MakeFile(Status::kOk, path.string(), body);
  }

  /**************************************************** POST
   * ************************************************************/

  // accept the body bytes and write them to a file under the root without decoding the payload
  // test with : curl -i -X POST http://127.0.0.1:8080/b \ --data-binary @./www/a
  // for overwrite : printf "B" > ./www/a  curl - i -X POST http://127.0.0.1:8080/a --data-binary @./www/a

  HTTPResponse HandlePost(const HTTPRequest& request, const RouteView& route, std::string_view remainder)
  {
    const std::string_view urlPath = request.GetPath();

    if (!request.GetBodyFilePath() && request.GetBody().size() > route.clientMaxBody)
      return Response::MakeError(Status::kPayloadTooLarge);

    if (urlPath == "/")
      return Response::MakeError(Status::kBadRequest);

    std::optional<fs::path> resolved = ResolvePath(route, urlPath, remainder);
    if (!resolved)
      return Response::MakeError(Status::kForbidden);

    const fs::path& path = *resolved;
    const PathInfo info = InspectPath(path);

    std::optional<HTTPResponse> errorResponse = ValidatePostTarget(path, info);
    if (errorResponse)
      return *errorResponse;

    const bool existed = info.exists;

    const FileStatusResult result = FinalizeBodyToPath(request, path, existed);

    if (request.GetBodyFilePath())
      Logger::Log(LogLevel::INFO, "Finalizing from temp {}", request.GetBodyFilePath()->string());

    std::optional<HTTPResponse> response = FileStatusToHTTP(result);
    if (response)
    {
      CleanupTempOnFailure(request);
      return *response;
    }

    return Response::MakeEmpty(existed ? Status::kNoContent : Status::kCreated);
  }

  /*************************************************** DELETE
   * ***********************************************************/

  HTTPResponse HandleDelete(const HTTPRequest& request, const RouteView& route, std::string_view remainder)
  {
    std::optional<fs::path> resolved = ResolvePath(route, request.GetPath(), remainder);
    if (!resolved)
      return Response::MakeError(Status::kForbidden);

    const fs::path& path = *resolved;
    PathInfo info = InspectPath(path);

    if (info.statError)
      return Response::MakeError(MapStatError(info.ec));

    if (!info.exists)
      return Response::MakeError(Status::kNotFound);

    if (info.isDir || !info.isFile)
      return Response::MakeError(Status::kForbidden);

    std::error_code ec;
    const bool removed = fs::remove(path, ec);

    if (ec)
    {
      if (IsPermDenied(ec))
        return Response::MakeError(Status::kForbidden);
      return Response::MakeError(Status::kInternalServerError);
    }

    if (!removed)
      return Response::MakeError(Status::kNotFound);

    return Response::MakeEmpty(Status::kNoContent);
  }

  /*************************************************** CGI ***********************************************************/

  HTTPResponse HandleCgi(const HTTPRequest& request, const RouteView& route, std::string_view /*remainder*/)
  {
    if (request.GetBody().size() > route.clientMaxBody)
      return Response::MakeError(Status::kPayloadTooLarge);

    return Response::MakeError(Status::kNotImplemented);  // until CGI finished
  }
};  // namespace request_handler
