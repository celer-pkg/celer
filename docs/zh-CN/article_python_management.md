# Python 版本管理

> **跨平台自动管理 Python 版本，确保编译依赖的一致性**

## 🎯 什么是 Python 版本管理？

Python 已经成为 C/C++ 项目编译期间的依赖库。许多现代库（如 Boost、CMake 插件和各种构建工具）在编译过程中需要使用 Python。Celer 提供了智能的 Python 版本管理，可以：

- **项目级 Python** - 为每个项目配置专属 Python 版本
- **智能回退** - 版本匹配时使用系统 Python，否则通过 conda 下载
- **跨平台支持** - Windows 和 Linux/macOS 采用不同的管理策略
- **虚拟环境隔离** - 按版本隔离 Python 包和环境
- **无缝集成** - 依赖需要时自动设置 Python

**为什么需要 Python 版本管理？**

- **依赖冲突** - 不同项目可能需要不同的 Python 版本
- **构建工具兼容性** - 构建工具需要特定的 Python 小版本号
- **跨平台差异** - 不同平台系统 Python 的可用性和版本差异大
- **包隔离** - 防止项目依赖之间的冲突

---

## 🔧 Python 版本选择原理

### 决策流程图

Celer 使用智能决策树来选择使用哪个 Python：

```
项目的 [python] 配置中是否指定了 Python 版本？
│
├─ 是 → 系统 Python 版本是否匹配？
│       │
│       ├─ 是 → 使用系统 Python ✅
│       └─ 否 → 通过 conda 下载 📥
│
└─ 否 → 是否有配置的默认 Python？
        │
        ├─ 是（Windows） → 使用 buildtools/static 配置的默认 Python 🔧
        ├─ 是（Linux） → 检测系统 Python 🖥️
        └─ 否 → 错误：没有可用的 Python ❌
```

### 平台特定行为

#### Windows

**默认 Python 来源：**
- Celer 从 `buildtools/static/x86_64-windows.toml` 读取默认 Python 版本
- 此文件指定了随 Celer 预配置的 Python 版本
- 首次使用时，Celer 会下载并设置此 Python 版本

**Python 解析流程：**
```
1. 检查项目的 [python] 配置是否指定了 version
   ├─ 如果是，与 Windows 默认 Python 比较版本
   │  ├─ 版本匹配 → 使用系统 Python
   │  └─ 版本不匹配 → 通过 conda 下载
   └─ 如果否，使用 Windows 默认 Python 版本
```

**虚拟环境存储位置：**
```
{工作区}/installed/venv-{小版本号}@{项目名}/
├── Scripts/
│   ├── python.exe
│   ├── pip.exe
│   └── ... 其他工具
└── Lib/
    └── site-packages/
```

#### Linux/macOS

**默认 Python 来源：**
- Celer 尝试检测系统 Python3 版本
- 如果无系统 Python，则回退到 conda

**Python 解析流程：**
```
1. 检测系统 Python3 版本
2. 检查项目的 [python] 配置是否指定了 version
   ├─ 如果是，与系统 Python 比较版本
   │  ├─ 版本匹配 → 使用系统 Python
   │  └─ 版本不匹配 → 通过 conda 下载
   └─ 如果否，使用检测到的系统 Python 版本
```

**虚拟环境存储位置：**
```
{工作区}/installed/venv-{小版本号}@{项目名}/
├── bin/
│   ├── python3
│   ├── pip
│   └── ... 其他工具
└── lib/
    └── python{版本号}/site-packages/
```

---

## 📝 在项目中配置 Python

Python 配置是**项目级**配置，写在项目配置文件（`conf/projects/<项目名>/<项目名>.toml`）里，
不再放在工作空间的 `celer.toml` 中。这样同一项目在不同工作空间中共享同一份 Python 设置，
不同项目也可以各自指定版本和 pip 源。

### 项目配置文件格式

在项目文件末尾添加 `[python]` 表：

```toml
# conf/projects/project_xxx/project_xxx.toml

ports = [
    "boost@1.83.0",
    "openssl@3.0.1",
    "zlib@1.3.1"
]

vars = [
    "CMAKE_VAR1=value1"
]

# Python 配置（可选）
[python]
  version = "3.11.5"
  index_url = "https://pypi.example.com/simple"
  extra_index_urls = ["https://pypi.internal/simple"]
  trusted_hosts = ["pypi.internal"]
```

> **注意**：`[python]` 是 TOML 表，一旦出现，其后所有键值都属于该表。请把它放在文件末尾，
> 或其他表之后，避免把 `ports`、`vars` 等顶层字段误写进 `[python]`。

### 配置字段

| 字段 | 必选 | 描述 | 示例 |
|------|------|------|------|
| `version` | ❌ | Python 版本，用于选择/准备虚拟环境 | `"3.11.5"` |
| `index_url` | ❌ | pip 主索引地址，等价于 `pip install -i` | `"https://pypi.example.com/simple"` |
| `extra_index_urls` | ❌ | pip 额外索引地址列表，等价于 `--extra-index-url` | `["https://pypi.internal/simple"]` |
| `trusted_hosts` | ❌ | pip 信任的 host 列表，等价于 `--trusted-host` | `["pypi.internal"]` |

