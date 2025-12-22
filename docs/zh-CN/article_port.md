
# 端口配置（第三方库 Port）

> **为 C/C++ 第三方库统一配置构建规则与依赖**

&emsp;&emsp;Celer 使用一个 git 仓库来管理第三方库的配置文件。该仓库不断扩展，旨在支持越来越多的 C/C++ 第三方库。

## 🎯 什么是端口配置？

端口配置（port.toml）定义了 Celer 如何自动化拉取、构建和安装第三方 C/C++ 库。每个端口配置文件描述了源码获取方式、构建系统、依赖关系和特殊构建选项。

**端口文件位置：** 所有端口配置文件存放在 `ports/<库名>/<版本>/port.toml` 目录中。

让我们看一个示例 port.toml 文件：**ports/glog/0.6.0/port.toml**：

```
### 完整示例配置

```toml
[package]
url = "https://github.com/google/glog.git"
ref = "v0.6.0"
archive = ""            # 可选字段，仅当 url 不是 git 仓库时有效
src_dir = "xxx"         # 可选字段
supported_hosts = []    # 可选字段

[[build_configs]]
pattern = "*linux*"     # 可选字段，默认空
build_system = "cmake"  # 必填字段，可选值：cmake、makefiles、b2、meson 等
cmake_generator = []    # 可选字段
build_tools = []        # 可选字段
library_type = "shared" # 可选字段，默认 shared，可选 static
build_shared = "--with-shared" # 可选字段
build_static = "--with-static" # 可选字段
c_standard = "c99"      # 可选字段
cxx_standard = "cxx17"  # 可选字段
envs = []               # 可选字段
patches = []            # 可选字段
build_in_source = false # 可选字段，默认 false
autogen_options = []    # 可选字段
pre_configure = []      # 可选字段
post_configure = []     # 可选字段
pre_build = []          # 可选字段
options = []            # 可选字段
fix_build = []          # 可选字段
post_build = []         # 可选字段
pre_install = []        # 可选字段
post_install = []       # 可选字段
dependencies = []       # 可选字段
dev_dependencies = []   # 可选字段
```
```

&emsp;&emsp;在 port.toml 中，只有少数字段是必填的，其他都是可选的。大多数情况下，管理一个第三方库都很简单，例如：

```toml
[package]
url = "https://gitlab.com/libeigen/eigen.git"
ref = "3.4.0"

[[build_configs]]
build_system = "cmake"
options = [
    "-DEIGEN_TEST_NO_OPENGL=1",
    "-DBUILD_TESTING=OFF"
]
```

### 主要字段说明

| 字段 | 必选 | 描述 | 示例 |
|------|------|------|------|
| url | ✅ | 库的代码仓库地址，可为 https、ftp 或 file:// 本地路径 | `https://github.com/google/glog.git` |
| ref | ✅ | 标签名、分支名、提交 ID 或压缩包版本号 | `v0.6.0`、`3.4.0` |
| archive | ❌ | 下载压缩包时重命名文件名 | `glog-0.6.0.tar.gz` |
| src_dir | ❌ | 指定 configure/CMakeLists.txt 所在目录 | `icu4c/source` |
| build_configs | ✅ | 构建配置数组，描述不同平台的构建方式 | 见下方示例 |
| dev_dependencies | ❌ | 构建期所需工具（如 autoconf、nasm） | `autoconf@2.72` |

## 🛠️ 构建配置详解

&emsp;&emsp;**build_configs** 被设计为一个数组，以满足不同系统平台上库的不同编译需求。Celer 会根据 **pattern** 自动找到匹配的 **build_config** 来组装编译命令。  
&emsp;&emsp;第三方库的编译配置通常在不同系统上会有差异。这些差异通常涉及平台特定的编译标志或甚至 entirely distinct build steps。一些库甚至需要特殊的预处理或后处理才能在 Windows 上正确编译。

### 1.2.1 pattern

&emsp;&emsp;**pattern** 用于匹配 **conf** 目录下的 **platform** 文件。其匹配规则与以下表格类似：

| 模式 | 描述 |
| --- | --- |
| * | 空字符串，也为默认值，意味着编译配置不区分系统平台。切换到任何平台都可以使用相同的 buildconfig 来编译 |
| *linux* | 匹配所有 linux 系统 |
| *windows* | 匹配所有 windows 系统 |
| x86_64‑linux* | 匹配所有 cpu 架构为 x86_64，系统为 linux 的平台 |
| aarch64‑linux* | 匹配所有 cpu 架构为 aarch64，系统为 linux 的平台 |
| x86_64‑windows* | 匹配所有 cpu 架构为 x86_64，系统为 windows 的平台 |
| aarch64‑windows* | 匹配所有 cpu 架构为 aarch64，系统为 windows 的平台 |

### 1.2.2 build_system

&emsp;&emsp;不同的构建工具在交叉编译配置上有显著差异。为了简化使用，Celer 抽象出统一的构建系统选项，目前支持 **b2**, **cmake**, **gyp**, **makefiles**, 和**meson**。未来版本将扩展支持更多工具，如 **bazel**, **msbuild** 和 **scons**等。 

