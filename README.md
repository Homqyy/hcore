# HCore {#mainpage}
===============================

For Chinese, please refer to [README-zh.md](./README-zh.md)

## Introduction

This is a high-performance C interface library.

The decision to write this interface library arose from falling in love with the excellent coding standards, APIs, and data structures during the development process of Nginix. For example: `ngx_pool_t`, `ngx_str_t`, `ngx_queue_t`, etc. These interfaces have a high reuse rate and a very good performance advantage. However, unfortunately, they can only be used in the Nginx project and have a strong binding relationship. Therefore, I decided to start separating them from Nginx, hence the birth of the `HCore` prototype. Currently, this interface only supports the `Linux x86_64` system.

I hope that `HCore` can become the basic library of various excellent C projects and accelerate the process of C development.

---

## Development

### Initialization

After obtaining the source code, use the following two commands to obtain the submodule code:

```bash
git submodule init
git submodule update
```

### Development environment

On a host with a container environment:

- Method 1 (Recommended): Supports containerized development through the `VSCode`'s `Remote - Containers` plugin. This way, you can avoid the trouble of setting up a development environment.
    - After running the development environment through the `Remote - Containers` plugin, execute `./build.sh` to build.
- Method 2: Execute the command `./build.sh -d` to build. This method will build the image and run the container, and compile and other operations in the container.

On a host without a container environment:

1. Install dependencies
    - cmake: v3.0.0 or above
    - gcc
2. Build: `./build.sh`

---

## Directory Structure Introduction

```sh
hcore
├── build.sh                    # Build script
├── CMakeLists.txt.in           # Template file for generating CMakeLists.txt
├── Config.cmake.in             # Template file for generating configuration information (Config.cmake)
├── CPackConfig-debug.cmake     # Debug version packaging configuration file
├── CPackConfig-release.cmake   # Release version packaging configuration file
├── Dockerfile
├── docs                        # Document directory
├── Doxyfile                    # Doxygen configuration file
├── googletest
├── hcore_config.h.in
├── include                     # Header file directory
├── Jenkinsfile
├── LICENSE                     # License file
├── README.md -> ./docs/README-zh.md
├── src                         # Source code directory
├── tests                       # Unit test directory
├── tools-dev                   # Development tools
```

---

## API Overview

### Logging (hcore_log)

Logging file: [hcore_log.h](./include/hcore_log.h)

A logging interface used to record program logs. It supports the common 8 logging levels and can use `_HCORE_DEBUG` to control whether debug logs are compiled into the program to distinguish between debug versions and formal versions.

The supported logging levels from high to low are:

- Standard Error
- Emergency
- Alert
- Critical
- Error
- Warning
- Notice
- Informational
- Debug

#### Example

[docs/example/e_log.c](./docs/example/e_log.c)

After running, you can see log content similar to the following in `/var/log/myapp.log`:

```text
2023/06/28 10:29:22 -0000 UTC [ debug] 12417 unknown: (0) This is a debug message.
2023/06/28 10:29:22 -0000 UTC [  warn] 12417 unknown: (0) This is a warning message.
2023/06/28 10:29:22 -0000 UTC [ error] 12417 unknown: (0) This is an error message.
2023/06/28 10:29:22 -0000 UTC [ debug] 12417 unknown: (0) run cleanup: 0000000001DA8390
```