`index_url`、`extra_index_urls`、`trusted_hosts` 会传给 Celer 内部执行的 `pip download` /
`pip install`，用于在私有 PyPI 镜像或自签名 HTTPS 环境下安装 Python 构建工具。

### 版本指定格式

Python 版本应按以下格式指定：

- **完整版本：** `3.11.5` - 使用精确版本 3.11.5
- **小版本：** `3.11` - 使用 3.11.x 的任何补丁版本
- **主版本：** `3` - 使用 Python 3 的任何版本（不推荐）

**示例：**
```toml
[python]
  version = "3.11.5"   # 完整版本
```

```toml
[python]
  version = "3.10.12"  # 另一个完整版本
```

```toml
[python]
  version = "3.12"     # 小版本号
```

### 未指定时的默认行为

如果项目中没有 `[python]` 表，或其中没有 `version`：

**在 Windows 上：**
- 使用 `buildtools/static/x86_64-windows.toml` 中的默认 Python 版本
- 示例：如果 static 配置指定 Python 3.9.0，所有未指定 `python.version` 的项目都将使用 3.9.0

**在 Linux/macOS 上：**
- 尝试使用检测到的系统 Python3
- 如果系统 Python 无法检测，回退到 conda

---

## 🧰 声明 Python 构建工具库

端口构建期需要的 Python 库在 `build_tools` 中通过 `python3:` 前缀声明。版本是**必填**的，必须使用 `name@version` 形式：

```toml
[[build_configs]]
  build_system = "cmake"
  build_tools = ["python3:mako@1.3.10", "python3:MarkupSafe@2.1.0"]
```

Celer 会用 pip 把每个库安装到项目的虚拟环境中（`mako==1.3.10`）。不带版本的条目（如 `python3:mako`）会被拒绝。`meson` 和 `gyp` 构建系统同样以 Python 包方式安装，因此也必须指定版本（`build_system = "meson@1.11.2"`、`build_system = "gyp@0.19.0"`）。详见 [构建工具](./article_build_tools.md) 和 [Python Wheel 缓存](./article_pkgcache_python_wheels.md)。

---

## 📦 安装 Python 包

Celer 支持通过 `build_system = "custom"` 将纯 Python 包安装到虚拟环境中。流程与 C++ 包一致：**暂存 → venv → trace → 移除**。

### port.toml 模板

```toml
[package]
  url = "https://example.com/my_python_pkg.git"
  ref = "v1.0.0"

[[build_configs]]
  build_system = "custom"
  build_in_source = true
  build = ["${PYTHON_VENV_EXE} setup.py build"]
  install = [
    "cmake -E make_directory ${PACKAGE_DIR}",
    "${PYTHON_VENV_EXE} setup.py install --prefix=${PACKAGE_DIR} --single-version-externally-managed --record=${PACKAGE_DIR}/install.log",
  ]
```

### 可用占位符

| 占位符 | 说明 |
| --- | --- |
| `${PYTHON_VENV_EXE}` | 虚拟环境 `python3` 可执行文件的绝对路径。 |
| `${PYTHON_VENV_DIR}` | 虚拟环境根目录的绝对路径。 |
| `${PACKAGE_DIR}` | 包的暂存目录（与 C++ 包相同）。 |

### 工作流程

1. **构建**：在源码目录执行 `${PYTHON_VENV_EXE} setup.py build`。
2. **安装**：执行 `setup.py install --prefix=${PACKAGE_DIR}`，将文件暂存到包目录。
3. **复制**：Celer 将暂存文件从 `${PACKAGE_DIR}` 复制到虚拟环境。
4. **记录**：写入 trace 文件，记录所有安装的文件（相对于 `installed/`）。
5. **移除**：`celer remove` 读取 trace 文件并从 venv 中删除对应文件。

### 自动检测

当 `build_system = "custom"` 的端口在命令中引用了 `${PYTHON_VENV_EXE}` 或 `${PYTHON_VENV_DIR}` 时，Celer 会自动：

- 触发 Python 虚拟环境初始化（无需在 `build_tools` 中声明 `python3`）。
- 将安装的文件复制到 venv 而非 `InstalledDir`。
- 写入 venv 感知的 trace 文件，支持后续移除。

### 示例：ament_package

```toml
[package]
  url = "https://github.com/ament/ament_package.git"
  ref = "humble"

[[build_configs]]
  build_system = "custom"
  build_in_source = true
  build = ["${PYTHON_VENV_EXE} setup.py build"]
  install = [
    "cmake -E make_directory ${PACKAGE_DIR}",
    "${PYTHON_VENV_EXE} setup.py install --prefix=${PACKAGE_DIR} --single-version-externally-managed --record=${PACKAGE_DIR}/install.log",
  ]
```

安装后，包会出现在 venv 的 `site-packages` 中，可通过 `celer remove ament_package@humble` 移除。