### 1.2.3 cmake_generator

&emsp;&emsp;CMake在configure能根据不同的系统生成Unix Makefiles， Xcodee 或者 Visual Studio xxx等构建文件，同时也支持手动制定构建工具，它的值为： **Ninja**, **Unix Makefiles**, **Visual Studio xxxx**.

### 1.2.4 build_tools

&emsp;&emsp;**build_tools** 是一个可选字段，用于指定一些库需要本地安装的额外工具，例如：ruby、perl、甚至通过 pip3 安装的额外 python 库，例如：["ruby", "perl", "python3:setuptools"]。

>**Tip:**  
&emsp;&emsp;实际上，Celer 已内置支持多种构建工具，包括：Windows 版的 CMake、MinGit、strawberry-perl、msys2、vswhere 等。虽然这些工具大多不支持用户配置，但当切换不同的构建系统时，Celer 会自动将它们加入构建工具列表。例如在 Windows 上使用 makefiles 编译时，msys2 就会被自动添加到构建工具中。

### 1.2.5 library_type

&emsp;&emsp;可选配置，用于指定库的类型，默认值为 **shared**，候选值为 **shared** 和 **static**，分别表示动态库和静态库。

### 1.2.6 build_shared，build_static

&emsp;&emsp;可选配置，部分较旧的 makefiles 项目不支持通过 --enable-shared 编译动态库，而是使用 --with-shared 参数。为灵活兼容此类情况，特保留此配置项。您或许会对此处的配置感到困惑，但幸运的是，build_shared 的默认值会根据不同的 buildsystem 自动适配，通常只需在需要时才覆盖指定值。build_shared 与 build_static 的默认值如下：

- **cmake**: "-DBUILD_SHARED_LIBS=ON"
- **makefiles**: "--enable-shared"
- **meson**: "--default-library=shared"
- **b2**: "link=shared runtime-link=shared"

>**注意：**  
>**1:** 由于大多数 C/C++ 库不支持显式地仅编译静态库，因此除非在 port.toml 中手动指定 **build_static**，否则默认值为空。  
>**2:** 有些makefiles项目的构建目标是一个可执行文件，而不是库。在这种情况下，您可以将 **build_shared** 和 **build_static** 设置为 **no** 来分别禁用编译动态库和静态库。

当 **library_type** 被设置为 **shared** 时，尝试读取 **build_shared** 中的值作为编译选项参数，否则读取 **build_static** 中的值作为编译选项参数。

### 1.2.7 c_standard, cxx_standard

&emsp;&emsp;可选配置，默认值为空，分别用于指定 c 和 c++ 标准。
- c_standard 的候选值：**c90**, **c99**, **c11**, **c17**, **c23**;
- cxx_standard 的候选值：**c++11**、**c++14**、**c++17**、**c++20**；

### 1.2.8 build_type

&emsp;&emsp;可选配置，默认值为空，用于指定构建类型。当在 port.toml 中指定 build_type 时，它会覆盖 celer.toml 中定义的全局 build_type 设置。这对于某些需要特定构建类型的库非常有用。
- build_type 的候选值：**release**, **debug**, **relwithdebinfo**, **minsizerel**；
- 如果未指定，则使用 celer.toml 中定义的 build_type（默认为 **release**）

>**注意：** build_type 也会影响 binary cache 的键值计算，不同的 build_type 会生成不同的缓存。

### 1.2.9 envs
&emsp;&emsp;可选配置，默认值为空，用于定义一些环境变量，例如 **CXXFLAGS=-fPIC**，或者甚至编译一些库需要设置指定的环境变量，例如：**libxext** 库在交叉编译到 aarch64 平台时需要设置环境变量：**"xorg_cv_malloc0_returns_null=yes"**，目的是屏蔽编译器检查错误报告；  
&emsp;&emsp;此外需注意，每个库的 toml 文件虽然支持定义 envs 环境变量，但在实际编译过程中，这些环境变量彼此完全独立——每当一个库编译完成时，其 toml 文件中定义的 envs 会从当前进程中被清除。当编译下一个库时，若对应的 toml 文件定义了新的 envs，则会重新设置新的环境变量。

### 1.2.10 patches

&emsp;&emsp;可选配置，默认值为空，用于定义一些补丁文件，例如：某些库的源代码包含问题，导致编译错误。传统上，这需要手动修改源代码并重新编译。为了避免手动干预，我们可以为这些修改创建修复补丁。您可以将多个补丁文件（git 补丁或 Linux 补丁格式均支持）放在端口版本目录中。由于此字段接受数组，因此可以定义多个补丁。Celer 会尝试在每个 configure 步骤之前自动应用这些补丁。

### 1.2.11 build_in_source

&emsp;&emsp;可选配置，默认值为空，用于指定一些库需要在源代码目录中进行配置和构建，例如：**NASM**、**Boost** 等库。注意：此 **build_in_source** 选项主要适用于 makefiles 项目。  
>需注意：b2 构建已经被封装为专用的构建系统（即 buildsystem = "b2"）。

