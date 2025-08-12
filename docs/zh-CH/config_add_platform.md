# 添加一个新的平台配置文件

&emsp;&emsp;平台配置文件存放于 **conf/platforms** 目录中，这些文件定义了该平台所需的 toolchain（工具链）和 rootfs（根文件系统）。

要创建一个新平台的配置文件，运行以下命令：

```
celer create --platform=x86_64-linux-22.04
```

> 生成的文件位于 **conf/platforms** 目录中。
> 然后，您需要打开生成的文件并根据您的目标环境进行配置。

## 1. 平台配置文件介绍

让我们看一个示例平台配置文件，**x86_64-linux-22.04.toml**：

  ```toml
  [rootfs]
    url = "https://github.com/celer-pkg/test-conf/releases/download/resource/ubuntu-base-20.04.5-base-amd64.tar.gz"
    name = "gcc"
    version = "9.5"
    path = "ubuntu-base-20.04.5-base-amd64"
    pkg_config_path = [
        "usr/lib/x86_64-linux-gnu/pkgconfig",
        "usr/share/pkgconfig",
        "usr/lib/pkgconfig"
    ]

  [toolchain]
    url = "https://github.com/celer-pkg/test-conf/releases/download/resource/gcc-9.5.0.tar.gz"
    path = "gcc-9.5.0/bin"
    system_name = "Linux"
    system_processor = "x86_64"
    host = "x86_64-linux-gnu"
    crosstool_prefix = "x86_64-linux-gnu-"
    cc = "x86_64-linux-gnu-gcc"
    cxx = "x86_64-linux-gnu-g++"
    fc = "x86_64-linux-gnu-gfortran"            # optional field
    ranlib = "x86_64-linux-gnu-ranlib"          # optional field
    ar = "x86_64-linux-gnu-ar"                  # optional field
    nm = "x86_64-linux-gnu-nm"                  # optional field
    objdump = "x86_64-linux-gnu-objdump"        # optional field
    strip = "x86_64-linux-gnu-strip"            # optional field
  ```

以下是字段和其描述：

| 字段             | 描述 |
| ----------------- | ----------- |
| url               | It can be http、https or ftp url, celer will download it, even it can be local file path, and it should start with **file:///**, e.g. **file:////home/phil/buildresource/ubuntu-base-20.04.5/gcc-9.5.0.tar.gz**. |
| path              | It is the path to the toolchain directory, celer will add it to the environment path during runtime, and it will also be added to $ENV{PATH} in the generated toolchain_file.cmake, which is convenient for compiling during runtime to access the executable files inside. |
| system_name       | It is the name of the system, e.g. **Linux**, **Windows**, **macOS**. |
| system_processor  | Processor of the system, e.g. **x86_64**, **arm64**, **i386**. |
| host              | Host of the toolchain, e.g. **x86_64-linux-gnu**, **aarch64-linux-gnu**, **i686-w64-mingw32**. |
| crosstool_prefix  | Prefix of the toolchain, e.g. **x86_64-linux-gnu-**, **aarch64-linux-gnu-**, **i686-w64-mingw32-**. |
| cc                | Path to the compiler, e.g. **x86_64-linux-gnu-gcc**, **aarch64-linux-gnu-gcc**, **i686-w64-mingw32-gcc**. |
| cxx               | Path to the c++ compiler, e.g. **x86_64-linux-gnu-g++**, **aarch64-linux-gnu-g++**, **i686-w64-mingw32-g++**. |
| fc, ranlib, ar, nm, objdump, strip, etc | They are optional fields, toolchain can find them with `crosstool_prefix`. |

## 2. 配置 Windows 平台

&emsp;&emsp;Windows 使用 MSVC 编译 C/C++ 项目，而 MSVC 的配置与 Linux 的 GCC 有很大的不同。区别在于 MSVC 中的编译器文件名基本上是固定的，但是头文件和库文件分散在不同的目录中，这对于 Celer 来说不是问题。Celer 封装了所有的细节，最终配置 MSVC 平台要简单得多，例如：

```toml
[toolchain]
url = "file:///C:/Program Files/Microsoft Visual Studio/2022/Community"
name = "msvc"
version = "14.44.35207"
system_name = "Windows"
system_processor = "x86_64"
```