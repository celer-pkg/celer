# 平台配置管理

&emsp;&emsp;平台的配置文件存储在 `workspace/conf/platforms` 目录下，这个文件定义了这个平台所需的工具链、根文件系统。

## 1. 创建新的平台配置

```
$ ./celer create --platform=x86_64-linux-22.04

[✔] ======== x86_64-linux-22.04 is created, please proceed with its refinement. ========
```

>随后，你需要打开生成的文件跟你的目标环境进行配置。

## 2. platform.toml 配置详解：

比如: `conf/platforms/x86_64-linux-20.04.toml`:

```toml
[rootfs]
url = "https://github.com/celer-pkg/test-conf/releases/download/resource/ubuntu-base-20.04.5-base-amd64.tar.gz"
name = "gcc"
version = "9.5"
path = "ubuntu-base-20.04.5-base-amd64"
pkg_config_path = [
    "usr/lib/x86_64-linux-gnu/pkgconfig",
    "usr/share/pkgconfig",
    "usr/lib/pkgconfig"
]

[toolchain]
url = "https://github.com/celer-pkg/test-conf/releases/download/resource/gcc-9.5.0.tar.gz"
path = "gcc-9.5.0/bin"
system_name = "Linux"
system_processor = "x86_64"
host = "x86_64-linux-gnu"
crosstool_prefix = "x86_64-linux-gnu-"
cc = "x86_64-linux-gnu-gcc"
cxx = "x86_64-linux-gnu-g++"
fc = "x86_64-linux-gnu-gfortran"            # optional field
ranlib = "x86_64-linux-gnu-ranlib"          # optional field
ar = "x86_64-linux-gnu-ar"                  # optional field
nm = "x86_64-linux-gnu-nm"                  # optional field
objdump = "x86_64-linux-gnu-objdump"        # optional field
strip = "x86_64-linux-gnu-strip"            # optional field
```

**Tips:**

- url: 它可以是http、https或ftp的url，celer会下载它。也可以是本地文件路径，并且应该以`file:///`开头，例如：`file:////home/phil/buildresource/ubuntu-base-20.04.5/gcc-9.5.0`。
- path: 它是指向toolchain目录内可执行文件所在路径，在celer运行期间会加入到环境path中，在生成toolchain_file.cmake里也会被加入到$ENV{PATH}里，便于编译期间能访问到里面的可执行文件；
- fc，ranlib，ar，nm，objdump，strip等: 它们是是非必填字段，如果crosstool_prefix指定了，通常会自动寻找到对应的工具。

### 2.1 Windows平台的配置

&emsp;&emsp;Windows下的MSVC配置跟Linux的GCC有很大区别，区别在于MSVC内的编译器文件名基本是固定的，但内置的头文件和库文件散落在不同目录下，好在都是固定的一些位置，celer把所有的细节都封装起来了，最后配置MSVC的platform配置反而更简单了，例如：

```toml
[toolchain]
url = "file:///C:/Program Files/Microsoft Visual Studio/2022/Community"
name = "msvc"
version = "14.44.35207"
system_name = "Windows"
system_processor = "x86_64"
```

## 3. platform 配置切换

&emsp;&emsp;切换了platform意味着切换了项目的编译环境，如aarch64-linux编译环境切换到了x86_64-linux编译环境。

执行如下命令进行切换platform：

```
$ ./celer configure --platform=x86_64-linux-20.04

[✔] ======== current platform: x86_64-linux-20.04. ========
```

>**Tip:**  
&emsp;&emsp;celer支持在不配置platform的情况下也能编译，其实就是利用本地已经安装的编译器来编译的，在linux下默认会寻找`/usr/bin`下的gcc和g++，在windows下默认会通过`vswhere`寻找已经安装的`msvc`;  
&emsp;&emsp;可能很多c++开发者都有一个误区：开发x86_64-linux平台的软件时候可以直接用本地的`gcc/g++`，然后直接访问`/usr/include`下的头文件，然后依赖什么库就直接`sudo apt install xxx`, 方便又快捷，比Windows下开发C++不要方便太多。殊不知，在软件开发好之后发布时候就愁了，可执行文件依赖的库散落在各处，需要用户提前安装一堆库才能把软件运行起来，在遇到对方系统版本不一致，很可能即便通过apt安装了指定的库也未必版本匹配。  
&emsp;&emsp;因此，开发linux软件极其推荐用独立的绿色版的gcc、rootfs，并且屏蔽访问系统`/usr`下的库，一切项目里依赖的库就交给celer来托管和编译吧。