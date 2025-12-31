# 项目配置

> **为不同项目定义独立的构建环境和依赖关系**

## 🎯 什么是项目配置？

项目配置定义了 Celer 如何为特定项目管理依赖和构建环境。每个项目配置包含五个核心组件：

- 📦 **Ports（依赖库）** - 项目所需的第三方库
- 🔧 **Vars（CMake 变量）** - 全局 CMake 构建变量
- 🌍 **Envs（环境变量）** - 构建时需要的环境变量
- 🏷️ **Macros（宏定义）** - C/C++ 预处理器宏
- ⚙️ **Compile Options（编译选项）** - 编译器标志和优化选项

**为什么需要项目配置？**

每个项目都有其独特的配置特征。项目配置让 Celer 能够：
- ✅ 统一管理项目依赖关系
- ✅ 在团队中共享一致的构建环境
- ✅ 快速切换不同项目的构建配置
- ✅ 独立管理每个项目的编译选项和宏定义

**项目文件位置：** 所有项目配置文件存放在 `conf/projects` 目录中。

---

## 📝 项目命名规范

项目配置文件遵循统一的命名格式：

```
project_<名称>.toml
```

**示例：**
- `project_001.toml` - 第一个项目配置
- `project_opencv.toml` - OpenCV 项目配置
- `project_multimedia.toml` - 多媒体项目配置

> 💡 **提示**：建议使用有意义的名称或编号来标识不同项目，方便团队识别和管理。

---

## 🛠️ 配置字段详解

### 完整示例配置

让我们看一个完整的项目配置文件 `project_xxx.toml`：

```toml
ports = [
    "x264@stable",
    "sqlite3@3.49.0",
    "ffmpeg@3.4.13",
    "zlib@1.3.1",
    "opencv@4.5.1"
]

vars = [
    "CMAKE_VAR1=value1",
    "CMAKE_VAR2=value2"
]

envs = [
    "ENV_VAR1=001"
]

macros = [
    "MICRO_VAR1=111",
    "MICRO_VAR2"
]

[optimize]
    debug = "-O0 -g3 -fno-omit-frame-pointer"
    release = "-O3 -DNDEBUG"
    relwithdebinfo = "-O2 -g -DNDEBUG"
    minsizerel = "-Os -DNDEBUG"
```

>其中optimize可以一次性配置多个编译器的情况，目的是为了一个project在多个系统以及多个编译器下共用一个project的toml配置，如下：

```toml
[optimize_msvc]
    debug = "/MDd /Zi /Od /Ob0 /RTC1"
    release = "/MD /O2 /Ob2 /DNDEBUG"
    relwithdebinfo = "/MD /Zi /O2 /Ob1 /DNDEBUG"
    minsizerel = "/MD /O1 /Ob1 /DNDEBUG"

[optimize_gcc]
    debug = "-O0 -g3 -fno-omit-frame-pointer"
    release = "-O3 -DNDEBUG"
    relwithdebinfo = "-O2 -g -DNDEBUG"
    minsizerel = "-Os -DNDEBUG"

[optimize_clang]
    debug = "-O0 -g3 -fno-omit-frame-pointer"
    release = "-O3 -DNDEBUG"
    relwithdebinfo = "-O2 -g -DNDEBUG"
    minsizerel = "-Oz -DNDEBUG"
```

### 配置字段说明

| 字段 | 必选 | 描述 | 示例 |
|------|------|------|------|
| `ports` | ❌ | 定义当前项目依赖的第三方库。格式为 `包名@版本号` | `["x264@stable", "zlib@1.3.1"]` |
| `vars` | ❌ | 定义当前项目所需的全局 CMake 变量。格式为 `变量名=值` | `["CMAKE_BUILD_TYPE=Release"]` |
| `envs` | ❌ | 定义当前项目所需的全局环境变量。格式为 `变量名=值` | `["xorg_cv_malloc0_returns_null=yes"]` |
| `macros` | ❌ | 定义当前项目所需的 C/C++ 宏定义。格式为 `宏名=值` 或 `宏名` | `["DEBUG=1", "ENABLE_LOGGING"]` |
| `optimize` | ❌ | 定义固定一个编译器的编译优化选项 | 见下文详细说明 |
| `optimize_gcc` | ❌ | 定义 GCC 编译器的优化选项 | 见下文详细说明 |
| `optimize_clang` | ❌ | 定义 Clang 编译器的优化选项 | 见下文详细说明 |
| `optimize_msvc` | ❌ | 定义 MSVC 编译器的优化选项 | 见下文详细说明 |

