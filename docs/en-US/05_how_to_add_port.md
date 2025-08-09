# 如何贡献新的三方库

&emsp;&emsp;三方库的配置文件存储在 `workspace/ports` 目录下，同时ports目录是一个独立repo，专门存储所有三方库的配置文件。在这个文件中，我们描述了三方库从哪里克隆/下载源代码，如何构建，以及当前三方库依赖于哪些其他三方库等。

## 1. 如何托管新的库

```
./celer create --port=glog@v0.6.0

[✔] ======== glog@v0.6.0 is created, please proceed with its refinement. ========
```

>随后，你需要打开生成的文件跟你的目标库的具体编译要求进行配置， 或者手动拷贝别的port.toml并修改也是可以的。

## 2. port.toml 配置详解：

比如：一个cmake三方库的port: `ports/glog/0.6.0/port.toml`:

```
[package]
url                 = "https://github.com/google/glog.git"
ref                 = "v0.6.0"
archive             = ""                    # optional field, it works only when url is not a git url.
src_dir             = "xxx"                 # optional field
supported_hosts     = [...]                 # optional field

[[build_configs]]
pattern             = "*linux*"             # optional field, default is "*"
build_system        = "cmake"               # should be `cmake`, `makefiles`, `b2`, `ninja`, `meson`, etc.
build_tools         = [...]                 # optional field
library_type        = "shared"              # optional field, should be `shared`, `static`, and default is `shared`.
build_shared        = "--with-shared"       # optional field
build_static        = "--with-static"       # optional field
c_standard          = "c99"                 # optional field
cxx_standard        = "cxx17"               # optional field
envs                = [...]                 # optional field
patches             = [...]                 # optional field
build_in_source     = false                 # optional field, default is `false`
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

&emsp;&emsp;在一个port.toml中可供配置的字段看起来比较多, 但是实际上只有几个是必填的, 其他的都是可选的，大部分情况托管一个三方库简单到如下：

```toml
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

**Tips**：

1. **url**: 三方库的git仓库地址, 也可以是https或者ftp地址, 甚至测试期间可以以`file:///`开头指向一个本地仓库；
2. **ref**: 库的tag名字, 也可以是branch名称，甚至是commit id，当库的代码是压缩包形式下载的，ref也可以是压缩包的文件名里的版本号；  
3. **archive**: 可选, 当库的代码是压缩包形式下载的, 可以指定下载后后新的压缩包名字, 比如`boost_1_87_0.tar.gz`;  
4. **src_dir**: 可选，有些库的`configure`文件或者`CMakeLists.txt`文件不在根目录下, 比如`icu`库的`configure`文件在`icu4c/source`目录下，我们可以通过`src_dir`来指定;  
5. **build_config**: 三方库的编译配置很多时候一个三方库在不同系统平台有着不同的编译方式, 区别往往体现在不同系统平台需要不同的编译参数，甚至编译步骤也不一样，甚至不少库在windows上编译需要特种预处理和后处理才能顺利编译。因此，build_config的定义是一个数组，我们可以在这里定义如何在不同系统平台上编译。

## 2.2 build_config

&emsp;&emsp;`build_configs`被设计成一个数组，为的是满足一个库在不同系统平台下不同的编译方式或者差异，`celer`会根据`pattern`自动找到匹配的`build_conig`来组织编译命令并编译。

### 2.2.1 pattern

&emsp;&emsp;有些三方库在不同的平台上需要不同的配置，它是用来匹配在`conf`目录下的`platform`文件的，它的匹配规则类似如下：

- "": 空，也是默认的值，即：编译配置不区分系统平台，切换到任意platform都可以用一套buildconfig来编译；
- "*": 效果等同于"", 编译配置一样不区分系统平台；
- "\*linux\*": 匹配所有linux系统；
- "\*windows\*": 匹配所有windows系统；
- "x86_64-linux*": 匹配所有cpu arch为x86_64，且系统为linux的情况；
- "aarch64-linux*": 匹配所有cpu的arch为aarch64，且系统为linux的情况
- "x86_64-windows*": 匹配所有cpu的arch为x86_64, 且系统为windows的情况；
- "aarch64-windows*": 匹配所有cpu的arch为aarch64, 且系统为windows的情况。

