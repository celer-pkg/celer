# 生成 CMake 配置文件

> **为任意库自动生成标准 CMake 配置文件**

## 为什么需要这个功能？

许多第三方库（如 FFmpeg、x264）不使用 CMake 作为构建系统，安装后也不会生成 CMake 配置文件。甚至有些 CMake 库也忘了导出 target。这给 CMake 项目的集成带来了困难：

**传统方案的问题：**
- 🔍 **查找困难**：需要手动编写 `FindXXX.cmake` 模块
- 🪟 **平台差异**：Windows 上使用 `pkg-config` 很困难
- 🔗 **依赖复杂**：多组件库的依赖关系难以管理
- ⚙️ **维护成本高**：每个库都需要自定义查找脚本

**Celer 的解决方案：**
- ✅ 支持**所有**构建系统，安装后自动生成标准 CMake 配置文件
- ✅ 简单库零配置，自动扫描 `lib/` 目录
- ✅ 跨平台一致的使用体验
- ✅ 自动处理组件间依赖关系

---

## 配置类型概览

| 类型 | 使用场景 | 典型示例 | 配置复杂度 |
|------|---------|---------|----------|
| **🎯 单目标库** | 单一库文件，没有子模块 | x264, zlib, sqlite | ⭐ 简单 |
| **📦 多组件库** | 包含多个独立模块，可单独使用 | FFmpeg, Boost, OpenCV | ⭐⭐⭐ 中等 |

---

## 1. 单目标库配置

### 使用场景

适用于只有一个主库文件的简单库，例如：
- **x264**：视频编码库
- **zlib**：压缩库
- **sqlite3**：数据库引擎

### 配置步骤

#### 步骤1：创建配置文件

在端口的版本目录中创建 `cmake_config.toml` 文件：

```
x264
└── stable
    ├── cmake_config.toml  # ← 创建此文件
    └── port.toml
```

#### 步骤2：编写配置（最简形式）

仅需指定命名空间，库文件名自动从 `lib/` 目录扫描：

```toml
namespace = "x264"
```

> 💡 **自动扫描**：当 `filename` / `filenames` 为空且未定义 `components` 时，celer 会自动扫描已安装的 `lib/` 目录中的库文件（`.a`、`.lib`、`.so*`、`.dylib`）。`bin/` 下的 DLL 属于运行时依赖，不会出现在链接列表中。

#### 步骤3：显式指定文件名（可选）

如需精确控制，可以明确指定库文件名：

```toml
namespace = "x264"

[linux]
filename = "libx264.a"

[windows]
filename = "libx264.lib"
```

或列出多个文件：

```toml
namespace = "zlib"

[linux]
filenames = ["libz.a", "libz.so.1"]

[windows]
filenames = ["zlib.lib"]
```

**字段说明：**

| 字段 | 说明 | 必需 |
|------|------|------|
| `namespace` | CMake 命名空间和配置文件前缀 | 否 — 默认为库名 |
| `filename` | 单个库文件名 | 否 — 自动从 `lib/` 扫描 |
| `filenames` | 多个库文件名 | 否 — 自动从 `lib/` 扫描 |

#### 步骤4：生成的文件

安装后，在 `lib/cmake/` 目录下会生成：

```
lib/cmake/x264/
├── x264Config.cmake
├── x264ConfigVersion.cmake
└── x264Targets.cmake
```

#### 步骤5：在项目中使用

```cmake
find_package(x264 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE x264::x264)
```

---

## 2. 多组件库配置

### 使用场景

适用于包含多个独立模块的库，每个模块可以单独使用：
- **FFmpeg**：avcodec、avformat、avutil 等
- **Boost**：众多独立的子库
- **OpenCV**：core、imgproc、video 等模块

### 配置步骤

#### 步骤1：创建配置文件

```
ffmpeg
└── 5.1.6
    ├── cmake_config.toml
    └── port.toml
```

#### 步骤2：编写配置

```toml
namespace = "FFmpeg"

[linux]
[[linux.components]]
  component = "avutil"
  filename = "libavutil.so.57"
  dependencies = []

[[linux.components]]
  component = "avcodec"
  filename = "libavcodec.so.59"
  dependencies = ["avutil"]

[[linux.components]]
  component = "avdevice"
  filename = "libavdevice.so.59"
  dependencies = ["avformat", "avutil"]

[windows]
[[windows.components]]
  component = "avutil"
  filename = "avutil.lib"
  dependencies = []

[[windows.components]]
  component = "avcodec"
  filename = "avcodec.lib"
  dependencies = ["avutil"]
```

> **注意：** 定义了 `components` 后自动扫描会关闭，每个组件必须显式指定 `filename`。

#### 步骤3：生成的文件

```
lib/cmake/FFmpeg/
├── FFmpegConfig.cmake
├── FFmpegConfigVersion.cmake
└── FFmpegTargets.cmake
```

#### 步骤4：在项目中使用

```cmake
find_package(FFmpeg REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
  FFmpeg::avutil
  FFmpeg::avcodec
  FFmpeg::avdevice
)
```

---

## 3. 工作原理

### 支持的构建系统

CMake 配置生成适用于**所有**构建系统。celer 在安装完成后（库文件已放入 `PackageDir/lib/`）执行生成。

| 构建系统 | 生成时机 |
|---------|---------|
| `prebuilt` | configure 阶段（使用 `RepoDir`） |
| `makefiles`、`cmake`、`meson`、`b2`、`gyp`、`qmake`、`bazel`、`custom` | install 之后（使用 `PackageDir`） |

### 自动扫描逻辑

当 `cmake_config.toml` 中未指定 `filename` / `filenames` 且无 `components` 时：

1. 扫描 `PackageDir/lib/` 中的 `.a`、`.lib`、`.so`、`.so.*`、`.dylib`
2. 命名空间默认使用 `port.toml` 中的库名
3. `bin/` 下的 DLL 不会被包含（运行时依赖，非链接时依赖）

```cmake
find_package(FFmpeg REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE FFmpeg::prebuilt-ffmpeg)
```

> **注意：**  
> **1.** 如果未指定 `namespace`，将使用库名作为默认值。