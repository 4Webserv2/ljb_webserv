# Webserv

*This project has been created as part of the 42Rio curriculum by lraggio, btaveira and jbergfel.*

**Webserv** is a lightweight HTTP server written in **C++ (C++98)** and developed from scratch as part of the 42 curriculum. The goal of this project is to gain a deep understanding of how a real web server works internally, including socket management, request parsing, event-driven I/O, and graceful shutdown.

The server is inspired by **NGINX-style configuration files** and supports multiple virtual servers. It uses **non-blocking sockets with epoll** to efficiently handle multiple concurrent clients.

Beyond implementing HTTP features, this project places strong emphasis on **clean architecture**, **robust resource management**, and **well-defined responsibilities** between components.

---

## Features

- HTTP/1.0 compliant
- Multiple virtual servers
- Non-blocking I/O using `epoll`
- GET / POST / DELETE methods
- Static file serving
- Directory autoindex
- CGI execution
- File upload handling
- Custom error pages
- Graceful shutdown via UNIX signals
- Fully configurable via `.conf` file
- Valgrind clean (no memory leaks, no still reachable)

---

## Architecture and Design Choices (Core Design Patterns)

#### Singleton
Used to guarantee a single global instance for critical components:
- `RunTime`
- `EpollInstance`
- `Logger`

This ensures centralized lifecycle management of system resources.

#### Strategy
Applied in the logging system:
- `LogHandler` (interface)
- `StdLogHandler`
- `FileLogHandler`

Allows multiple logging strategies without modifying the core logger logic.

#### Composite
Used by `CompositeLogHandler` to broadcast log events to multiple handlers transparently.

#### Dispatcher / Router
Incoming requests are dispatched through a central mechanism that:
- Selects the correct `ServerBlock`
- Resolves the matching `LocationBlock`
- Routes the request to the appropriate handler (GET, POST, DELETE, CGI)

---

## Configuration System

- Inspired by NGINX syntax
- Fully tokenized and validated
- Hierarchical configuration:
  - Server blocks
  - Location blocks
- Inheritance rules similar to real web servers
- Safe defaults with explicit overrides

Simple Example, but we have more improved config files in /config:
```conf
server {
    listen 8080;
    root ./www;

    location / {
        index index.html;
        autoindex on;
    }
}
```
---

## Usage

### *Compilation*

Compile the project using:

```bash
make
```
This command will generate the webserv executable in the root directory.

### *Execution*

Run the server with a configuration file: (or without a file, our project uses a default config file)

```bash
./webserv config/default.conf
```
or

```bash
./webserv
```
The server will start according to the settings defined in the configuration file.

---

### *Manual Testing*
You can open ```http://localhost:8080``` in a browser or, using a terminal:

```bash
curl http://localhost:8080
curl -X POST http://localhost:8080/upload
```
Tests can be done using our html, all methods can be tested in the page.

---

### *Using testers as Siege*
This project can be stress-tested using **Siege**, a HTTP load testing and benchmarking tool.

Install Siege (if not already installed):

for macOS
```bash
brew install siege
```

or linux
```bash
apt-install siege
```

Run a basic stress test:

```bash
siege http://localhost:8080
```

Run Siege with multiple concurrent clients:

```bash
siege -c 20 -r 50 http://localhost:8080
```

Where:
- -c defines the number of concurrent clients
- -r defines the number of repetitions per client

Run Siege in benchmark mode (no delays between requests):

```bash
siege -b http://localhost:8080
```

---

### Memory Check

```bash
make val
```
*This rule in Makefile runs ```valgrind --leak-check=full --show-leak-kinds=all ./webserv config/default.conf``` and shows if there is any memory leak, still reachable resources or opened invalid file descriptors.*

---

### Resources and References

- RFC 1945 — HTTP/1.0
- Linux manuals
- NGINX Documentation
- Beej’s Guide to Network Programming
- *The Linux Programming Interface by Michael Kerrisk*
- The C++ Programming Language by Bjarne Stroustrup
- *Refactoring.guru for Design Patterns*
- *Pair Programming at our campus, of course!*

---

### AI Usage Disclosure
AI tools were used as a support resource, not as code generators. The usage was for:

- Clarifying protocol behavior
- Reviewing architectural decisions
- Explaining differences between design patterns and applications
- Assisting with documentation and tests