### 2.2.2 build_system

&emsp;&emsp;不同的编译工具在实现交叉编译时，它们的配置差异巨大，Celer为了简化用户的使用，将其都封装抽象为一个个buildsystem的选项，目前选项支持`b2`, `cmake`, `gyp`, `makefiles`, `meson`, `ninja`等， Celer会在未来扩展支持更多的构建工具，比如：bazel, msbuild, scons等。

### 2.2.3 build_tools

&emsp;&emsp;可选，有些库编译需要本地安装额外的工具，比如：ruby、perl，甚至需要通过pip3安装额外的python库，例如：["ruby", "perl", "python3:setuptools"]

>**Tip:**  
实际Celer内置管理的build tool有很多，比如：Window版本的CMake，MinGit，strawberry-perl，msys2，vswhere等，只是这些绝大部分不需要用户关心，当切换不同buildsystem，这些隐藏的工具被被自动加入到了buildtools里了。比如：在windows上选择makefiles编译时候，msys2被自动加入buildtools。

### 2.2.4 library_type

&emsp;&emsp;可选，默认值为`shared`, 候选值为`shared`和`static`，即：动态库和静态库。

### 2.2.5 build_shared，build_static

&emsp;&emsp;可选，有一些古老的`makefiles`项目不支持以`--enable-shared`方式指定编译动态库，而是以`--with-shared`方式指定，为了灵活支持故在此预留配置入口。你可能会害怕这里的配置麻烦，幸运的是`build_shared`的值根据不同的`buildsystem`都有对应的默认值，只要按需覆盖指定即可, 而且需要覆盖的情况非常罕见，它们的默认值如下：

- **cmake**: "-DBUILD_SHARED_LIBS=ON"
- **makefiles**: "--enable-shared"
- **meson**: "--default-library=shared"
- **b2**: "link=shared runtime-link=shared"

同时，build_static对应不同buildsystem的默认值如下：

- **cmake**: "-DBUILD_SHARED_LIBS=OFF"
- **makefiles**: "--enable-static"
- **meson**: "--default-library=static"
- **b2**: "link=static runtime-link=static"

当`library_type`被设置为`shared`, 则读取`build_shared`里的值作为编译选项参数，否则读取`build_static`里的值作为编译选项。

### 2.2.6 c_standard, cxx_standard

&emsp;&emsp;可选，默认为空，它们候选项目分别如下：  
c_standard候选值：`C89`、`C99`、`C11`、`C17`；  
cxx_standard候选值：`C++98`、`C++03`、`C++11`、`C++14`、`C++17`、`C++20`；

### 2.2.7 envs

&emsp;&emsp;可选, 你可以在这里定义一些环境变量, 比如`CXXFLAGS=-fPIC`，甚至编译一些库需要设置指定的环境变量，比如：libxext这个库在交叉编译目标为aarch64平台时候需要指定环境变量：`"xorg_cv_malloc0_returns_null=yes"`，目的是为了屏蔽编译器检查报错的误报；  
&emsp;&emsp;另外，需要注意的是，每个库的`toml`文件里都支持定义`envs`，但在编译它们时候，`envs`是完全独立的不共享的，因为每个库编译结束后`toml`里配置的`envs`会从当前进程中删除，编译下一个库时候如果对应的`toml`里有定义新的`envs`则再设置新的环境变量。

### 2.2.8 patches

&emsp;&emsp;可选, 有些库源码有一些问题会导致编译报错，传统办法是手动修改源码再重新编译，为了能方便一键编译，我们可以针对源码的修改创建修复的patch，你可以在port的版本目录下创建多个patch文件，支持git patch, 也支持linux patch, 这里类型是一个数组，因此允许定义多个patch文件，每次configure前Celer会先尝试应用这些patch。

### 2.2.9 build_in_source

&emsp;&emsp;可选，绝大部分库编译可以指定将编译产生的缓存文件放在源码目录之外，但依然有少部分必须在源码目录里进行configure和build，比如：nasm，boost。不过，这里的`build_in_source`基本只为`makfiles`项目服务，b2的编译已经被封装成了一个独立的buildsystem，即: buildsystem = "b2"。

