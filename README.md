*This project has been created as part of the 42 curriculum by jgirbau-, iborge-g and amarquez*

# Webserv

## Description

`webserv` is a non-blocking HTTP/1.1 server written from scratch in C++98, built as
part of the 42 core curriculum. The goal of the project is to understand the HTTP
protocol at a low level by implementing a working web server without relying on any
existing server or HTTP library: sockets, request parsing, response building, static
file serving, and CGI execution are all handled manually, using a single `poll()` (or
equivalent) event loop for every read/write operation between the server and its
clients.

The server is configured through an NGINX-inspired configuration file, allowing
multiple virtual servers, multiple listening ports, per-route rules, and CGI execution,
while staying compatible with standard web browsers.

Key features:
- Fully non-blocking I/O, driven by a single `poll()` loop (no blocking `read`/`write`
  outside of readiness notifications).
- `GET`, `POST`, and `DELETE` methods.
- Configuration file supporting multiple servers, multiple `listen` interfaces/ports,
  and per-location rules (allowed methods, root, index, redirections, autoindex, upload
  location, CGI extensions).
- Static file serving, directory listing (autoindex), and custom/default error pages.
- File uploads from clients.
- CGI execution (e.g. Python), including support for chunked request bodies and
  streamed CGI output.
- Client request body size limits.

## Instructions

### Compilation

```bash
make        # builds the webserv binary
make clean  # removes object files
make fclean # removes object files and the binary
make re     # fclean + all
```

### Running the server

```bash
./webserv [configuration file]
```

### Configuration file

The configuration file follows an NGINX-inspired `server { ... }` / `location { ... }`
syntax. Each `server` block can define:
- one or more `listen` interface:port pairs,
- default error pages,
- a maximum client request body size,
- one or more `location` blocks, each of which can set allowed HTTP methods, a root
  directory, an index file, a redirection, directory listing (autoindex), an upload
  location, and CGI extensions mapped to interpreters.

A config file always needs a `listen` and a `root` directive at the server level.
Sample configuration files are provided under `conf/` and can be used as a starting
point or reference for testing every feature described above.

### Testing

```bash
make test
```

Runs the project's unit test suite (config parsing/validation, HTTP request/response
handling, CGI execution, sockets, etc.).

For manual/browser testing, start the server with a config file exposing a static
site and a CGI route, then:
```bash
curl -v http://localhost:<port>/
```
or open `http://localhost:<port>/` directly in a browser.

## Resources

- [Webserv: Building an HTTP Server from Scratch](https://www.alimnaqvi.com/blog/webserv) — walkthrough of the 42 webserv project.
- [NGINX Beginner's Guide](https://nginx.org/en/docs/beginners_guide.html) — official NGINX documentation, used as a reference for the configuration file syntax and behavior.
- [Understanding Nginx Server and Location Block Selection Algorithms](https://www.digitalocean.com/community/tutorials/understanding-nginx-server-and-location-block-selection-algorithms) — DigitalOcean tutorial on how NGINX matches requests to `server`/`location` blocks.
- [RFC 3986 — Uniform Resource Identifier (URI): Generic Syntax](https://www.hjp.at/doc/rfc/rfc3986.html)
- [RFC 9112 — HTTP/1.1](https://www.rfc-editor.org/info/rfc9112/)
- [RFC 3875 — The Common Gateway Interface (CGI) Version 1.1](https://www.rfc-editor.org/info/rfc3875/)


### AI usage

AI was used as a learning tool at various points of the development, as well as a test generator for the unit tests and as to help diagnose some bugs and edge cases we found during our builds.

-