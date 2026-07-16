# 动态变量

> **Celer 支持的动态变量清单，以及可用位置**

## 语法

- `${VAR}`：推荐写法。
- `$VAR`：同样支持。
- `$ENV{NAME}`：读取当前进程环境变量。
- `~`：展开为 `$ENV{HOME}`的值。

## 内置变量

| 变量 | 来源 | 说明 |
| --- | --- | --- |
| `${BUILDTREES_DIR}` | Celer 上下文 | buildtrees 根目录 |
| `${INSTALLED_DIR}` | Celer 上下文 | 当前平台/项目/构建类型的安装目录 |
| `${INSTALLED_DEV_DIR}` | Celer 上下文 | 主机工具的 dev 安装目录 |

## 平台变量

这些变量在平台配置加载后注入。

| 变量 | 来源 | 说明 |
| --- | --- | --- |
| `${HOST}` | `toolchain.host` | 目标 host triple |
| `${SYSTEM_NAME}` | 平台工具链元数据 | 用于配置匹配的系统选择变量 |
| `${SYSTEM_PROCESSOR}` | `toolchain.system_processor` | CPU 架构选择变量 |
| `${SYSTEM_VERSION}` | `toolchain.system_version` | 系统版本号，Android系统必填 |
| `${CROSSTOOL_PREFIX}` | `toolchain.crosstool_prefix` | 工具链可执行前缀 |
| `${TOOLCHAIN}` | 工具链根目录 | 用于平台表达式展开的工具链根目录 |
| `${SYSROOT}` | `rootfs` | 仅在配置了 rootfs 时可用 |
| `${CC}` | `toolchain.cc` | C 编译器。可用于 Rust 交叉编译，如 `CC_aarch64_unknown_linux_gnu=${CC}` |
| `${CXX}` | `toolchain.cxx` | C++ 编译器 |
| `${CPP}` | `toolchain.cpp` | C 预处理器 |
| `${AR}` | `toolchain.ar` | 静态库归档器 |
| `${LD}` | `toolchain.ld` | 链接器 |
| `${AS}` | `toolchain.as` | 汇编器 |
| `${OBJCOPY}` | `toolchain.objcopy` | 目标文件复制 |
| `${OBJDUMP}` | `toolchain.objdump` | 目标文件转储 |
| `${STRIP}` | `toolchain.strip` | 去除符号 |
| `${READELF}` | `toolchain.readelf` | ELF 读取器 |
| `${SIZE}` | `toolchain.size` | 段大小 |
| `${STRINGS}` | `toolchain.strings` | 可打印字符串 |
| `${NM}` | `toolchain.nm` | 符号表 |
| `${RANLIB}` | `toolchain.ranlib` | 归档索引 |
| `${GCOV}` | `toolchain.gcov` | 覆盖率工具 |
| `${ADDR2LINE}` | `toolchain.addr2line` | 地址转行号 |
| `${CXXFILT}` | `toolchain.cxxfilt` | 符号反修饰 |
| `${FC}` | `toolchain.fc` | Fortran 编译器 |

## 可选全局变量

| 变量 | 来源 | 说明 |
| --- | --- | --- |
| `${PYTHON_PATH}` | 自动识别 Python3 | Python3 可执行文件路径（相对于工作区） |
| `${PYTHON_VENV_EXE}` | 自动识别 Python3 | 虚拟环境 Python 可执行文件的绝对路径 |
| `${PYTHON_VENV_DIR}` | 自动识别 Python3 | 虚拟环境根目录的绝对路径 |

## 端口构建变量

这些变量会在匹配到 `build_config` 后、构建执行前注入。

| 变量 | 来源 | 说明 |
| --- | --- | --- |
| `${REPO_DIR}` | 当前端口 | 端口源码仓库目录 |
| `${SRC_DIR}` | 当前端口 | 端口源码解压目录 |
| `${BUILD_DIR}` | 当前端口 | 端口构建目录 |
| `${PACKAGE_DIR}` | 当前端口 | 端口打包输出目录 |
| `${DEV_DEPS_DIR}` | workspace tmp deps | 主机构建工具依赖目录 |
| `${DEPS_DIR}` | workspace tmp deps | 当前模式（dev/target）对应依赖目录 |

## 变量替换生效位置

- 平台 TOML 中 `toolchain.envs`。
- `project.vars`、`project.envs`、`project.macros`、`toolchain.cflags`、`toolchain.cxxflags`、`toolchain.linkflags`。
- 端口 `build_config` 中的 `envs`、`options` 等字段。

## 构建阶段额外占位符

以下占位符不属于 `ExprVars`，但在构建选项展开时也会替换：

- `${CC}`
- `${CXX}`
