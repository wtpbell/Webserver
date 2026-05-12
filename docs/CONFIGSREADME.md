*This project has been created as part of the 42 curriculum by bewong, jboon, and jstuhrin.*

*The config loader was implemented by jstuhrin.*

# Description

## Config File
A config file is used to define the static properties of servers, it consists of block directives and directives and
their parameters. The grammar of this config file follows the grammar of nginx config files wherever possible.

### Block Directives

#### http
All `server` blocks defining http servers must be inside the `http` block. There must be exactly one `http` block.
```
  Syntax:  http { ... }
  Context: main
```

#### server
Defines the properties of at least one physical server. For each `listen` directive, a `server` block defines one
physical server. There must be at least one `server` block in the `http` block. A config file consisting of only one
`server` block inside the `http` block is the minimal valid config file.
```
  Syntax:  server { ... }
  Context: http
```
#### location
Defines a location inside a server. A `server` block without a `location` block has only a default location.
```
  Syntax:  location PATH { ... }
  Default: /
  Context: server
```
### Directives

#### listen
Defines the ip-port pair of a server. If there is more than one `listen` directive inside a `server` block, this block
defines several physical servers with different ip-port pairs and otherwise identical properties.
```
  Syntax:   listen address;
            listen address:port;
            listen port;
  Default:  listen [::]:8080;
  Context:  server
  Examples: listen 127.0.0.1:8000;
            listen 127.0.0.1;
            listen 8000;
            listen [::]:8000;
            listen [::1];
```
#### server_name
Defines the server names of a server. A server may have several names. Two servers with identical ip-port pairs
may not have the same name. 
```
  Syntax:   server_name name ...;
  Default:  server_name "";
  Context:  server
```
#### root
Maps a URL path to a filesystem location, the path parameter is prepended to the location prefix.
```
  Syntax:   root path;
  Default:  root ./www
  Context:  http, server, location
```
#### index
Defines the default file to serve when a directory is requested.
```
  Syntax:   index file ...;
  Default:  index index.html;
  Context:  http, server, location
```
#### alias
Maps a URL path to a filesystem location, the path parameter replaces the location prefix.
```
  Syntax:   alias path;
  Default:  —
  Context:  location
```
#### client_max_body_size
Defines the maximum size of a client request body.
```
  Syntax:   client_max_body_size size number k | m | g | K | M | G;
  Default:  client_max_body_size 1m;
  Context:  http, server, location
```
#### error_page
Defines custom responses for http error codes.
```
  Syntax:   error_page code ... uri;
  Default:  -
  Context:  http, server, location
```
#### return
Defines a code to be sent to the client and/or a URL the client is redirected to. No further processing takes place.
```
  Syntax: return code;
          return code URL;
          return URL;
  Default:  -
  Context:  server, location
```
#### allowed_methods
Defines which methods can be used at a the location.
```
  Syntax:  allowed_methods 1*3method;
  Default: GET
  Context: location
  Note:    each method must be unique
```
#### auto_index
Allows directory listing if no index file is present at the root of the directory.
```
  Syntax:   autoindex on | off;
  Default:  autoindex off;
  Context:  http, server, location
```
#### cgi
If set to `on`, everything located at the `root` of the location is treated as a cgi script.
```
  Syntax:   cgi on | off;
  Default:  cgi off;
  Context:  location
```
#### cgi_extension
Defines the cgi `type` to be run. The file located at `path` is interpreted as a cgi script.
```
  Syntax:   cgi_extension type path;
  Default:  -
  Context:  server, location
```
#### default_server
Defines which server will be the default server for the specified ip-port pair. If no server with a specific ip-port
pair is explicitly marked, the server with the ip-port pair defined in the config file will be the default
server.
```
  Syntax:  listen port | ip | ip:port default_server;
  Default: -
  Context: listen
```



## Configs Loader Pipeline
### Lexer-Parser-Validator
The lexer-parser-validator pipeline takes a config file as input and produces an AST as output.
This production process is resilient: the lexer-parser-validator pipeline will always produce the AST specified by the
config file and the config file grammar. It should be noted that the AST will be degenerate if the config file contains any errors.