> ⚠️ **注意**：所有字段都是可选的，您可以根据项目需求选择性配置。

### 1️⃣ Ports（依赖库）

指定项目所依赖的第三方库，Celer 会自动下载、编译和安装这些依赖。

**格式：** `"包名@版本号"`

**示例：**
```toml
ports = [
    "zlib@1.3.1",           # 压缩库
    "openssl@3.0.0",        # 加密库
    "sqlite3@3.49.0",       # 数据库
    "x264@stable"           # 视频编码（使用 stable 版本）
]
```

**版本说明：**
- 指定具体版本：`@3.49.0`
- 使用特定标签：`@stable`, `@latest`
- 版本格式必须与 `ports` 目录中定义的版本一致

> 💡 **提示**：可以使用 `celer search <包名>` 查看可用的版本列表。

### 2️⃣ Vars（CMake 变量）

定义全局 CMake 变量，这些变量会传递给所有依赖库以及App开发项目的构建过程。

**格式：** `"变量名=值"`

**示例：**
```toml
vars = [
    "PROJECT_NAME=telsa/model3",
    "PROJECT_CODE=0033FF"
]
```

### 3️⃣ Envs（环境变量）

定义构建时需要的环境变量，影响编译过程的行为。

**格式：** `"变量名=值"`

**示例：**
```toml
envs = [
    "CFLAGS=-march=native",         # 设置 C 编译器标志
    "CXXFLAGS=-march=native"        # 设置 C++ 编译器标志
]
```

### 4️⃣ Macros（宏定义）

定义 C/C++ 预处理器宏，在编译时注入到代码中。

**格式：** `"宏名=值"` 或 `"宏名"`（无值宏）

**示例：**
```toml
macros = [
    "DEBUG=1",              # 启用调试模式（有值宏）
    "ENABLE_LOGGING",       # 启用日志功能（无值宏）
    "MAX_BUFFER_SIZE=4096", # 定义缓冲区大小
    "_GNU_SOURCE"           # 启用 GNU 扩展
]
```

### 5️⃣ Optimize（编译优化选项）

定义不同构建类型下的编译器优化标志。Celer 支持为不同编译器配置独立的优化选项，实现跨平台、跨编译器的一致性构建配置。

#### 🎯 配置方式

**方式 1：通用配置（适用于所有编译器）**

使用 `[optimize]` 配置通用的编译优化选项：

```toml
[optimize]
    debug = "-O0 -g3 -fno-omit-frame-pointer"
    release = "-O3 -DNDEBUG"
    relwithdebinfo = "-O2 -g -DNDEBUG"
    minsizerel = "-Os -DNDEBUG"
```

**方式 2：编译器特定配置（推荐）**

为不同编译器配置独立的优化选项，一个项目可以在多个系统和编译器下共用同一个配置文件：

```toml
# GCC 编译器优化配置
[optimize_gcc]
    debug = "-O0 -g3 -fno-omit-frame-pointer"
    release = "-O3 -DNDEBUG"
    relwithdebinfo = "-O2 -g -DNDEBUG"
    minsizerel = "-Os -DNDEBUG"

# Clang 编译器优化配置
[optimize_clang]
    debug = "-O0 -g3 -fno-omit-frame-pointer"
    release = "-O3 -DNDEBUG"
    relwithdebinfo = "-O2 -g -DNDEBUG"
    minsizerel = "-Oz -DNDEBUG"  # Clang 使用 -Oz 获得更小体积

# MSVC 编译器优化配置
[optimize_msvc]
    debug = "/MDd /Zi /Od /Ob0 /RTC1"
    release = "/MD /O2 /Ob2 /DNDEBUG"
    relwithdebinfo = "/MD /Zi /O2 /Ob1 /DNDEBUG"
    minsizerel = "/MD /O1 /Ob1 /DNDEBUG"
```

> 💡 **优先级规则**：Celer 会优先使用编译器特定的配置（如 `optimize_gcc`），如果未定义，则使用通用配置 `optimize`。

