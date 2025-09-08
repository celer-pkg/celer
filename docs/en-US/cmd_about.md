# About command

&emsp;&emsp;The about command displays essential information about the Celer build system, including:

- Version number and release channel (stable/beta/nightly).
- Usage guide.
- License information.

## Command Syntax

```shell
celer about
```

The output of the command is as follows:

```shell
./celer about       

Welcome to celer (v0.0.0).
-------------------------------------------
This is a lightweight pkg-manager for C/C++.

How to apply it in your cmake project:
option1: set(CMAKE_TOOLCHAIN_FILE "D:/Workspace/celer/toolchain_file.cmake")
option2: cmake .. -DCMAKE_TOOLCHAIN_FILE="D:/Workspace/toolchain_file.cmake"
```