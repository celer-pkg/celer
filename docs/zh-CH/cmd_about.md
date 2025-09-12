# About 命令

&emsp;&emsp;About 命令显示Celer的基本信息，包括：

- 版本号和发布渠道（稳定/测试/ nightly）。
- 使用指南。
- 许可证信息。

## 命令语法

```shell
celer about
```

命令输出如下：

```shell
./celer about       

Welcome to celer (v0.0.0).
-------------------------------------------
This is a lightweight pkg-manager for C/C++.

How to apply it in your cmake project:
option1: set(CMAKE_TOOLCHAIN_FILE "D:/Workspace/celer/toolchain_file.cmake")
option2: cmake .. -DCMAKE_TOOLCHAIN_FILE="D:/Workspace/toolchain_file.cmake"
```