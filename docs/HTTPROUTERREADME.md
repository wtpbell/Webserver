## HTTP and Router

The HTTP part of `webserv` is responsible for turning raw bytes from a client socket into an HTTP request, selecting the correct server/location block, executing the correct handler, and finally producing a valid HTTP response.

The flow is:

```text
Client socket
    ↓
Connection
    ↓
HTTPParser
    ↓
HTTPRequest
    ↓
ServerRegistry route lookup
    ↓
Router
    ↓
Return rule / CGI / GET / POST / DELETE handler
    ↓
HTTPResponse
    ↓
Serialized response sent back to client
```

---

### HTTP Request Parsing

Incoming data is read by the connection and passed into `HTTPParser`.

The parser works as a state machine. It parses the request in this order:

```text
Start line
    ↓
Headers
    ↓
Body decision
    ↓
Body / Chunked body
    ↓
Complete request
```

The parser supports:

- request line parsing, for example `GET /index.html HTTP/1.1`
- header parsing
- `Content-Length`
- `Transfer-Encoding: chunked`
- request body storage
- validation errors such as bad request syntax or unsupported methods

After the start line and headers are parsed, the request is validated before it is routed. The validation checks include:

- HTTP version
- supported method
- valid request target
- required `Host` header
- valid header names and values
- valid cookie format
- valid `Content-Length`
- supported transfer encoding

If parsing or validation fails, the server returns the corresponding HTTP error response.

---

### HTTPRequest

`HTTPRequest` stores the parsed request data in a structured form.

Important fields include:

- method, such as `GET`, `POST`, or `DELETE`
- original target, such as `/upload/file.txt?x=1`
- normalized path, such as `/upload/file.txt`
- query string
- headers
- cookies
- body
- optional temporary body file path

When the target is set, the path is URL-decoded and normalized. This prevents unsafe paths such as traversal attempts from being used directly by the router.

Example:

```text
Raw target: /static/../index.html
Normalized path: /index.html
```
---

### Body Storage

Request bodies are written through a small `IBodySink` interface.

This allows the HTTP parser to write body data without knowing where the data is stored.

There are two body sink implementations:

- `MemorySink`
  - stores small request bodies directly inside `HTTPRequest`
  - used when the body is small enough to safely keep in memory

- `FileSink`
  - stores request bodies in a temporary file
  - used for large request bodies or chunked uploads
  - avoids keeping large uploads fully in memory

The parser writes each received body chunk through the same interface:

```cpp
bodySink->Write(chunk);
```

This keeps parsing independent from storage details. The parser only handles HTTP syntax, while the sink decides whether the body is appended to the request object or written to disk.

One important note: `BodySink` itself does **not** check `client_max_body_size`. That limit should still be checked by the parser/body-reading logic before or while writing into the sink.

---

### Session and Cookies

The HTTP layer also handles simple session cookies.

When a request contains a valid `session_id` cookie, the session is reused and refreshed.

If the request has no valid session, the server creates a new session id and adds a `Set-Cookie` header to the response:

```http
Set-Cookie: session_id=<id>; Path=/; HttpOnly; SameSite=Lax
```

This allows the server to recognize the same client across multiple requests.

---

## Router

The router is responsible for deciding what should happen after a valid HTTP request has been created.

The high-level router flow is:

```text
HTTPRequest
    ↓
Find matching server and route
    ↓
Router::Dispatch
    ↓
Check return rule
    ↓
Try CGI dispatch
    ↓
Dispatch by HTTP method
    ↓
Apply custom error page if needed
    ↓
HTTPResponse or CGIProcess
```

---

### Server and Route Selection

Before the router handles the request, `ServerRegistry` selects the correct `RouteView`.

The selection is based on:

1. the listening IP and port
2. the `Host` header
3. the request path
4. the longest matching location prefix

For example:

```nginx
server {
    root ./www;

    location /images {
        root ./data/images;
    }
}
```

Request:

```http
GET /images/logo.png HTTP/1.1
Host: localhost
```

The path starts with `/images`, so the `/images` location is selected.

Request:

```http
GET /index.html HTTP/1.1
Host: localhost
```

There is no more specific location match, so the server-level root route is used.

### Return Rules

If the selected route contains a `return` rule, the router handles it before normal method dispatch.

Examples:

```nginx
return 301 /new-path;
return 404;
return 204;
```

The router creates the correct response directly:

- `3xx` return rules create redirect responses with a `Location` header
- `4xx` return rules create error responses
- other return rules create empty responses with the configured status code

---

### CGI Dispatch

After return rules, the router tries to dispatch the request to CGI.

If the request matches a CGI rule, the router creates a `CGIProcess` instead of a normal `HTTPResponse`.

This allows CGI execution to be handled asynchronously by the connection registry and event loop.

If CGI dispatch fails with a CGI error, the error is converted into an HTTP status code. If the route has a matching custom error page, the router applies it.

---

