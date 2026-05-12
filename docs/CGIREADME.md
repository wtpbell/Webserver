# CGI

CGI (Common Gateway Interface) is an external script (or program) that is run and managed by the HTTP server that takes over the HTTP request and generates a response message for the client.
The server will serialize this response back into an HTTP response message and send it to the client in the appropriate format of the protocol standard. The main use case for CGI is to serve dynamic web pages (pages constructed at run-time).

## Types of responses

There are two different types of responses that a CGI script can produce: Non-Parsed Header(NPH) Responses (not supported) and CGI Responses.

CGI Responses can be further subdivided into four categories:
 - Document Response
 - Local Redirect Response
 - Client Redirect Response
 - Client Redirect Response with Document

Every CGI Response follows the same pattern: `generic-response = 1*header-field NL [ response-body ]`. It will have at least one or more CGI headers, `Content-Type`, `Status`, and/or `Location`, and zero or more generic headers, which need to be followed up by an empty new line and can have an optional body afterwards.

### Document Response

A Document Response pattern is as follows: `document-response = Content-Type [ Status ] *other-field NL response-body`. This type of response is the closest to an HTTP response message and will return a dynamically served page.

### Local Redirect Response

A Local Redirect Response pattern is as follows: `local-redir-response = local-Location NL`. This type of response will be handled by the server internally as if the server received a new HTTP Request from the same client. The response can either spawn a new response to further handle the new request or produce an HTTP response that can be sent back to the client.

### Client Redirect Response

A Client Redirect Response pattern is as follows: `client-redir-response = client-Location *extension-field NL`. This type of response generates an HTTP redirect response with HTTP `302` code.

### Client Redirect Response with Document

A Client Redirect Response with Document is as follows: `client-redirdoc-response = client-Location Status Content-Type *other-field NL response-body`. This response is similar to Client Redirect Response, with the only difference being the response body included in the HTTP redirect response.

## CGI Pipeline

Once the HTTP request, received from the client, is fully parsed and validated, and has an associated route view, it will be passed on to the router and dispatched either an HTTP Response or a CGI Process.

The conditions for spawning a CGI Process are that the route view either has a CGI extension that matches the target file extension or the location block is considered a CGI block. A CGI block means that everything within that directory root will be considered a CGI script that can be executed to generate a CGI Response. If none of these conditions match, the default will be an HTTP response.

The `cgi_extension` directive takes priority over the `cgi` directive. With the CGI extension, the target file will be executed by the program assigned within the `cgi_extension` directive. If no CGI-extension matches, it will check if the directory root is considered a `cgi-bin` directory. With both CGI-extension and `cgi-bin`, a CGI request object will be constructed, which stores the executable, program arguments, and environment variables needed to run the CGI script in a system-defined manner described within the CGI RFC. The CGI request will also, if the HTTP request has a body attached to the message, copy the body into the CGI request object, which will be passed to the CGI program through the socket pair established between the server and CGI program.

With the CGI request constructed, the CGI dispatcher can now spawn and return a CGI process. The returned CGI Process will be registered within the Connection registry, a waiting pending response will be enqueued, and the life-cycle of the CGI process will be managed by the server.

## CGI Life-cycle

The CGI process life cycle is dependent on the state it is in:
 - `kRunning`: `Send` in the request body to the CGI program and `Recv` from the CGI program its response.
 - `kWaitPID`: CGI program is finished sending the response and is awaiting clean up.
 - `kComplete`: CGI program is fully cleaned up and can be removed from the registry.
 - `kError`: An error occurred while running the CGI program and can be forcibly stopped and removed from the registry.

To prevent the CGI program from running indefinitely, the life cycle is limited to a maximum duration determined by the timeout handler.

The CGI program will send an EOF to indicate the end of the response message to the server and that the program is ready to be cleaned up. The CGI Parser parses this response message into a CGI Response object that is passed to the pending queue response that was created for the CGI, and serialized into an HTTP response message that can be passed back to the client. Afterwards, the server will clean up the CGI program.

If during the life-cycle of the CGI program an error occurs (CGI program crash, socket failure, parsing failure), a `504` HTTP response will be generated, and if a timeout of the program occurs, a `503` HTTP response will be generated.

## CGI Code Overview

- CGI: The main CGI dispatcher interface that either runs as an executable CGI program for the target based on the file extension or by directly executing the CGI program located within what is considered the designated cgi-bin directory.
- CGI Error: Enumeration of the possible CGI error codes that return when the CGI program is unable to be instantiated.
- CGI Parser: A stateless static class that parses the output of the CGI program based on the CGI RFC rules and constructs a CGI Response object.
- CGI Process: An object that manages the running CGI program and provides the interfaces for sending the optional CGI request body and receiving the CGI program output as a raw response. It has a socket pair pipeline between the main process and the CGI Program, where the stdin and stdout of the child process have been redirected to the newly created socket pair.
- CGI Request: Object that stores the executable, program arguments, environment variables, and optional body message based on the HTTP Request object used by the CGIProcess to spawn the CGI program.
- CGI Response: Object that stores the CGI program response and can be serialized to an HTTP response message.

# Resources

* [Cpp Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
* [The Common Gateway Interface (CGI) Version 1.1](https://datatracker.ietf.org/doc/html/rfc3875)
* [Common Gateway Interface - Wikipedia](https://en.wikipedia.org/wiki/Common_Gateway_Interface)
* [Simple CGI Web Server - Felix John COLIBRI](http://www.felix-colibri.com/papers/web/simple_cgi_web_server/simple_cgi_web_server.html)
* [Perl and CGI](https://www.perl.com/article/perl-and-cgi/)
* [timerfd_create(2) — Linux manual page](https://man7.org/linux/man-pages/man2/timerfd_create.2.html)
* [execve(2) - Linux man page](https://linux.die.net/man/2/execve)
* [fork(2) - Linux man page](https://linux.die.net/man/2/fork)
* [socketpair(2) - Linux man page](https://linux.die.net/man/2/socketpair)
* [setpgid - Linux man page](https://linux.die.net/man/3/setpgid)
* [socket - Linux man page](https://linux.die.net/man/2/socket)
* [Filesystem library - cppreference](https://en.cppreference.com/cpp/filesystem)
* [std::string_view - cppreference](https://en.cppreference.com/cpp/header/string_view)
* [std::basic_string - cppreference](https://en.cppreference.com/cpp/string/basic_string)
* [std::expected - cppreference](https://en.cppreference.com/cpp/utility/expected)
* [std::variant - cppreference](https://en.cppreference.com/cpp/utility/variant)
* [std::optional - cppreference](https://en.cppreference.com/cpp/header/optional)
* [std::vector - cppreference](https://en.cppreference.com/cpp/header/vector)
* [std::array - cppreference](https://en.cppreference.com/cpp/header/array)
* [std::unordered_map - cppreference](https://en.cppreference.com/cpp/container/unordered_map)
