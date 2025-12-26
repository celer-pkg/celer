# 生成 CMake 配置文件

> **为非 CMake 库自动生成标准 CMake 配置文件**

## 🎯 为什么需要这个功能？

许多优秀的第三方库（如 FFmpeg、x264）不使用 CMake 作为构建系统，安装后也不会生成 CMake 配置文件。这给使用 CMake 的项目带来了集成困难：

**传统方案的问题：**
- 🔍 **查找困难**：需要手动编写 `FindXXX.cmake` 模块
- 🪟 **平台差异**：Windows 上使用 `pkg-config` 很困难
- 🔗 **依赖复杂**：多组件库的依赖关系难以管理
- ⚙️ **维护成本高**：每个库都需要自定义查找脚本

**Celer 的解决方案：**
- ✅ 自动生成标准 CMake 配置文件
- ✅ 跨平台一致的使用体验
- ✅ 自动处理组件间依赖关系
- ✅ 支持静态库、动态库和 Interface 库

## 📚 配置类型概览

根据库的特点，选择合适的配置类型：

| 类型 | 使用场景 | 典型示例 | 配置复杂度 |
|------|---------|---------|----------|
| **🎯 单目标库** | 单一库文件，没有子模块 | x264, zlib, sqlite | ⭐ 简单 |
| **📦 多组件库** | 包含多个独立模块，可单独使用 | FFmpeg, Boost, OpenCV | ⭐⭐⭐ 中等 |
| **🔗 Interface 库** | 预编译库或仅头文件库 | 预构建 SDK, header-only 库 | ⭐⭐ 简单 |

---

### 使用场景

适用于只有一个主库文件的简单库，例如：
- **x264**：视频编码库
- **zlib**：压缩库
- **sqlite3**：数据库引擎

### 配置步骤

#### 步骤1：创建配置文件

在端口的版本目录中创建 `cmake_config.toml` 文件：

```shell
x264
└── stable
    ├── cmake_config.toml  # ← 创建此文件
    └── port.toml
```

#### 步骤2：编写配置

`cmake_config.toml` 内容示例：

```toml
# 命名空间，也是 CMake 配置文件的前缀
namespace = "x264"

# Linux 静态库配置
[linux_static]
filename = "libx264.a"  # 库文件名

# Linux 动态库配置
[linux_shared]
filename = "libx264.so.164"  # 实际文件名（带版本号）
soname = "libx264.so"        # 符号链接名（SONAME）

# Windows 静态库配置
[windows_static]
filename = "x264.lib"

# Windows 动态库配置
[windows_shared]
filename = "libx264-164.dll"  # DLL 文件名
impname = "libx264.lib"       # 导入库名（.lib）
```

**字段说明：**

| 字段 | 说明 | 平台 | 必需 |
|------|------|------|------|
| `namespace` | CMake 命名空间和配置文件前缀 | 通用 | 否* |
| `filename` | 实际库文件名 | 全部 | 是 |
| `soname` | 共享库的符号名（符号链接） | Linux | 动态库必需 |
| `impname` | 导入库文件名 | Windows | 动态库必需 |

> 💡 *如果未指定 `namespace`，将使用库名作为默认值

#### 步骤3：生成的文件

编译安装后，在 `lib/cmake/` 目录下会生成：

```shell
lib/cmake/x264
├── x264Config.cmake           # 主配置文件
├── x264ConfigVersion.cmake    # 版本信息
├── x264Targets.cmake          # 目标定义
└── x264Targets-release.cmake  # Release 配置
```

#### 步骤4：在项目中使用

```cmake
# 查找库
find_package(x264 REQUIRED)

# 链接到你的目标
target_link_libraries(${PROJECT_NAME} PRIVATE x264::x264)
```

---

## 2️⃣ 多组件库配置

### 使用场景

适用于包含多个独立模块的库，每个模块可以单独使用，例如：
- **FFmpeg**：包含 avcodec、avformat、avutil 等多个模块
- **Boost**：包含众多独立的子库
- **OpenCV**：包含 core、imgproc、video 等模块

### 配置步骤

#### 步骤1：创建配置文件

```shell
ffmpeg
└── 5.1.6
    ├── cmake_config.toml  # ← 创建此文件
    └── port.toml
```

#### 步骤2：编写配置

`cmake_config.toml` 内容示例（仅展示部分组件）：

```toml
namespace = "FFmpeg"

[linux]
# avutil 组件 - 基础工具库（无依赖）
[[linux.components]]
component = "avutil"                    # 组件名
filename = "libavutil.so.55"            # 库文件名
dependencies = []                       # 无依赖

# avcodec 组件 - 编解码器（依赖 avutil）
[[linux.components]]
component = "avcodec"
filename = "libavcodec.so.57"
dependencies = ["avutil"]              # 依赖 avutil

[[linux.components]]
component = "avdevice"
filename = "libavdevice.so.57"
dependencies = ["avformat", "avutil"]

[[linux.components]]
...

[windows]
...
```

> **注意：**  
> 不同的组件可能有不同的依赖关系，CMake 会在生成的配置文件中自动处理这些依赖。

编译安装后，会生成如下配置文件：

