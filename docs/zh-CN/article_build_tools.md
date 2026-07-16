# 构建工具

> **跨平台自动下载和管理构建时工具链及实用程序**

## 🎯 什么是构建工具？

C/C++ 项目通常依赖多种构建时工具 — CMake、Ninja、CCache、LLVM/Clang、Git、Python 等。在不同平台和团队成员之间手动安装和版本匹配这些工具既容易出错又耗时。

Celer 的**构建工具**系统通过以下方式解决这个问题：

- 📦 **内置工具注册表** — Celer 按平台（如 `x86_64-linux`、`x86_64-windows`）捆绑了常用构建工具的预配置定义。
- 🔧 **用户自定义覆盖** — 通过 `conf/buildtools/<arch>-<os>.toml` 自定义或添加工具，无需修改 Celer 本身。
- 🔄 **自动下载与缓存** — 工具在首次使用时自动下载并缓存到下载目录。
- 🎯 **版本锁定** — 每个工具可以有多个版本；标记一个为 `default` 即可零配置使用。
- 🛤️ **PATH 注入** — 工具二进制文件在构建期间自动添加到 `PATH`。
- 🌍 **跨平台** — Linux、Windows 和 macOS 的不同工具集，均使用相同的 TOML 格式描述。

**为什么需要构建工具管理？**

- 🚫 **版本漂移** — 不同团队成员可能安装了不同版本的 CMake/Ninja。
- 🔧 **缺少依赖** — 新贡献者需要花费数小时安装构建前提条件。
- 🌍 **平台差异** — Linux 和 Windows 需要完全不同的工具集（如 Windows 上的 MSYS2）。
- 📦 **CI 可复现性** — 锁定的工具版本确保 CI 和开发环境完全一致。

---

## 📂 构建工具的定义位置

Celer 从两个来源查找构建工具，按顺序合并：

| 优先级 | 来源 | 位置 | 用途 |
|--------|------|------|------|
| 1（低） | **内置** | `buildtools/static/<arch>-<os>.toml` | Celer 捆绑的默认配置 |
| 2（高） | **用户配置** | `conf/buildtools/<arch>-<os>.toml` | 用户覆盖和新增 |

> 💡 **合并逻辑**：如果两个来源中存在相同 `name` 和 `version` 的工具，用户配置会**完全替换**内置定义。不同名称或版本的工具则追加到列表中。

**当前各平台内置工具：**

| 工具 | Linux | Windows | 描述 |
|------|:-----:|:-------:|------|
| `cmake` | ✅ | ✅ | CMake 构建系统 |
| `ninja` | ✅ | ✅ | Ninja 构建系统 |
| `ccache` | ✅ | ✅ | 编译器缓存 |
| `git` | — | ✅ | Windows 版 Git (MinGit) |
| `git-lfs` | ✅ | ✅ | Git 大文件存储 |
| `llvm` | ✅ | — | LLVM/Clang 工具链 |
| `conda` | ✅ | ✅ | Miniforge（Python 和包管理） |
| `python3` | — | ✅ | Windows 上的 Python 解释器 |
| `msys2` | — | ✅ | Windows 上的 MSYS2 环境 |
| `strawberry-perl` | — | ✅ | Windows 版 Perl |
| `vswhere` | — | ✅ | Visual Studio 定位器 |

---

## 📝 配置格式

每个构建工具在 TOML 文件中定义为一个 `[[build_tools]]` 条目。

### 完整字段参考

```toml
[[build_tools]]
  name    = "cmake"           # 工具标识符（必填）
  version = "4.3.2"           # 版本字符串（必填）
  default = true              # 标记为默认版本（可选）
  url     = "https://..."     # 下载地址（必填）
  sha256  = "abc123..."       # SHA-256 校验和（可选，推荐填写）
  archive = "cmake-4.3.2..." # 重命名下载的文件（可选）
  paths   = ["cmake-4.3.2.../bin"]  # 添加到 PATH 的子路径（可选）
  vars    = ["MY_VAR=value"]  # 全局表达式变量（可选）
  envs    = ["MY_ENV=value"]  # 要设置的环境变量（可选）
```

### 字段详解

#### `name`
工具标识符。当工具需要特定版本时，以 `name` 或 `name@version` 的格式引用。工具名称必须唯一；使用相同 `name` 但不同 `version` 的多个 `[[build_tools]]` 条目来支持多版本。

#### `version`
标识工具版本的任意字符串。用于版本锁定和匹配。当依赖请求 `"cmake"`（不带版本）时，会选择带有 `default = true` 的条目。

#### `default`
标记在不指定版本时使用哪个版本。每个工具名称只能有一个条目设置 `default = true`。

#### `url`
下载地址。Celer 在首次使用时下载工具并缓存。

#### `sha256`
下载文件的 SHA-256 哈希值，用于完整性校验。留空（`""`）则跳过校验。

#### `archive`
重命名下载的文件。当 URL 文件名比较通用（如 `download.zip`）而您希望使用描述性名称时很有用，或者当解压后的文件夹名称与压缩包名称不同时使用。

#### `paths`
要添加到系统 `PATH` 的子目录列表（相对于解压后的归档根目录）。每个路径会与工具目录拼接。

- **有 `paths`**：归档解压到 `downloads/tools/<第一个路径组件>/`，每个子路径都会添加到 `PATH`。
- **无 `paths`**：工具被视为单文件下载，直接放在 `downloads/` 目录下。

#### `vars`
通过 `${VAR_NAME}` 在配置表达式中可用的全局表达式变量。每个条目格式为 `KEY=VALUE`。变量名不能已被定义。