### Method Dispatch

If the request is not handled by a return rule or CGI, the router dispatches it by HTTP method.

Supported methods are:

```text
GET
POST
DELETE
```

Before calling the method handler, the route checks whether the method is allowed.

If the method is not allowed, the server returns:

```http
405 Method Not Allowed
Allow: GET, POST, DELETE
```

depending on the methods configured for that route.

---

## Path Resolution

The request path must be mapped safely to a filesystem path.

This is done by `ResolvePath`.

There are two mapping modes: `root` and `alias`.

### Root

With `root`, the full normalized request path is appended to the route root.

Example:

```nginx
location /static {
    root /srv/www;
}
```

Request:

```text
/static/logo.png
```

Filesystem path:

```text
/srv/www/static/logo.png
```

---

### Alias

With `alias`, the matched location prefix is replaced by the alias path.

Example:

```nginx
location /static {
    alias /srv/assets;
}
```

Request:

```text
/static/logo.png
```

Route tail:

```text
/logo.png
```

Filesystem path:

```text
/srv/assets/logo.png
```

---

### Security Check

After the path is resolved, the server checks that the final filesystem path is still inside the configured root or alias base.

This prevents directory traversal or symlink escape attacks.

Example rejected path:

```text
/static/../../etc/passwd
```

If path resolution fails, the server returns:

```http
403 Forbidden
```

---

## GET Handler

The `GET` handler serves files and directories.

The flow is:

```text
Resolve filesystem path
    ↓
Inspect path
    ↓
Path missing?
    → 404 Not Found

Path is directory?
    ↓
Missing trailing slash?
    → 301 Moved Permanently

Directory has index file?
    → Serve index file

Autoindex enabled?
    → Generate directory listing

Otherwise
    → 403 Forbidden

Path is regular readable file?
    → Serve file

Otherwise
    → 403 Forbidden
```

For files, the response includes the correct content type based on the file extension.

For static files, the server also adds a `Last-Modified` header when possible.

---

### Directory Handling

If the request targets a directory without a trailing slash, the server redirects to the slash version.

Example:

```http
GET /docs HTTP/1.1
```

Response:

```http
301 Moved Permanently
Location: /docs/
```

This is important because relative links inside HTML pages depend on the trailing slash.

If the directory contains the configured index file, such as `index.html`, the server serves it.

If there is no index file and `autoindex` is enabled, the server generates a simple HTML directory listing.

If `autoindex` is disabled, the server returns `403 Forbidden`.

---

## POST Handler

The `POST` handler writes the request body to a file under the resolved route path.

The flow is:

```text
Check body size
    ↓
Reject POST /
    ↓
Resolve filesystem path
    ↓
Validate target path
    ↓
Write body or move temporary body file
    ↓
Return 201 Created or 204 No Content
```

If the target file does not exist, a successful upload returns:

```http
201 Created
```

If the target file already exists and is overwritten, a successful upload returns:

```http
204 No Content
```

The handler checks:

- whether the target is a directory
- whether the parent directory exists
- whether the parent directory is writable and executable
- whether an existing target file is writable
- whether the request body exceeds `client_max_body_size`

If the body is too large, the server returns:

```http
413 Payload Too Large
```

---

## DELETE Handler

The `DELETE` handler removes a file from the filesystem.

The flow is:

```text
Resolve filesystem path
    ↓
Inspect path
    ↓
Missing path?
    → 404 Not Found

Path is directory?
    → 403 Forbidden

Path is not writable?
    → 403 Forbidden

Remove file
    ↓
Return 204 No Content
```

The server only deletes valid filesystem targets inside the configured route root or alias base.

Directory deletion is not supported.

---

## Custom Error Pages

Routes may define custom error pages.

When a response has an error status code and the route has a matching `error_page` rule, the router tries to apply it.

Example:

```nginx
error_page 404 /errors/404.html;
```

If a request produces `404 Not Found`, the router internally creates a new `GET` request for `/errors/404.html`.

If that internal request succeeds, the body of the custom page is returned while keeping the original error status.

Example:

```http
HTTP/1.1 404 Not Found
Content-Type: text/html
```

The body comes from:

```text
/errors/404.html
```

If the custom error page cannot be served, the original error response is kept.

External error page targets are treated as redirects.

Example:

```nginx
error_page 404 https://example.com/not-found;
```

This produces a redirect response.

---

## HTTP Response Creation

Responses are built through `ResponseFactory`.

The main response helpers are:

```text
MakeText
MakeHTML
MakeFile
MakeEmpty
MakeError
MakeRedirect
```

This keeps response creation consistent across the router and request handlers.

Examples:

```cpp
Response::MakeError(Status::kNotFound);
Response::MakeFile(Status::kOk, path, body);
Response::MakeRedirect(Status::kMovedPermanently, "/dir/");
```

Each response stores:

- status code
- reason phrase
- headers
- body

Before being sent to the client, the response is serialized into a valid HTTP response message.