```
lib
└── cmake
    └─── FFmpeg
        ├── FFmpegConfig.cmake
        ├── FFmpegConfigVersion.cmake
        └── FFmpegTarget.cmake
```

最后，在你的 CMake 项目中可以这样使用：

```cmake
find_package(FFmpeg REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
    FFmpeg::avutil
    FFmpeg::avcodec
    FFmpeg::avdevice
    FFmpeg::avfilter
    FFmpeg::avformat
    FFmpeg::postproc
    FFmpeg::swresample
    FFmpeg::swscale
)
```

---

## 3️⃣ 预构建库 CMake 配置生成

对于预构建库（`build_system = "prebuilt"`），你可以手动创建 `cmake_config.toml` 文件，无需手动配置。

```
prebuilt-ffmpeg
└── 5.1.6
    ├── cmake_config.toml
    └── port.toml
```

```toml
[package]
ref = "5.1.6"

[[build_configs]]
url = "https://github.com/celer-pkg/test-conf/releases/download/resource/prebuilt-ffmpeg@5.1.6@x86_64-linux.tar.gz"
pattern = "x86_64-linux*"
build_system = "prebuilt"
```

```toml
namespace = "FFmpeg"

[linux]
filenames = [
    "libavutil.so.57",
    "libavcodec.so.59",
    "libavdevice.so.59",
    "libavfilter.so.8",
    "libavformat.so.59",
    "libpostproc.so.56",
    "libswresample.so.4",
    "libswscale.so.6",
]

[windows]
filenames = [
    "avutil.lib",
    "avcodec.lib",
    "avdevice.lib",
    "avfilter.lib",
    "avformat.lib",
    "postproc.lib",
    "swresample.lib",
    "swscale.lib",
]
```

> 💡 **提示**：对于 Interface 类型，只需列出所有需要链接的库文件名，无需指定组件或依赖关系。

**步骤3：生成的文件**

```
lib/cmake/FFmpeg/
├── FFmpegConfig.cmake
└── FFmpegConfigVersion.cmake
```

**步骤4：在项目中使用**

```cmake
find_package(FFmpeg REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE FFmpeg::prebuilt-ffmpeg)
```

> **注意：**  
> **1.** 如果未指定 `namespace`，将使用库名作为默认值。

---

## 🎯 最佳实践

### 选择正确的配置类型

```
你的库是什么情况？
│
├─ 只有一个主库文件？
│   └─ ✅ 使用【单目标库】配置
│
├─ 有多个独立的模块/组件？
│   └─ ✅ 使用【多组件库】配置
│
└─ 已经预编译或只有头文件？
    └─ ✅ 使用【Interface 库】配置
```

### 命名规范建议

| 项目 | 建议 | 示例 |
|------|------|------|
| **namespace** | 使用库的官方名称（首字母大写） | `"FFmpeg"`, `"OpenCV"` |
| **component** | 使用小写，与库的模块名一致 | `"avcodec"`, `"core"` |
| **filename** | 使用实际的文件名（含版本号） | `"libavcodec.so.57.107.100"` |
| **soname** | 使用主版本号的符号名 | `"libavcodec.so.57"` |

### 依赖管理技巧

**✅ 推荐做法：**
- 明确声明组件间的直接依赖
- 按依赖顺序组织组件（基础库在前）
- 使用注释说明每个组件的用途

**❌ 避免：**
- 循环依赖
- 声明不必要的传递依赖
- 使用不存在的组件名

---

## 🔧 故障排查

### 问题1：配置文件未生成

**可能原因：**
- ✗ `cmake_config.toml` 文件位置不正确
- ✗ TOML 语法错误
- ✗ 库文件实际不存在

**解决方法：**
```bash
# 检查配置文件语法
celer install <library> --verbose/-v

# 查看详细的安装日志
```

### 问题2：CMake 找不到库

**可能原因：**
- ✗ `namespace` 与 `find_package()` 名称不匹配
- ✗ CMake 搜索路径未包含安装目录

**解决方法：**
```cmake
# 方法1：设置 CMAKE_PREFIX_PATH
set(CMAKE_PREFIX_PATH "/path/to/celer/installed" ${CMAKE_PREFIX_PATH})

# 方法2：使用 toolchain 文件（推荐）
# celer deploy 会自动生成包含正确路径的 toolchain_file.cmake
```

### 问题3：链接错误

**症状：** undefined reference 错误

**可能原因：**
- ✗ 组件依赖关系声明不正确
- ✗ 缺少必要的系统库

**解决方法：**
```toml
# 检查并修正 dependencies 字段
[[linux_shared.components]]
component = "avcodec"
dependencies = ["avutil"]  # 确保包含所有直接依赖
```

### 问题4：文件名不匹配

**症状：** 库文件找不到

**解决方法：**
```bash
# 检查实际安装的文件名
ls -la /path/to/installed/lib/

# 确保 filename 字段与实际文件名完全一致（包括版本号）
```

---

## 📚 相关文档

- [快速开始](./quick_start.md) - 开始使用 Celer
- [创建新端口](./cmd_create.md#3-创建一个新的端口) - 添加新库
- [缓存构建产物](./article_binary_cache.md) - 加速构建

---

**需要帮助？** [报告问题](https://github.com/celer-pkg/celer/issues) 或查看[文档](../../README.md)