# 🚀 快速上手

## 📋 目录

1. [克隆仓库](#1-克隆仓库)
2. [编译 Celer](#2-编译-celer)
3. [配置 Conf](#3-配置-conf)
4. [配置平台或项目](#4-配置平台或项目)
5. [部署 Celer](#5-部署-celer)
6. [构建您的 CMake 项目](#6-构建您的-cmake-项目)

---

## 1. 📦 克隆仓库

第一步是从 GitHub 克隆 Celer 仓库，这是 Celer 项目的所有源代码。

**执行命令：**

```shell
git clone https://github.com/celer-pkg/celer.git
```

---

## 2. 🔨 编译 Celer

### 编译步骤

1. **安装 Go SDK**  
   参考官方文档：https://go.dev/doc/install

2. **编译 Celer**  
   进入 Celer 目录，执行以下命令：
   ```shell
   cd celer
   go build
   ```

### 💡 提示

在中国，你可能需要为 Go 设置代理：

```shell
export GOPROXY=https://goproxy.cn
```

### 📌 注意

目前 Celer 已经发布稳定版本，用户可以直接下载预构建的二进制文件，跳过前两步。

**下载地址：** https://github.com/celer-pkg/celer/releases

---

## 3. ⚙️ 配置 Conf

### 什么是 Conf？

不同的 C++ 项目通常需要不同的构建环境和依赖项。Celer 建议使用 **conf** 来定义每个项目的跨平台编译环境和第三方依赖项。

### Conf 目录结构

```
conf
├── buildtools/                      # 构建工具配置
│   ├── x86_64-linux.toml
│   └── x86_64-windows.toml
├── platforms/                       # 平台配置
│   ├── aarch64-linux-gnu-gcc-9.2.toml
│   ├── x86_64-linux-ubuntu-22.04-gcc-11.5.0.toml
│   └── x86_64-windows-msvc-community-14.44.toml
└── projects/                        # 项目配置
    ├── project_test_01/             # 项目 01 的依赖
    │   ├── boost/                   # 覆盖公共构建选项
    │   │   └── 1.87.0/
    │   │       └── port.toml
    │   └── sqlite3/                 # 覆盖公共构建选项
    │       └── 3.49.0/
    │           └── port.toml
    ├── project_test_01.toml         # 项目 01 配置文件
    ├── project_test_02/             # 项目 02 的依赖
    │   ├── ffmpeg/                  # 覆盖公共构建选项
    │   │   └── 5.1.6/
    │   │       └── port.toml
    │   ├── lib_001/                 # 项目私有库
    │   │   └── port.toml
    │   └── lib_002/                 # 项目私有库
    │       └── port.toml
    └── project_test_02.toml         # 项目 02 配置文件
```

### 📚 Conf 文件说明

| 文件路径 | 描述 |
|---------|------|
| `buildtools/*.toml` | 定义某些第三方库在编译期间需要的额外工具 |
| `platforms/*.toml` | 定义平台配置，包括工具链和根文件系统 |
| `projects/*.toml` | 定义项目配置，包括依赖项、CMake 变量、C++ 宏和构建选项 |
| `projects/*/port.toml` | 用于覆盖项目特定的第三方库版本、自定义构建参数和定义项目的私有库 |

### 🔗 相关文档

- [创建新平台](./cmd_create.md#1-创建一个新的平台)
- [创建新项目](./cmd_create.md#2-创建一个新的项目)
- [创建新端口](./cmd_create.md#3-创建一个新的端口)

### 📌 注意事项

虽然 **conf** 是 Celer 推荐的配置方式，但 Celer 也可以在没有 **conf** 的情况下工作。在这种情况下，Celer 会使用本地工具链来构建第三方库：

- **Windows 环境：** Celer 通过 `vswhere` 定位已安装的 Visual Studio 作为默认工具链。对于基于 makefile 的库，它会自动下载并配置 MSYS2。
- **Linux 环境：** Celer 会自动使用本地安装的 x86_64 gcc/g++ 工具链。

### 初始化 Conf

执行以下命令来配置 conf：

```shell
celer init --url=https://github.com/celer-pkg/test-conf.git
```

### 🌏 配置代理（可选）

**如果你在中国，建议配置代理以便于访问 GitHub 等资源：**

```shell
celer configure --proxy-host 127.0.0.1 --proxy-port 7890
```

### 📄 生成的配置文件

执行初始化命令后，会在工作目录中生成 `celer.toml` 配置文件：

```toml
[global]
  conf_repo = "https://github.com/celer-pkg/test-conf.git"
  platform = ""
  project = ""
  jobs = 16
  build_type = "release"
  offline = false
  verbose = false

[proxy]
  host = "127.0.0.1"
  port = 7890
```

### 💡 提示

- **测试仓库：** `https://github.com/celer-pkg/test-conf.git` 是一个测试用的 conf 仓库，你可以使用它来体验 Celer，也可以参考它创建自己的 conf 仓库。
- **Ports 仓库：** 在初始化期间，Celer 会克隆一个 ports 仓库到当前工作目录，该仓库包含所有可用的第三方库配置文件。
  - Celer 会优先使用环境变量 `CELER_PORTS_REPO` 中指定的 ports 仓库
  - 如果环境变量未设置，Celer 会使用默认的 ports 仓库：`https://github.com/celer-pkg/ports.git`

---

## 4. 🎯 配置平台或项目

### 灵活的组合方式

**platform** 和 **project** 是两个独立的配置项，它们可以自由组合。例如，即使目标环境是 **aarch64-linux**，你也可以选择在 **x86_64-linux** 平台上进行编译/开发/调试。

### 配置命令

```shell
# 配置平台
celer configure --platform=x86_64-linux-ubuntu-22.04-gcc-11.5.0

# 配置项目
celer configure --project=project_test_02
```

### 更新后的配置文件

配置完成后，`celer.toml` 文件会更新为以下内容：

```toml
[global]
  conf_repo = "https://github.com/celer-pkg/test-conf.git"
  platform = "x86_64-linux-ubuntu-22.04-gcc-11.5.0"
  project = "project_test_02"
  jobs = 16
  build_type = "release"
  offline = false
  verbose = false

[binary_cache]
  dir = "/home/phil/celer_cache"
```

### 📊 配置字段说明

| 字段 | 描述 |
|------|------|
| `conf_repo` | 用于保存平台和项目配置的仓库 URL |
| `platform` | 当前工作空间选定的平台。为空时，Celer 会自动检测并使用本地工具链来编译库和项目 |
| `project` | 当前工作空间选定的项目。为空时，会创建一个名为 \"unname\" 的默认项目 |
| `jobs` | Celer 编译时使用的最大 CPU 核心数，默认值为系统 CPU 的核心数 |
| `build_type` | 构建类型，默认为 `release`，也可设置为 `debug` |
| `offline` | 离线模式。启用后，Celer 将不再尝试更新仓库和下载资源 |
| `verbose` | 详细模式。启用后，Celer 将生成更详细的编译日志 |
| `binary_cache` | 二进制缓存配置。Celer 支持缓存构建产物以避免重复编译。[可配置为本地目录或局域网共享文件夹](./article_binary_cache.md) |

---

## 5. 🚀 部署 Celer

### 什么是部署？

部署 Celer 是在所选平台的构建环境中构建项目所需的所有第三方库的过程。

### 执行部署

```shell
celer deploy
```

### 部署产物

成功部署后，会在工作空间目录下生成以下文件和目录：

- **`toolchain_file.cmake`** - CMake 工具链文件，允许项目仅依赖此文件，无需再使用 Celer
- **`installed/`** - 已安装的第三方库目录
- **`downloads/`** - 下载的源代码目录

### 💡 提示

你可以将工作空间打包为以下三部分，作为构建环境分发给其他用户使用：

1. `installed/` 文件夹
2. `downloads/` 文件夹
3. `toolchain_file.cmake` 文件

---

## 6. 🏗️ 构建您的 CMake 项目

使用 Celer 生成的 `toolchain_file.cmake` 文件，构建 CMake 项目变得非常简单。

### 方式一：在 CMakeLists.txt 中设置

```cmake
set(CMAKE_TOOLCHAIN_FILE "/path/to/workspace/toolchain_file.cmake")
```

### 方式二：通过命令行参数指定

```shell
cmake .. -DCMAKE_TOOLCHAIN_FILE="/path/to/workspace/toolchain_file.cmake"
```

### 📌 注意事项

- 请将 `/path/to/workspace/toolchain_file.cmake` 替换为实际的工具链文件路径
- 使用工具链文件后，CMake 会自动找到所有已安装的第三方库
- 项目无需再依赖 Celer，可以独立构建

---

## 📚 相关文档

- [平台配置进阶](./article_platform.md)
- [端口配置进阶](./article_port.md)
- [二进制缓存配置](./article_binary_cache.md)
- [命令参考](./cmd_install.md)

---

## ❓ 获取帮助

如需更多帮助，请运行：

```shell
celer --help
```

或访问 [Celer 官方文档](https://github.com/celer-pkg/celer)
