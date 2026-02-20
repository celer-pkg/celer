# Port Configuration (Third-Party Library Port)

&emsp;&emsp;Celer uses a git repository to manage third-party library configuration files. This repository is continuously expanding and aims to support more and more C/C++ third-party libraries.

## 1. Introduction to port.toml

Let's look at an example port.toml file: **ports/glog/0.6.0/port.toml**:

```
[package]
url                 = "https://github.com/google/glog.git"
ref                 = "v0.6.0"
archive             = ""                    # optional field, only works when url is not a git repo url.
src_dir             = "xxx"                 # optional field
supported_hosts     = [...]                 # optional field

[[build_configs]]
system_name         = "linux"               # optional selector
system_processor    = "x86_64"              # optional selector
build_system        = "cmake"               # mandatory field, should be **cmake**, **makefiles**, **b2**, **meson**, etc.
cmake_generator     = []                    # optional field, should be "Ninja", "Unix Makefiles", "Visual Studio xxx"
build_tools         = [...]                 # optional field
library_type        = "shared"              # optional field, should be **shared**, **static**, and default is **shared**.
build_shared        = "--with-shared"       # optional field
build_static        = "--with-static"       # optional field
c_standard          = "c99"                 # optional field
cxx_standard        = "cxx17"               # optional field
build_type          = "release"             # optional field, default is build_type in celer.toml
envs                = [...]                 # optional field
patches             = [...]                 # optional field
build_in_source     = false                 # optional field, default is **false**
autogen_options     = [...]                 # optional field
pre_configure       = [...]                 # optional field
post_configure      = [...]                 # optional field
pre_build           = [...]                 # optional field
options             = [...]                 # optional field
fix_build           = [...]                 # optional field
post_build          = [...]                 # optional field
pre_install         = [...]                 # optional field
post_install        = [...]                 # optional field
dependencies        = [...]                 # optional field
dev_dependencies    = [...]                 # optional field
```

&emsp;&emsp;In a port.toml, there are many fields that can be configured, but actually only a few are mandatory, and the rest are optional. Most of the time, managing a third-party library is very simple, for example:

```
[package]
url = "https://gitlab.com/libeigen/eigen.git"
ref = "3.4.0"

[[build_configs]]
build_system = "cmake"
options = [
    "-DEIGEN_TEST_NO_OPENGL=1", 
    "-DBUILD_TESTING=OFF"，
]
```

The following are fields and their descriptions:

| Field | Description |
| --- | --- |
| url | The url to clone or download library code, it can be https or ftp, or **file:///** for local directory or repo. |
| ref | Tag name, branch name, commit id, or version in archive filename. |
| archive | Optional, only works when url is not a git url. Used to rename downloaded archive file. |
| src_dir | Optional, used to specify where **configure** or **CMakeLists.txt** is located. |
| build_tool | Optional. Set to `true` for build-time tools (e.g. m4, automake, libtool, autoconf): always built natively, install path has no platform/project/buildType segments, and only built on Linux/Darwin (no need for supported_hosts). |
| build_configs | Array, describes how to build the library on different platforms. |
| dev_dependencies | Array, tools required during build (e.g. autoconf, nasm). |

## 1.2 build_configs

&emsp;&emsp;**build_configs** is an array to meet different compilation requirements on different platforms. Celer will automatically find the matching **build_config** according to **system_name/system_processor** to assemble the compilation command. Build configuration often varies across systems, involving platform-specific flags or distinct build steps. Some libraries require special pre-processing or post-processing to compile correctly on Windows.

### 1.2.1 system_name, system_processor

&emsp;&emsp;Used to match selectors in platform toolchain (`toolchain.system_name`, `toolchain.system_processor`). Matching rules are as follows:

| Selector | Description |
| --- | --- |
| `system_name` unset and `system_processor` unset | Match all platforms (no selector constraints) |
| `system_name = "linux"` | Match all Linux platforms |
| `system_name = "windows"` | Match all Windows platforms |
| `system_name = "linux"` + `system_processor = "x86_64"` | Match x86_64 Linux platforms |
| `system_name = "linux"` + `system_processor = "aarch64"` | Match aarch64 Linux platforms |
| `system_name = "windows"` + `system_processor = "x86_64"` | Match x86_64 Windows platforms |

### 1.2.2 build_system

&emsp;&emsp;Different build tools vary significantly in their cross-compilation configurations. To simplify usage, Celer abstracts them into unified buildsystem options, celer supports build systems as shown below:

