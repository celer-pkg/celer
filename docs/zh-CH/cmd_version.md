# Version 命令

&emsp;&emsp;Version 命令显示Celer的基本信息，包括：

- 版本号;
- 使用指南;

## 命令语法

```shell
celer version
```

命令输出如下：

```shell
./celer version

Welcome to celer (v0.0.0).
-------------------------------------------
This is a lightweight pkg-manager for C/C++.

How to apply it in your cmake project:
option1: set(CMAKE_TOOLCHAIN_FILE "/celer_workspace/toolchain_file.cmake")
option2: cmake .. -DCMAKE_TOOLCHAIN_FILE="/celer_workspace/toolchain_file.cmake"
```