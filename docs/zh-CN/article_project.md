# 项目配置

> **为不同项目定义独立的构建环境和依赖关系**

## 什么是项目配置？

项目配置定义了 Celer 如何为特定项目管理依赖和构建环境。每个项目配置包含五个核心组件：

- 📦 **Ports（依赖库）** - 项目所需的第三方库
- 🔧 **Vars（CMake 变量）** - 全局 CMake 构建变量
- 🌍 **Envs（环境变量）** - 构建时需要的环境变量
- 🏷️ **Macros（宏定义）** - C/C++ 预处理器宏

**为什么需要项目配置？**

每个项目都有其独特的配置特征。项目配置让 Celer 能够：
- ✅ 统一管理项目依赖关系
- ✅ 在团队中共享一致的构建环境
- ✅ 快速切换不同项目的构建配置
- ✅ 独立管理每个项目的优化策略和宏定义

**项目文件位置：** 所有项目配置文件存放在 `conf/projects` 目录中。

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
```

### 配置字段说明

| 字段 | 必选 | 描述 | 示例 |
|------|------|------|------|
| `ports` | ❌ | 定义当前项目依赖的第三方库。格式为 `包名@版本号` | `["x264@stable", "zlib@1.3.1"]` |
| `vars` | ❌ | 定义当前项目所需的全局 CMake 变量。格式为 `变量名=值` | `["CMAKE_BUILD_TYPE=Release"]` |
| `envs` | ❌ | 定义当前项目所需的全局环境变量。格式为 `变量名=值` | `["xorg_cv_malloc0_returns_null=yes"]` |
| `macros` | ❌ | 定义当前项目所需的 C/C++ 宏定义。格式为 `宏名=值` 或 `宏名` | `["DEBUG=1", "ENABLE_LOGGING"]` |

> ⚠️ **注意**：所有字段都是可选的，您可以根据项目需求选择性配置。

### 1. Ports（依赖库）

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

### 2. Vars（CMake 变量）

定义全局 CMake 变量，这些变量会传递给所有依赖库以及App开发项目的构建过程。

**格式：** `"变量名=值"`

**示例：**
```toml
vars = [
  "PROJECT_NAME=telsa/model3",
  "PROJECT_CODE=0033FF"
]
```

### 3. Envs（环境变量）

定义构建时需要的环境变量，影响编译过程的行为。

**格式：** `"变量名=值"`

**示例：**
```toml
envs = [
  "PKG_CONFIG_PATH=/opt/sdk/pkgconfig",
  "QNX_CONFIGURATION=/opt/qnx/.qnx"
]
```

### 4. Macros（宏定义）

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

---

## 使用项目配置

### 创建新项目

使用 `celer create` 命令创建基于项目配置的新项目：

```bash
# 使用指定的项目配置
celer create --project=x86_64-linux-ubuntu-22.04-gcc-11.5.0
```

### 切换项目配置

使用 `celer configure` 命令切换项目:

```bash
celer configure --project=project_001
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