# 平台配置

> **为不同目标平台配置交叉编译环境**

## 什么是平台配置？

平台配置定义了 Celer 如何为特定目标系统编译 C/C++ 库。每个平台配置包含两个核心组件：

- 🔧 **Toolchain（工具链）** - 编译器、链接器和其他构建工具
- 📦 **Rootfs（根文件系统）** - 目标系统的头文件和库文件

**为什么需要平台配置？**

构建 C/C++ 项目需要正确的编译器和系统库。平台配置让 Celer 能够：
- ✅ 为不同操作系统（Linux、Windows、macOS）构建
- ✅ 支持交叉编译（如在 x86 上构建 ARM 二进制文件）
- ✅ 使用特定编译器版本（GCC 9.5、Clang 14、MSVC 2022）
- ✅ 管理多平台构建环境

**平台文件位置：** 所有平台配置文件存放在 `conf/platforms` 目录中。

---

## 📝 平台命名规范

平台配置文件遵循统一的命名格式：

```
<架构>-<系统>-<发行版>-<编译器>-<版本>.toml
```

**示例：**
- `x86_64-linux-ubuntu-22.04-gcc-11.5.0.toml`
- `aarch64-linux-gnu-gcc-9.2.toml`
- `x86_64-windows-msvc-14.44.toml`

**命名组成部分：**

| 部分 | 说明 | 示例 |
|------|------|------|
| 架构 | CPU 架构 | `x86_64`, `aarch64`, `arm` |
| 系统 | 操作系统 | `linux`, `windows`, `darwin` |
| 发行版 | 系统发行版（可选） | `ubuntu-22.04`, `centos-7` |
| 编译器 | 工具链类型 | `gcc`, `clang`, `msvc`, `qcc` |
| 版本 | 编译器版本 | `11.5.0`, `14.44` |

> 💡 **提示**：一致的命名有助于团队快速识别和选择正确的平台配置。

## 🛠️ 配置字段详解

### 完整示例配置

让我们看一个完整的 Linux 平台配置文件 `x86_64-linux-ubuntu-22.04-gcc-9.5.toml`：

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
  cflags = ["-fPIC"]                          # 可选字段
  cxxflags = ["-fPIC", "-stdlib=libc++"]      # 可选字段
  linkflags = ["-Wl,--as-needed"]             # 可选字段
  fc = "x86_64-linux-gnu-gfortran"            # 可选字段
  ranlib = "x86_64-linux-gnu-ranlib"          # 可选字段
  ar = "x86_64-linux-gnu-ar"                  # 可选字段
  nm = "x86_64-linux-gnu-nm"                  # 可选字段
  objdump = "x86_64-linux-gnu-objdump"        # 可选字段
  strip = "x86_64-linux-gnu-strip"            # 可选字段