The lexer consumes the config file and tokenizes and lexes it. The lexer is fine-grained and enforces all lexical rules
that define directives, block directives, and comments. The lexer only loggs an error when it encounters an illegal
character. The output of the lexer is a token list.

The parser consumes the token list and produces an AST. It enforces all syntactical rules that determine the relative
positions of block directives, directives, and parameters in the AST, while only enforcing minimal contextual validity.
The main rules enforced by the parser are these:

1. block directives may have block directives and directives as children (a location block must also have one parameter child),
2. directives may only have parameters as children,
3. parameters may not have any children,
4. the syntacical context opened by a block directive must be demarcated by left and right braces,
5. the syntactical context opened by a directive must be closed by a semicolon.

Any errors encountered by the parser are logged to be printed later.

The Validator validates the AST produced by the Parser. The validator enforces

1. the contextual validaty of block directives, directives, and parameters,
2. the presence of mandated block directives, directives, and parameters,
3. the lexical, syntactical, and semantic correctness of parameters.

It should be noted that the validation of directive parameters is not internally resilient. The validator stops
validating the parameter list of a given directive after encountering the first error in this parameter list. However,
the validator will continue validating other parts of the AST.
Any errors encountered by the validator are logged.

If the lexer-parser-validator pipeline encountered any errors, the config loader prints all errors to standard error
and the program exits.

If the lexer-parser-validator does not encounter eny errors, the config loader will start the builder.

### Builder
The builder consumes the AST. It produces a server registry and populates this registry with a server view for each
server and a route view for each route.

The builder enforces these rules:

1. no duplicate hostnames within a server block,
2. no duplicate ip-port within a server block,
3. no duplicate ip-port hostname combinations across all server blocks,
4. the default_server flag may only be used once for each ip-port pair across all server blocks.

If the builder does not encounter any errors, it produces a server registry, which can then be used to start the
servers.

### Server Registry
The server registry persists throughout the runtime of any servers. It allows servers to be started, and it
dispatches route views to servers which have received an http or CGI request. The server registry matches
route view requests to both ip-port pairs and host names, which makes name-based virtual hosting possible.

# Instructions

The config loader pipeline offers some tools that can help debug a config file:

1. flag --print-tokenlist will print a list of all tokens extracted from the config file,
2. flag --print-AST will print the AST,
3. flag --config-debug will print the config file with all error tokens highlighted in red.

Example:
```
./webserv configs/multi-server.conf --config-debug
```

# Resources
## Web Resources
- [Wikipedia: Recursive Descent Parser](https://en.wikipedia.org/wiki/Recursive_descent_parser#C_implementation)
- [RFC 5952 section 2](https://datatracker.ietf.org/doc/html/rfc5952#section-2)
- [NGINX: Create NGINX Configuration Files](https://docs.nginx.com/nginx/admin-guide/basic-functionality/managing-configuration-files/)
- [NGINX: ngx_http_core_module](https://nginx.org/en/docs/http/ngx_http_core_module.html)
- [Digital Ocean: NGINX Server and Location Blocks Selection Algorithms](https://www.digitalocean.com/community/tutorials/understanding-nginx-server-and-location-block-selection-algorithms)
- [C++ reference](https://www.cppreference.com)

## Books
- Josh Lospinoso, *C++ Crash Course: A Fast-Paced Introduction*, No Starch Press, 2019.
- Rich Yonts, *100 C++ Mistakes and How to Avoid Them*, Manning, 2025. 


## AI Usage
A variety of LLMs (Claude Opus, Claude Sonnett, Gemini, Grok, Mistral Large, etc.) was used for two main purposes:

1. discussing high-level concepts and design choices ("What are advantages and disadvantages of a fine-grained lexer?"),
2. researching and discussing modern C++ syntax.

Typically, the same question was posed to at least two models, their answers were compared, and follow-up questions asked. Since this
student believes that the best way to learn programming is to manually write code, AI was only used to answer general questions, and
all code included in the config loader pipeline was hand-written without use of auto-complete.