#### `envs`
工具激活时全局设置的环境变量。每个条目格式为 `KEY=VALUE`。变量名不能已在当前环境中存在。

---

## 🔧 构建工具的解析流程

### 工具选择规则

1. **精确版本匹配**：`"cmake@3.31.12"` 匹配 `version = "3.31.12"` 的 `cmake` 条目。
2. **默认版本**：`"cmake"`（不带版本）匹配带有 `default = true` 的 `cmake` 条目。
3. **歧义报错**：如果找到工具名但没有 `default` 条目且未指定版本，Celer 会报错。
4. **系统回退（仅 Linux）**：在构建工具注册表中未找到的工具会通过系统包管理器检查。

---

## 🛠️ 自定义构建工具

### 添加新工具

在您的工作区中创建或编辑 `conf/buildtools/<arch>-<os>.toml`：

```toml
# conf/buildtools/x86_64-linux.toml
[[build_tools]]
  name = "doxygen"
  version = "1.14.0"
  default = true
  url = "https://www.doxygen.nl/files/doxygen-1.14.0.linux.bin.tar.gz"
  archive = "doxygen-1.14.0.linux.bin.tar.gz"
  paths = ["doxygen-1.14.0/bin"]
  envs = ["DOXYGEN_DIR=${WORKSPACE_DIR}/downloads/tools/doxygen-1.14.0"]
  vars = ["DOXYGEN_ENABLED=true"]
```

这会将 `doxygen` 添加到 Linux x86_64 的工具注册表中。

### 添加带环境变量的工具

某些工具需要设置环境变量才能正常工作：

```toml
[[build_tools]]
  name = "my-custom-tool"
  version = "1.0"
  default = true
  url = "https://example.com/my-tool.tar.gz"
  sha256 = "..."
  paths = ["my-tool/bin"]
  vars = ["TOOL_DIR=${root}"]
  envs = ["MY_TOOL_HOME=/path/to/tool"]
```

---

## 📋 完整配置示例

### 完整 Linux 配置

```toml
# conf/buildtools/x86_64-linux.toml

[[build_tools]]
  name = "cmake"
  version = "4.3.2"
  default = true
  url = "https://github.com/Kitware/CMake/releases/download/v4.3.2/cmake-4.3.2-linux-x86_64.tar.gz"
  sha256 = "791ae3604841ca03cb3889a3ad89165346e4b180ae3448efd4b0caa9ef46d245"
  archive = "cmake-4.3.2-linux-x86_64.tar.gz"
  paths = ["cmake-4.3.2-linux-x86_64/bin"]

[[build_tools]]
  name = "ninja"
  version = "v1.12.1"
  default = true
  url = "https://github.com/ninja-build/ninja/releases/download/v1.12.1/ninja-linux.zip"
  sha256 = "6f98805688d19672bd699fbbfa2c2cf0fc054ac3df1f0e6a47664d963d530255"
  archive = "ninja-linux-x86_64-v1.12.1.zip"
  paths = ["ninja-linux-x86_64-v1.12.1"]

[[build_tools]]
  name = "ccache"
  version = "4.12.1"
  default = true
  url = "https://github.com/ccache/ccache/releases/download/v4.12.1/ccache-4.12.1-linux-x86_64.tar.xz"
  sha256 = "742e6a6e17c0a060046874eece2949b221c228e1119698a4c6e0b096cbc87152"
  archive = "ccache-4.12.1-linux-x86_64.tar.xz"
  paths = ["ccache-4.12.1-linux-x86_64"]
```

### 完整 Windows 配置

```toml
# conf/buildtools/x86_64-windows.toml

[[build_tools]]
  name = "cmake"
  version = "4.3.2"
  default = true
  url = "https://github.com/Kitware/CMake/releases/download/v4.3.2/cmake-4.3.2-windows-x86_64.zip"
  sha256 = "83d20c23f5c5f64b3b328785e35b23c532e33057a97ed6294acaca3781b78a01"
  archive = "cmake-4.3.2-windows-x86_64.zip"
  paths = ["cmake-4.3.2-windows-x86_64/bin"]

[[build_tools]]
  name = "ninja"
  version = "v1.12.1"
  default = true
  url = "https://github.com/ninja-build/ninja/releases/download/v1.12.1/ninja-win.zip"
  sha256 = "f550fec705b6d6ff58f2db3c374c2277a37691678d6aba463adcbb129108467a"
  archive = "ninja-v1.12.1-windows-x86_64.zip"
  paths = ["ninja-v1.12.1-windows-x86_64"]

[[build_tools]]
  name = "git"
  version = "2.49.0"
  default = true
  url = "https://github.com/git-for-windows/git/releases/download/v2.49.0.windows.1/MinGit-2.49.0-64-bit.zip"
  sha256 = "971cdee7c0feaa1e41369c46da88d1000a24e79a6f50191c820100338fb7eca5"
  archive = "MinGit-2.49.0-64-bit.zip"
  paths = ["MinGit-2.49.0-64-bit/cmd"]

[[build_tools]]
  name = "msys2"
  version = "2.16.03"
  default = true
  url = "https://github.com/msys2/msys2-installer/releases/download/2025-02-21/msys2-base-x86_64-20250221.tar.xz"
  sha256 = "850589091e731d14b234447084737ca62aee1cc1e3c10be62fcdc12b8263d70b"
  archive = "msys2-base-x86_64-20250221.tar.xz"
  paths = [
    "msys2-base-x86_64-20250221/msys64/usr/bin",
    "msys2-base-x86_64-20250221/msys64/bin",
  ]
```