- **cmake**: Standard CMake build system
- **makefiles**: Autotools/Makefiles build system
- **meson**: Meson build system
- **b2**: Boost.Build system
- **gyp**: GYP build system
- **qmake**: Qt build system
- **prebuilt**: Pre-built library
- **nobuild**: Header-only libraries requiring no compilation
- **custom**: Custom build logic

### 1.2.3 cmake_generator

&emsp;&emsp;CMake can generate default build system files in different OS and also allows to specify it manually. Currently, celer supports **Ninja**, **Unix Makefiles**, **Visual Studio xxxx**.

### 1.2.4 build_tools

&emsp;&emsp;Optional, some libraries require certain tools to be installed, such as: ruby, perl, or even additional python libraries via pip3, for example: pip3 install setuptools. Celer has managered some builtin buildtools: [windows builtin buildtools](../../buildtools/static/x86_64-windows.toml) and [linux builtin buildtools](../../buildtools/static/x86_64-linux.toml), Celer also support definng extral buildtools by creating **x86_64-windows.toml** or **x86_64-linux.toml** under folder of **conf/buildtools**.

>**Tip:**  
&emsp;&emsp;In actuality, Celer has built-in support for a variety of build tools, such as: Windows versions of CMake, MinGit, strawberry-perl, msys2, vswhere, and more. Although most of these tools are not user-configurable, when switching between different buildsystems, Celer automatically adds them to the buildtools list. For example, when compiling with makefiles on Windows, msys2 is automatically added to the buildtools.

### 1.2.5 library_type

&emsp;&emsp;Optional, default value is **shared**, candidate values are **shared** and **static**, which means dynamic library and static library respectively.

### 1.2.6 build_shared，build_static

&emsp;&emsp;Optional, some old **makefiles** projects do not support compiling dynamic libraries with **--enable-shared**, but with **--with-shared**. To flexibly support this, this configuration entry is reserved. You may be afraid of the configuration here, but fortunately, the default value of **build_shared** depends on the different **buildsystem**, and you only need to override the specified value when needed. The default values of **build_shared** and **build_static** are as follows:

- **cmake**: "-DBUILD_SHARED_LIBS=ON"
- **makefiles**: "--enable-shared"
- **meson**: "--default-library=shared"
- **b2**: "link=shared runtime-link=shared"

**Candidate Values:**

| Value | Description | Example |
|-------|-------------|---------|
| `""` (empty string) | Use build system's default value | `build_shared = ""` → automatically uses `--enable-shared` |
| `"_"` | Explicitly disable this option, no parameters added | `build_shared = "_"` → no shared library parameters added |
| Custom string | Specify a custom configure parameter | `build_shared = "--with-shared"` |
| `"enable\|disable"` | Specify both enable and disable parameters | `build_shared = "--enable-shared\|--disable-shared"` |

>**Note:**  
>**1:** Since most C/C++ libraries don't support explicitly compiling static libraries only, **build_static**'s default value is empty unless manually specified in port.toml.  
>**2:** Some makefiles projects have executable build targets, not libraries. In this case, you should set **build_shared** and **build_static** to **`"_"`** to explicitly disable library compilation options.

When **library_type** is set to **shared**, Celer will try to read the value in **build_shared** as the compilation option parameter, otherwise read the value in **build_static** as the compilation option parameter.

### 1.2.7 c_standard, cxx_standard

&emsp;&emsp;Optional, default is empty, they are used to override the c and c++ standard value that defined in global **celer.toml**.
- c_standard's candicated values：**c90**, **c99**, **c11**, **c17**, **c23**；  
- cxx_standard's candicated values：**c++11**、**c++14**、**c++17**、**c++20**；

### 1.2.8 build_type

&emsp;&emsp;Optional, default is empty, used to specify the build type. When build_type is specified in port.toml, it will override the global build_type setting defined in celer.toml. This is useful for libraries that require a specific build type.
- build_type's candidate values：**release**, **debug**, **relwithdebinfo**, **minsizerel**；
- If not specified, the build_type defined in celer.toml will be used (defaults to **release**)

>**Note:** build_type also affects package cache key calculation. Different build_type values will generate different cache entries.

### 1.2.9 envs

&emsp;&emsp;Optional, you can define some environment variables here, such as **CXXFLAGS=-fPIC**, or even compile some libraries need to set specified environment variables, such as: the **libxext** library needs to set the environment variable: **"xorg_cv_malloc0_returns_null=yes"** when cross-compiling to the aarch64 platform, the purpose is to mask the compiler check error report;  
&emsp;&emsp;In addition, it should be noted that each library's **toml** file supports defining **envs**, but when compiling them, **envs** are completely independent of each other, as each library compilation ends, the **envs** defined in the **toml** file will be cleared from the current process, and when compiling the next library, if the corresponding **toml** file defines new **envs**, then set the new environment variables.