### 2.2.10 autogen_options

&emsp;&emsp;可选，有比较少的makefile编译的三方库在执行`./autogen.sh`时候需要同时指定一些参数，但目前非常少见，这里只是预留了一个位置。

### 2.2.11 dependencies

&emsp;&emsp;可选，如果你的三方库编译期间依赖其他三方库, 你需要在这里定义它们, 然后它们会在当前三方库之前被编译和安装。注意，依赖的格式是`name@version`, 我们必须精确指定当前三方库应该使用哪个版本。

### 2.2.12 dev_dependencies

&emsp;&emsp;可选，同dependencies类似，只是这里的三方库依赖是编译期间需要的工具，比如： 很多makefiles编译的项目在configure之前需要提供autoconf, nasm等工具，凡是在`dev_dependencies`里配置的库, 默认就会用本地的x86_64的编译器编译它们，它们会被安装到特定的目录下，例如：`installed/x86_64-linux-dev`，然后内部的`bin`路径会被自动加入到环境变量`PATH`里，随后编译库期间就能访问到这些工具。

**`dev_dependencies`设计出来的原因是：**

1. 为了避免以 `sudo apt install xxx`方式时不时手动安装一些本地工具；
2. 当编译目标库是较新版本，当你的当前系统版本较低（比如: ubuntu-20.04），即便通过apt安装了这些工具，但依然不能编译，会提示`autoconf`等工具版本太低，最后还是需要用户下载工具源码本地编译并安装到系统目录，不光繁琐，还会把系统环境污染；

因此，celer可以做到一键无中断地编译出目标库或App；

### 2.2.13 pre_configure, post_configure, pre_build, fix_build, post_build, pre_install, post_install

&emsp;&emsp;可选，总是有些库的代码有些问题，遇到代码编译不通过的情况我们可以通过提供patch修复源码，对于编译产物文件名不对这类相对小的问题，我们可以通过在`post_install`里加执行命令修复。类似的如果在别的环节有文件问题，我们可以在对应的环节加预处理和后处理进行矫正，比较典型的是libffi库在windows编译不顺畅，通过各种预处理和后处理得以顺利编译：

```toml
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

>celer提供了一些动态变量，可以在toml文件里使用，比如：`${BUILD_DIR}`, 在编译期间它们会被替换为实际的路径，详细参考[动态变量](#3-动态变量)。

### 2.2.14 options

&emsp;&emsp;编译三方库的时候, 总是有很多的选项需要开启或者关闭, 我们可以在这里定义它们, 比如`-DBUILD_TESTING=OFF`；

## 3. 动态变量

- **${SYSTEM_NAME}**: 它的值来自platform文件中的`toolchain.system_name`定义;
- **${HOST}**: 它的值来自platform文件中的`toolchain.host`定义;
- **${SYSTEM_PROCESSOR}**: 它的值来自platform文件中的`toolchain.system_processor`定义;
- **${SYSROOT}**: 它的值来自platform文件中的`toolchain.sysroot`定义;
- **${CROSS_PREFIX}**: 它的值来自platform文件中的`toolchain.crosstool_prefix`定义;
- **${BUILD_DIR}**: 它指向当前库在buildtrees目录里的编译目录，如：`buildtrees\x264@stable\x86_64-windows-test_project_02-release`;
- **${PACKAGE_DIR}**: 它指向当前库在packages目录里的独立目录，如：`packages\x264@stable@x86_64-windows@test_project_02@release`;
- **${BUILDTREES_DIR}**: 它指向workspace下buildtrees根目录，即：`buildtrees`;
- **${REPO_DIR}**: 它指向当前库源码clone后所在目录，如：`buildtrees\x264@stable\src`;
- **${DEPS_DIR}**: 它指向`tmp/deps`目录，它是编译期间依赖库寻找的目录;
- **${DEPS_DEV_DIR}**: 它指向 `tmp/deps/\${HOST_NAME}-dev`目录，即编译期间依赖工具所在目录，如：`tmp/deps/x86_64-linux-dev`;
- **${PYTHON3_PATH}**: 它指向本地安装的python3所在路径，无需手动指定，它由celer自动识别到的; 
