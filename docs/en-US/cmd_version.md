# Version command

&emsp;&emsp;The version command displays information about the Celer build system, including:

- Version number.
- Usage guide.

## Command Syntax

```shell
celer version
```

&emsp;&emsp;The output of the command is as follows:

```shell
./celer version

Welcome to celer (v0.0.0).
-------------------------------------------
This is a lightweight pkg-manager for C/C++.

How to apply it in your cmake project:
option1: set(CMAKE_TOOLCHAIN_FILE "/celer_workspace/toolchain_file.cmake")
option2: cmake .. -DCMAKE_TOOLCHAIN_FILE="/celer_workspace/toolchain_file.cmake"
```