---

## HTTP Error Codes

The server returns HTTP status codes to describe the result of each request.  
Successful responses use `2xx` codes, redirects use `3xx` codes, client errors use `4xx` codes, and server errors use `5xx` codes.

| Status Code | Name | Meaning |
| ----------- | ---- | ------- |
| `200` | OK | The request was successful. For example, a file was found and returned. |
| `201` | Created | The request created a new resource successfully, usually after a `POST` upload. |
| `204` | No Content | The request was successful, but there is no response body. This can be used after a successful `DELETE`. |
| `301` | Moved Permanently | The requested resource has been permanently redirected to another URL. For example, a directory request without a trailing `/` may redirect to the same path with `/`. |
| `302` | Found | The requested resource is temporarily redirected to another URL. |
| `400` | Bad Request | The request is invalid. For example, the request line, headers, or body format cannot be parsed correctly. |
| `403` | Forbidden | The server understood the request, but access is not allowed. For example, the target path is outside the configured root, permissions are denied, or directory listing is disabled. |
| `404` | Not Found | The requested file, directory, route, or resource does not exist. |
| `405` | Method Not Allowed | The HTTP method is not allowed for the matched route. For example, sending `POST` to a location that only allows `GET`. |
| `413` | Payload Too Large | The request body is larger than the configured `client_max_body_size`. |
| `414` | URI Too Long | The request target or URI is longer than the server accepts. |
| `500` | Internal Server Error | An unexpected error happened inside the server. |
| `501` | Not Implemented | The server does not support the requested HTTP method or feature. |

### Common Examples

| Situation | Possible Response |
| --------- | ----------------- |
| Requesting an existing HTML file | `200 OK` |
| Uploading a file successfully | `201 Created` |
| Deleting a file successfully | `204 No Content` |
| Requesting a directory without trailing slash | `301 Moved Permanently` |
| Invalid HTTP request syntax | `400 Bad Request` |
| Trying to access a file outside the web root | `403 Forbidden` |
| Requesting a missing file | `404 Not Found` |
| Using `POST` where only `GET` is allowed | `405 Method Not Allowed` |
| Uploading a file larger than the limit | `413 Payload Too Large` |
| Sending a request target that is too long | `414 URI Too Long` |
| Unsupported method or feature | `501 Not Implemented` |
```:


## Example Request Flow

Example request:

```http
GET /images/logo.png HTTP/1.1
Host: localhost
```

Flow:

```text
1. Connection reads bytes from socket
2. HTTPParser parses request line and headers
3. HTTPRequest stores method, target, path, headers and cookies
4. ServerRegistry selects the matching server and route
5. Router checks return rule
6. Router checks CGI
7. Router dispatches to GET handler
8. GET handler resolves filesystem path
9. File is read from disk
10. HTTPResponse is created
11. Connection sends serialized response to client
```

Result:

```http
HTTP/1.1 200 OK
Content-Type: image/png
Content-Length: ...
Last-Modified: ...
```

---

## Design Notes

The HTTP and router layers are separated to keep the code easier to reason about.

The HTTP parser only understands HTTP syntax and validation.

The router decides which rule applies to the request.

The request handlers deal with filesystem behavior for `GET`, `POST`, and `DELETE`.

The response factory centralizes response creation so status codes, headers, redirects, errors, and file responses are generated consistently.

Path normalization and base-directory checks are done before filesystem access. This is important because the server must never allow a request path to escape the configured web root.

---

## Resources

### HTTP and Web Server References

- [RFC 9110 - HTTP Semantics](https://www.rfc-editor.org/rfc/rfc9110)
- [RFC 9112 - HTTP/1.1](https://www.rfc-editor.org/rfc/rfc9112)
- [MDN - HTTP](https://developer.mozilla.org/en-US/docs/Web/HTTP)
- [MDN - HTTP response status codes](https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Status)
- [MDN - HTTP headers](https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Headers)
- [MDN - HTTP cookies](https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/Cookies)
- [MDN - HTTP redirection](https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/Redirections)
- [nginx documentation](https://nginx.org/en/docs/)
- [nginx root directive](https://nginx.org/en/docs/http/ngx_http_core_module.html#root)
- [nginx alias directive](https://nginx.org/en/docs/http/ngx_http_core_module.html#alias)

### C++ Resources

- [std::unique_ptr](https://en.cppreference.com/cpp/memory/unique_ptr)
- [Filesystem library](https://en.cppreference.com/cpp/filesystem)
- [std::unique_ptr](https://en.cppreference.com/cpp/memory/unique_ptr)
- [std::optional](https://en.cppreference.com/cpp/utility/optional)
- [std::unreachable](https://en.cppreference.com/cpp/utility/unreachable)
- [_builtin_unreachable](https://runebook.dev/en/docs/c/program/unreachable)
- [_builtin_unreachable](https://runebook.dev/en/docs/c/program/unreachable)

---