```

### 1. Toolchain（工具链）配置字段

| 字段 | 必选 | 描述 | 示例 |
|------|------|------|------|
| `url` | ✅ | 工具链下载地址或本地路径。支持 http/https/ftp 协议，本地路径需以 `file:///` 开头 | `https://...gcc-9.5.0.tar.gz`<br>`file:///C:/toolchains/gcc.tar.gz` |
| `path` | ✅ | 工具链 bin 目录的相对路径。Celer 会将其添加到 PATH 环境变量和 CMake 的 `$ENV{PATH}` 中 | `gcc-9.5.0/bin` |
| `system_name` | ✅ | 目标操作系统名称 | `Linux`, `Windows`, `Darwin` |
| `system_processor` | ✅ | 目标 CPU 架构 | `x86_64`, `aarch64`, `arm`, `i386` |
| `host` | ✅ | 工具链的目标三元组，定义编译器生成代码的目标平台 | `x86_64-linux-gnu`<br>`aarch64-linux-gnu`<br>`i686-w64-mingw32` |
| `crosstool_prefix` | ✅ | 工具链可执行文件的前缀，用于查找编译器工具 | `x86_64-linux-gnu-`<br>`arm-none-eabi-` |
| `cc` | ✅ | C 编译器可执行文件名 | `x86_64-linux-gnu-gcc`<br>`clang` |
| `cxx` | ✅ | C++ 编译器可执行文件名 | `x86_64-linux-gnu-g++`<br>`clang++` |
| `name` | ✅ | 工具链名称（用于标识） | `gcc`, `clang`, `msvc`, `qcc` |
| `version` | ✅ | 工具链版本号 | `9.5`, `11.3`, `14.0.0` |
| `c_compiler_target` | ❌ | 传递给 CMake 的 C 编译器 target 变体（`CMAKE_C_COMPILER_TARGET`） | `gcc_ntoaarch64le` |
| `cxx_compiler_target` | ❌ | 传递给 CMake 的 C++ 编译器 target 变体（`CMAKE_CXX_COMPILER_TARGET`） | `gcc_ntoaarch64le_cxx` |
| `envs` | ❌ | 工具链额外环境变量（适用于需要运行时环境的工具链，例如 QNX） | `["QNX_CONFIGURATION=/dir/of/qnx/license"]` |
| `cflags` | ❌ | 追加到 `toolchain_file.cmake` 中 `CMAKE_C_FLAGS_INIT` 的 C 编译参数 | `["-fPIC", "--sysroot=${SYSROOT_DIR}"]` |
| `cxxflags` | ❌ | 追加到 `toolchain_file.cmake` 中 `CMAKE_CXX_FLAGS_INIT` 的 C++ 编译参数 | `["-fPIC", "-stdlib=libc++"]` |
| `linkflags` | ❌ | 追加到 `toolchain_file.cmake` 中 `CMAKE_EXE/SHARED/MODULE_LINKER_FLAGS_INIT` 的链接参数 | `["-Wl,--as-needed"]` |
| `cflags_debug` | ❌ | 当 `build_type=debug` 时优先使用的 C 编译参数；未配置时回退到 `cflags` | `["-O0", "-g3"]` |
| `cxxflags_debug` | ❌ | 当 `build_type=debug` 时优先使用的 C++ 编译参数；未配置时回退到 `cxxflags` | `["-O0", "-g3"]` |
| `linkflags_debug` | ❌ | 当 `build_type=debug` 时优先使用的链接参数；未配置时回退到 `linkflags` | `["-Wl,--export-dynamic"]` |
| `embedded_system` | ❌ | 是否为嵌入式系统环境（如 MCU、裸机） | `true`（MCU/裸机）<br>`false` 或不设置（常规系统） |
| `fc` | ❌ | Fortran 编译器（如果需要） | `x86_64-linux-gnu-gfortran` |
| `ranlib` | ❌ | 库索引生成器 | `x86_64-linux-gnu-ranlib` |
| `ar` | ❌ | 静态库归档器 | `x86_64-linux-gnu-ar` |
| `nm` | ❌ | 符号表查看器 | `x86_64-linux-gnu-nm` |
| `objdump` | ❌ | 目标文件分析器 | `x86_64-linux-gnu-objdump` |
| `strip` | ❌ | 符号剥离工具 | `x86_64-linux-gnu-strip` |

> ⚠️ **注意**：可选工具（fc、ranlib 等）如果未指定，Celer 会使用 `crosstool_prefix` 自动查找。

### 2. Rootfs（根文件系统）配置字段

| 字段 | 必选 | 描述 | 示例 |
|------|------|------|------|
| `url` | ✅ | 根文件系统下载地址或本地路径。支持 http/https/ftp 协议，本地路径需以 `file:///` 开头 | `https://...ubuntu-base.tar.gz`<br>`file:///D:/sysroots/ubuntu.tar.gz` |
| `path` | ✅ | 根文件系统解压后的目录名 | `ubuntu-base-20.04.5-base-amd64` |
| `pkg_config_path` | ✅ | pkg-config 搜索路径列表，相对于 rootfs 根目录 | `["usr/lib/x86_64-linux-gnu/pkgconfig", "usr/share/pkgconfig"]` |

---

## 💼 实际配置示例

### Linux 平台配置

#### GCC 工具链

```toml
[rootfs]
  url = "https://github.com/celer-pkg/test-conf/releases/download/resource/ubuntu-base-22.04-amd64.tar.gz"
  path = "ubuntu-base-22.04-amd64"
  pkg_config_path = [
      "usr/lib/x86_64-linux-gnu/pkgconfig",
      "usr/share/pkgconfig"
  ]

[toolchain]
  url = "https://github.com/celer-pkg/test-conf/releases/download/resource/gcc-11.3.0.tar.gz"
  path = "gcc-11.3.0/bin"
  system_name = "Linux"
  system_processor = "x86_64"
  host = "x86_64-linux-gnu"
  crosstool_prefix = "x86_64-linux-gnu-"
  cc = "x86_64-linux-gnu-gcc"
  cxx = "x86_64-linux-gnu-g++"
```

