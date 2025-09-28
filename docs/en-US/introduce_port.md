# Introduce port

&emsp;&emsp;Celer use a git repo to manager configure files of third-party libraries. This repository is continuously expanding and aims to support an increasing number of C/C++ third-party libraries.

## 1. Introduction to port.toml

Let's take a look at the example port.toml file: **ports/glog/0.6.0/port.toml**:

```
[package]
url                 = "https://github.com/google/glog.git"
ref                 = "v0.6.0"
archive             = ""                    # optional field, it works only when url is not a git url.
src_dir             = "xxx"                 # optional field
supported_hosts     = [...]                 # optional field

[[build_configs]]
pattern             = "*linux*"             # optional field, default is "*"
build_system        = "cmake"               # should be **cmake**, **makefiles**, **b2**, **meson**, etc.
build_tools         = [...]                 # optional field
library_type        = "shared"              # optional field, should be **shared**, **static**, and default is **shared**.
build_shared        = "--with-shared"       # optional field
build_static        = "--with-static"       # optional field
c_standard          = "c99"                 # optional field
cxx_standard        = "cxx17"               # optional field
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

&emsp;&emsp;In a port.toml, there are many fields that can be configured, but actually only a few are required, and the rest are optional. Most of the time, it is simple to manage a third-party library as follows:

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
| url | The url to clone or download library code, it can be https or ftp, even it can be **file:///** pointing to a local dir, even during testing, it can be **file:///** pointing to a local repo. |
| ref | It can be a tag name, branch name, or commit id, and it can also be the version number in the filename of the compressed package when the library code is downloaded in the compressed package form. |
| archive | Optional, it works only when url is not a git url. We can rename the downloaded archive file name with this field. |
| src_dir | Optional, some libraries' **configure** file or **CMakeLists.txt** file is not in the root directory, for example, the **configure** file of **icu** library is in **icu4c/source** directory, we can use **src_dir** to specify. |
| build_configs | It's a array of config to descript how to build library in different platforms. |

## 1.2 build_configs

&emsp;&emsp;**build_configs** is designed as an array to meet the different compilation requirements of a library on different system platforms. Celer will automatically find the matching **build_config** according to **pattern** to assemble the compilation command.  
&emsp;&emsp;The build configuration of third-party libraries often varies across different systems. These differences typically involve platform-specific compilation flags or even entirely distinct build steps. Some libraries even require special pre-processing or post-processing to compile correctly on Windows. 

### 1.2.1 pattern

&emsp;&emsp;Some third-party libraries require different configurations on different platforms. It is used to match the **platform** file in the **conf** directory. Its matching rules are similar to the following:

| Pattern | Description |
| --- | --- |
| * | Empty, also the default value, which means that the compilation configuration does not distinguish between system platforms. Switching to any platform can use the same buildconfig to compile |
| *linux* | Match all linux systems |
| *windows*" | Match all windows systems |
| x86_64‑linux* | Match all cpu arch is x86_64, and system is linux |
| aarch64‑linux* | Match all cpu arch is aarch64, and system is linux |
| x86_64‑windows* | Match all cpu arch is x86_64, and system is windows; |
| aarch64‑windows* | Match all cpu arch is aarch64, and system is windows; |

### 1.2.2 build_system

&emsp;&emsp;Different build tools vary significantly in their cross-compilation configurations. To simplify usage, Celer abstracts them into unified buildsystem options, currently supporting **b2**, **cmake**, **gyp**, **makefiles** and **meson**. Future versions will extend support to more tools like **bazel**, **msbuild**, and **scons**.

### 1.2.3 build_tools

&emsp;&emsp;Optional, some libraries require local installation of additional tools, such as: ruby, perl, or even additional python libraries via pip3, e.g., ["ruby", "perl", "python3:setuptools"].

>**Tip:**  
&emsp;&emsp;In actuality, Celer has built-in support for a variety of build tools, such as: Windows versions of CMake, MinGit, strawberry-perl, msys2, vswhere, and more. Although most of these tools are not user-configurable, when switching between different buildsystems, Celer automatically adds them to the buildtools list. For example, when compiling with makefiles on Windows, msys2 is automatically added to the buildtools.

### 1.2.4 library_type

&emsp;&emsp;Optional, default value is **shared**, candidate values are **shared** and **static**, which means dynamic library and static library respectively.

### 1.2.5 build_shared，build_static

&emsp;&emsp;Optional, some old **makefiles** projects do not support compiling dynamic libraries with **--enable-shared**, but with **--with-shared**. To flexibly support this, this configuration entry is reserved. You may be afraid of the configuration here, but fortunately, the default value of **build_shared** depends on the different **buildsystem**, and you only need to override the specified value when needed. The default values of **build_shared** and **build_static** are as follows:

- **cmake**: "-DBUILD_SHARED_LIBS=ON"
- **makefiles**: "--enable-shared"
- **meson**: "--default-library=shared"
- **b2**: "link=shared runtime-link=shared"

>**Note**:  
>
>**1.** Since most C/C++ libraries don't support explicitly compiling static libraries only, **build_static**'s default value is empty unless manually specified in **port.toml**.  
>
>**2.** Some makefiles project's build target is an execuable, not library. In this case, you can set **build_shared** and **build_static** to **no** to disable compiling dynamic library and static library respectively.

When **library_type** is set to **shared**, try to read the value in **build_shared** as the compilation option parameter, otherwise read the value in **build_static** as the compilation option parameter.

### 1.2.6 c_standard, cxx_standard

&emsp;&emsp;Optional, default is empty, they are used to specify the c and c++ standard respectively.
- c_standard's candicated values：**c90**, **c99**, **c11**, **c17**, **c23**；  
- cxx_standard's candicated values：**c++11**、**c++14**、**c++17**、**c++20**；

### 1.2.7 envs
&emsp;&emsp;Optional, you can define some environment variables here, such as **CXXFLAGS=-fPIC**, or even compile some libraries need to set specified environment variables, such as: the **libxext** library needs to set the environment variable: **"xorg_cv_malloc0_returns_null=yes"** when cross-compiling to the aarch64 platform, the purpose is to mask the compiler check error report;  
&emsp;&emsp;In addition, it should be noted that each library's **toml** file supports defining **envs**, but when compiling them, **envs** are completely independent of each other, as each library compilation ends, the **envs** defined in the **toml** file will be cleared from the current process, and when compiling the next library, if the corresponding **toml** file defines new **envs**, then set the new environment variables.

### 1.2.8 patches

&emsp;&emsp;Optional. Some library source codes may contain issues that cause compilation errors. Traditionally, this requires manual source code modification and recompilation. To avoid manual intervention, we can create fix patches for these modifications. You may place multiple patch files (git patch or Linux patch formats supported) in the port's version directory. As this field accepts an array, multiple patches can be defined. Celer will attempt to apply these patches automatically before each configure step.

### 1.2.9 build_in_source

&emsp;&emsp;Optional, a few third-party libraries (e.g., NASM, Boost) require in-source configure and build. Note: This **build_in_source** option primarily serves makefiles projects.   
>Please note that: b2 builds are already encapsulated as a dedicated buildsystem (i.e., buildsystem = "b2").

### 1.2.10 autogen_options

&emsp;&emsp;Optional, a few third-party libraries (e.g., NASM, Boost) require running **./autogen.sh** before configure. This field is used to specify the options to be passed to **./autogen.sh**.

### 1.2.11 dependencies

&emsp;&emsp; Optional, if your third-party library depends on other third-party libraries during compilation, you need to define them here. These libraries will be compiled and installed before the current library. Note that the format is **name@version**, and we must explicitly specify the version of the current library.

### 1.2.12 dev_dependencies
&emsp;&emsp;Optional, similar to **dependencies**, but here the third-party library dependencies are tools required during compilation, such as: many makefiles projects require **autoconf**, **nasm**, etc. tools before configure. Any library defined in **dev_dependencies** will be compiled and installed using the local tooolchain compiler. They will be installed to a specific directory, such as: **installed/x86_64-linux-dev**, and the **installed/x86_64-linux-dev/bin** path will be automatically added to the **PATH** environment variable, enabling access to these tools during compilation.

>Why support **dev_dependencies**:   
>- To avoid manually installing some local tools using **sudo apt install xxx**.  
>- When compiling a third-party library that is a newer version, even if you install these tools using **apt**, you may still encounter errors such as **autoconf** version too low. In this case, you need to manually download the tool source code, compile it locally, and install it to the system directory. This is not only time-consuming but also pollutes the system environment.

### 1.2.13 pre_configure, post_configure, pre_build, fix_build, post_build, pre_install, post_install

&emsp;&emsp;Optional, there are always libraries with problematic code. When compilation fails, we can provide patches to fix the source code. For relatively minor issues like incorrect output filenames, we can add corrective commands in **post_install**. Similarly, if file-related issues occur in other stages, we can apply pre-processing or post-processing adjustments at the corresponding steps. A typical example is the libffi library, which doesn't compile smoothly on Windows—various pre-and post-processing steps are required to make it work.

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

>Celer provides some dynamic variables that can be used in the toml file, such as: **${BUILD_DIR}**, which will be replaced with the actual path during compilation. For more details, please refer to [Dynamic Variables](#3-dynamic-variables).

### 1.2.14 options

&emsp;&emsp;Optional, when compiling third-party libraries, there are often many options that need to be enabled or disabled. We can define them here, such as **-DBUILD_TESTING=OFF**;

## 2. Dynamic Variables

| Variable | Description |
| --- | --- |
| ${SYSTEM_NAME} | Its value is defined in the **toolchain.system_name** section of the platform file. |
| ${HOST} | Its value is defined in the **toolchain.host** section of the platform file. |
| ${SYSTEM_PROCESSOR} | Its value is defined in the **toolchain.system_processor** section of the platform file. |
| ${SYSROOT} | Its value is defined in the **toolchain.sysroot** section of the platform file. |
| ${CROSS_PREFIX} | Its value is defined in the **toolchain.crosstool_prefix** section of the platform file. |
| ${BUILD_DIR} | Its value points to the current library's compile directory in the buildtrees directory, such as: **buildtrees\x264@stable\x86_64-windows-test_project_02-release**. |
| ${HOST_NAME} | Its value is defined in the **toolchain.host_name** section of the platform file. |
| ${PACKAGE_DIR} | Its value points to the current library's independent directory in the packages directory, such as: **packages\x264@stable@x86_64-windows@test_project_02@release**. |
| ${BUILDTREES_DIR} | Its value points to the buildtrees root directory in the workspace. |
| ${REPO_DIR} | Its value points to the current library source code clone directory, such as: **buildtrees\x264@stable\src**. |
| ${DEPS_DIR} | Its value points to the **tmp/deps** directory, which is the directory where compiled dependencies are searched for. |
| ${DEPS_DEV_DIR} | Its value points to the **tmp/deps/\${HOST_NAME}-dev** directory, which is the directory where compiled dependency tools are located, such as: **tmp/deps/x86_64-linux-dev**. |
| ${PYTHON3_PATH} | Its value points to the local installed python3 path, no need to manually specify, it is automatically recognized by Celer. |