### 1.2.10 patches

&emsp;&emsp;Optional. Some library source codes may contain issues that cause compilation errors. Traditionally, this requires manual source code modification and recompilation. To avoid manual intervention, we can create fix patches for these modifications. You may place multiple patch files (git patch or Linux patch formats supported) in the port's version directory. As this field accepts an array, multiple patches can be defined. Celer will attempt to apply these patches automatically before each configure step.

### 1.2.11 build_in_source

&emsp;&emsp;Optional, a few third-party libraries (e.g., NASM, Boost) require in-source configure and build. Note: This **build_in_source** option primarily serves makefiles projects.   
>Please note that: b2 builds are already encapsulated as a dedicated buildsystem (i.e., buildsystem = "b2").

### 1.2.12 autogen_options

&emsp;&emsp;Optional, a few third-party libraries (e.g., NASM, Boost) require running **./autogen.sh** before configure. This field is used to specify the options to be passed to **./autogen.sh**.

### 1.2.13 dependencies

&emsp;&emsp; Optional, if your third-party library depends on other third-party libraries during compilation, you need to define them here. These libraries will be compiled and installed before the current library. Note that the format is **name@version**, and we must explicitly specify the version of the current library.

### 1.2.14 dev_dependencies

&emsp;&emsp;Optional, similar to **dependencies**, but here the third-party library dependencies are tools required during compilation, such as: many makefiles projects require **autoconf**, **nasm**, etc. tools before configure. Any library defined in **dev_dependencies** will be compiled and installed using the local tooolchain compiler. They will be installed to a specific directory, such as: **installed/x86_64-linux-dev**, and the **installed/x86_64-linux-dev/bin** path will be automatically added to the **PATH** environment variable, enabling access to these tools during compilation.

>Why support **dev_dependencies**:   
>- To avoid manually installing some local tools using **sudo apt install xxx**.  
>- When compiling a third-party library that is a newer version, even if you install these tools using **apt**, you may still encounter errors such as **autoconf** version too low. In this case, you need to manually download the tool source code, compile it locally, and install it to the system directory. This is not only time-consuming but also pollutes the system environment.

### 1.2.15 pre_configure, post_configure, pre_build, fix_build, post_build, pre_install, post_install

&emsp;&emsp;Optional, there are always libraries with problematic code. When compilation fails, we can provide patches to fix the source code. For relatively minor issues like incorrect output filenames, we can add corrective commands in **post_install**. Similarly, if file-related issues occur in other stages, we can apply pre-processing or post-processing adjustments at the corresponding steps. A typical example is the libffi library, which doesn't compile smoothly on Windows—various pre-and post-processing steps are required to make it work.

```
# =============== build for windows ============ #
[[build_configs]]
system_name = "windows"
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

>Celer provides some dynamic variables that can be used in the toml file, such as: **${BUILD_DIR}**, which will be replaced with the actual path during compilation. For more details, please refer to [Dynamic Variables](#3-dynamic-variables).

### 1.2.15 options

&emsp;&emsp;Optional, when compiling third-party libraries, there are often many options that need to be enabled or disabled. We can define them here, such as **-DBUILD_TESTING=OFF**;

## 2. Dynamic Variables

| Variable | Description | Source |
| --- | --- | --- |
| ${SYSTEM_NAME} | Value of **toolchain.system_name** | platform |
| ${HOST} | Value of **toolchain.host** | platform |
| ${SYSTEM_PROCESSOR} | Value of **toolchain.system_processor** | platform |
| ${SYSROOT} | Value of **toolchain.sysroot** | platform |
| ${CROSS_PREFIX} | Value of **toolchain.crosstool_prefix** | platform |
| ${BUILD_DIR} | Path to current library's compile directory in buildtrees | buildtrees |
| ${HOST_NAME} | Value of **toolchain.host_name** | platform |
| ${PACKAGE_DIR} | Path to current library's package directory | port |
| ${BUILDTREES_DIR} | Path to buildtrees root directory in workspace | buildtrees |
| ${REPO_DIR} | Path to current library source code directory | port/buildtrees |
| ${DEPS_DIR} | Path to **tmp/deps** directory | workspace |
| ${DEPS_DEV_DIR} | Path to **tmp/deps/${HOST_NAME}-dev** directory | workspace |
| ${PYTHON3_PATH} | Path to installed python3, auto detected | system |