#### Clang 工具链

```toml
[toolchain]
  url = "file:///opt/llvm-14.0.0"
  path = "bin"
  system_name = "Linux"
  system_processor = "x86_64"
  host = "x86_64-linux-gnu"
  cc = "clang"
  cxx = "clang++"
```

### 嵌入式系统平台配置

### QNX 平台配置

```toml
[toolchain]
  url = "https://github.com/celer-pkg/test-conf/releases/download/resource/qnx800.tar.xz"
  path = "qnx800/host/linux/x86_64/usr/bin"
  name = "qcc"
  version = "12.2.0"
  system_name = "qnx"
  system_processor = "aarch64"
  host = "aarch64-nto-qnx"
  crosstool_prefix = "ntoaarch64-"
  cc = "qcc"
  cxx = "q++"
  ld = "ntoaarch64-ld"
  ranlib = "ntoaarch64-ranlib"
  ar = "ntoaarch64-ar"
  nm = "ntoaarch64-nm"
  objdump = "ntoaarch64-objdump"
  strip = "ntoaarch64-strip"
  c_compiler_target = "gcc_ntoaarch64le"
  cxx_compiler_target = "gcc_ntoaarch64le_cxx"
  envs = [
    "QNX_CONFIGURATION=${TOOLCHAIN_DIR}/.qnx",
    "QNX_CONFIGURATION_EXCLUSIVE=${TOOLCHAIN_DIR}/.qnx",
    "MAKEFLAGS=${TOOLCHAIN_DIR}/target/qnx/usr/include",
  ]
```

> ⚠️ **注意**： QNX需要提供一些额外的环境变量，比如：license所在路径，定义如上。

#### ARM Cortex-M MCU 配置

嵌入式系统（如 MCU 或裸机环境）需要特殊配置，因为它们没有完整的操作系统：

```toml
[toolchain]
  url = "https://developer.arm.com/-/media/Files/downloads/gnu-rm/gcc-arm-none-eabi-10.3.tar.bz2"
  path = "gcc-arm-none-eabi-10.3/bin"
  system_name = "Generic"
  system_processor = "arm"
  host = "arm-none-eabi"
  crosstool_prefix = "arm-none-eabi-"
  embedded_system = true
  cc = "arm-none-eabi-gcc"
  cxx = "arm-none-eabi-g++"
  ar = "arm-none-eabi-ar"
  objcopy = "arm-none-eabi-objcopy"
  objdump = "arm-none-eabi-objdump"
```

> 💡 **关键要点**：
> - `embedded_system = true` 告诉 Celer 这是嵌入式环境
> - `system_name = "Generic"` 表示没有特定操作系统
> - `host = "arm-none-eabi"` 是裸机 ARM 工具链的标准三元组
> - 不需要 rootfs 配置，因为 MCU 没有文件系统

### Windows 平台配置

#### MSVC 2022 配置

Windows 使用 MSVC 编译 C/C++ 项目。MSVC 的配置与 Linux GCC 不同：
- ✅ 编译器文件名是固定的（`cl.exe`、`link.exe`）
- ✅ 头文件和库文件分散在多个目录
- ✅ Celer 自动处理所有路径配置

**简化的 MSVC 配置：**

```toml
[toolchain]
url = "file:///C:/Program Files/Microsoft Visual Studio/2022/Community"
name = "msvc"
version = "14.44.35207"
system_name = "Windows"
system_processor = "x86_64"
```

> 💡 **提示**：Celer 会自动检测 MSVC 安装路径，包括 Windows SDK、UCRT 和编译器工具。

---

## 📚 相关文档

- [快速开始指南](./quick_start.md) - 开始使用 Celer
- [项目配置](./cmd_create.md) - 在 celer.toml 中选择平台
- [构建配置](./article_port.md) - 配置构建选项和依赖

---

**需要帮助？** [报告问题](https://github.com/celer-pkg/celer/issues) 或查看我们的[文档](../../README.md)