### 1.2.12 autogen_options

&emsp;&emsp;可选配置，默认值为空，用于指定一些库需要在源代码目录中运行 **./autogen.sh** 脚本，例如：**NASM**、**Boost** 等库。注意：此 **autogen_options** 选项主要适用于 makefiles 项目。

### 1.2.13 dependencies

&emsp;&emsp; 可选配置，默认为空，若当前第三方库在编译时依赖其他第三方库，需在此处定义。这些依赖库将在当前库之前完成编译安装。需注意格式必须为 name@version，且必须显式指定依赖库的版本号。

### 1.2.14 dev_dependencies
&emsp;&emsp;可选配置，默认为空，与 dependencies 类似，但此处定义的第三方库依赖项是编译期间所需的工具。例如：许多 makefiles 项目在配置前需要 autoconf、nasm 等工具。所有在 dev_dependencies 中定义的库都将使用本地工具链编译器进行编译安装，它们会被安装到特定目录（如 installed/x86_64-linux-dev），且 installed/x86_64-linux-dev/bin 路径将自动加入 PATH 环境变量，确保编译期间可访问这些工具。

>为什么需要 **dev_dependencies**:   
>- 避免手动使用 **sudo apt install xxx** 安装一些本地工具。  
>- 当编译一个第三方库的新版本时，即使你使用 **apt** 安装了这些工具，仍然可能遇到 **autoconf** 版本过低的错误。在这种情况下，你需要手动下载工具的源代码，本地编译安装，而不是污染系统环境。

### 1.2.15 pre_configure, post_configure, pre_build, fix_build, post_build, pre_install, post_install

&emsp;&emsp;可选配置，默认为空，某些库可能存在代码问题导致编译失败时，可通过补丁修复源码。对于相对较小的问题（如输出文件名错误），可在 post_install 中添加修正命令；同理，若其他阶段出现文件相关问题，也可在对应步骤进行预处理或后处理调整。典型案例如 libffi 库在 Windows 上无法直接编译通过——必须通过多项预处理和后处理步骤才能使其正常工作。

```
# =============== build for windows ============ #
[[build_configs]]
pattern = "*windows*"
build_system = "makefiles"
dev_dependencies = ["autoconf@2.72"]
pre_install = [
    "cmake -E rename ${BUILD_DIR}/.libs/libffi-8.lib ${BUILD_DIR}/.libs/libffi.lib",
]
post_install = [
    "cmake -E make_directory ${PACKAGE_DIR}/bin",
    "cmake -E copy ${BUILD_DIR}/.libs/libffi.lib ${PACKAGE_DIR}/lib/libffi.lib",
    "cmake -E rename ${PACKAGE_DIR}/lib/libffi-8.dll ${PACKAGE_DIR}/bin/libffi-8.dll",
]
options = [
    "..."
]
```

> 注意：Celer 提供了一些动态变量，可在 toml 文件中使用，例如：**${BUILD_DIR}**，在编译过程中会被实际路径替换。更多详情请参考 [动态变量](#3-动态变量)。

### 1.2.15 options

&emsp;&emsp;可选配置，默认值为空，当编译第三方库时，通常会有许多选项需要启用或禁用。我们可以在这里定义它们，例如 **-DBUILD_TESTING=OFF**；

## 📦 动态变量

| 变量 | 描述 | 来源 |
|------|------|------|
| ${SYSTEM_NAME} | 系统名称，如 `x86_64-linux` | platform |
| ${HOST} | 主机名称，如 `x86_64-linux` | platform |
| ${SYSTEM_PROCESSOR} | 系统处理器架构，如 `x86_64` | platform |
| ${SYSROOT} | 系统根目录，如 `/usr/x86_64-linux` | platform |
| ${CROSS_PREFIX} | 交叉编译前缀，如 `x86_64-linux-` | platform |
| ${BUILD_DIR} | 当前库编译目录 | buildtrees |
| ${HOST_NAME} | 主机名称，如 `x86_64-windows` | platform |
| ${PACKAGE_DIR} | 当前库包目录 | port |
| ${BUILDTREES_DIR} | 编译根目录 | buildtrees |
| ${REPO_DIR} | 当前库源码目录 | port/buildtrees |
| ${DEPS_DIR} | 依赖目录 | workspace |
| ${DEPS_DEV_DIR} | 依赖开发目录 | workspace |
| ${PYTHON3_PATH} | 本地 python3 路径，自动识别 | system |

---

## 📚 相关文档

- [快速开始指南](./quick_start.md) - Celer 入门
- [项目配置](./cmd_create.md#2-创建一个新的项目) - 在 celer.toml 中选择端口
- [构建配置](./article_buildconfig.md) - 配置构建选项和依赖

---

**需要帮助？** [报告问题](https://github.com/celer-pkg/celer/issues) 或查看我们的[文档](../../README.md)