#### 📋 构建类型说明

| 构建类型 | 字段名 | 用途 | 典型场景 |
|---------|--------|------|----------|
| Debug | `debug` | 调试构建，无优化，包含完整调试信息 | 开发、调试、问题定位 |
| Release | `release` | 发布构建，最高优化，无调试信息 | 生产环境、性能测试 |
| RelWithDebInfo | `relwithdebinfo` | 发布构建 + 调试信息 | 性能分析、生产环境调试 |
| MinSizeRel | `minsizerel` | 最小体积优化 | 嵌入式系统、存储受限环境 |

#### 🔧 GCC/Clang 常用优化选项

**优化级别：**

| 选项 | 描述 | 适用场景 |
|------|------|----------|
| `-O0` | 无优化，快速编译 | Debug 构建 |
| `-O1` | 基本优化 | 平衡编译速度和性能 |
| `-O2` | 中等优化（推荐） | RelWithDebInfo 构建 |
| `-O3` | 最高优化 | Release 构建 |
| `-Os` | 优化代码体积 | MinSizeRel 构建（GCC） |
| `-Oz` | 更激进的体积优化 | MinSizeRel 构建（Clang） |

**调试选项：**

| 选项 | 描述 |
|------|------|
| `-g` | 生成基本调试信息 |
| `-g3` | 生成最详细调试信息（包含宏定义） |
| `-fno-omit-frame-pointer` | 保留栈帧指针，便于调试和性能分析 |

**其他常用选项：**

| 选项 | 描述 |
|------|------|
| `-DNDEBUG` | 禁用断言（assert） |
| `-Wall` | 启用所有常见警告 |
| `-Wextra` | 启用额外警告 |
| `-fPIC` | 生成位置无关代码 |
| `-march=native` | 针对当前 CPU 优化 |
| `-flto` | 启用链接时优化（LTO） |

#### 🪟 MSVC 常用优化选项

**优化级别：**

| 选项 | 描述 | 适用场景 |
|------|------|----------|
| `/Od` | 禁用优化 | Debug 构建 |
| `/O1` | 最小化代码体积 | MinSizeRel 构建 |
| `/O2` | 最大化速度 | Release/RelWithDebInfo 构建 |

**调试选项：**

| 选项 | 描述 |
|------|------|
| `/Zi` | 生成完整调试信息（PDB 文件） |
| `/Z7` | 在 .obj 文件中嵌入调试信息 |

**运行时库：**

| 选项 | 描述 |
|------|------|
| `/MD` | 多线程 DLL 运行时（Release） |
| `/MDd` | 多线程 DLL 运行时（Debug） |
| `/MT` | 多线程静态运行时（Release） |
| `/MTd` | 多线程静态运行时（Debug） |

**内联选项：**

| 选项 | 描述 |
|------|------|
| `/Ob0` | 禁用内联 |
| `/Ob1` | 仅内联标记为 inline 的函数 |
| `/Ob2` | 编译器自动内联 |

**其他选项：**

| 选项 | 描述 |
|------|------|
| `/RTC1` | 运行时检查（检测栈损坏、未初始化变量） |
| `/DNDEBUG` | 定义 NDEBUG 宏（禁用断言） |
| `/GL` | 全程序优化 |
| `/std:c++17` | 使用 C++17 标准 |

---

## 🚀 使用项目配置

### 创建新项目

使用 `celer create` 命令创建基于项目配置的新项目：

```bash
# 使用指定的项目配置
celer create --project x86_64-linux-ubuntu-22.04-gcc-11.5.0
```

### 切换项目配置

使用 `celer configure` 命令切换项目:

```bash
celer configure --project project_001
```

或在 `celer.toml` 中修改项目配置：

```toml
project = "project_001"
```

### 查看项目依赖

查看当前项目的依赖树：

```bash
celer tree project_001
```

---

## 📚 相关文档

- [快速开始指南](./quick_start.md) - 开始使用 Celer
- [创建项目](./cmd_create.md) - 使用 celer create 命令
- [平台配置](./article_platform.md) - 配置编译工具链

---

**需要帮助？** [报告问题](https://github.com/celer-pkg/celer/issues) 或查看我们的[文档](../../README.md)
