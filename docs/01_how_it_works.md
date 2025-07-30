# 1. C.

celer的workspace如下:

```
├── celer.toml
├── conf
│   ├── platforms
│   │   ├── aarch64-linux-jetson.toml
│   │   ├── aarch64-linux-raspberry.toml
│   │   ├── x86_64-linux-20.04.toml
│   │   └── x86_64-linux-22.04.toml
│   └─── projects
│       ├── project_001.toml
│       ├── project_001
│       │            ├─ ffmpeg
│       │            │    └─ 5.1.6
│       │            │       └─ port.toml
│       │            └─ opencv
│       │                └─ 4.1.2
│       │                    └─ port.toml
│       ├── project_002.toml
│       └── project_003.toml
├── downloads
│   ├── cmake-3.30.5-linux-x86_64.tar.gz
│   ├── gcc-9.5.0.tar.gz
│   ├── nasm-2.16.03.tar.gz
│   ├── ubuntu-base-20.04.5-base-amd64.tar.gz
│   └── tools
│        ├── cmake-3.30.5-linux-x86_64
│        ├── gcc-9.5.0
│        ├── nasm-2.16.03
│        └── ubuntu-base-20.04.5-base-amd64
├── installed
│   ├── celer
│   │   ├── hash
│   │   │   ├── ffmpeg@3.4.13@x86_64-linux-20.04@Release.hash
│   │   │   ├── opencv@3.4.18@x86_64-linux-20.04@Release.hash
│   │   │   ├── x264@stable@x86_64-linux-20.04@Release.hash
│   │   │   ├── x265@4.0@x86_64-linux-20.04@Release.hash
│   │   │   └── zlib@1.3.1-x86_64-linux-20.04@Release.hash
│   │   └── info
│   │       ├── ffmpeg@3.4.13@x86_64-linux-20.04@Release.list
│   │       ├── opencv@3.4.18@x86_64-linux-20.04@Release.list
│   │       ├── x264@stable@x86_64-linux-20.04@Release.list
│   │       ├── x265@4.0@x86_64-linux-20.04@Release.list
│   │       └── zlib@1.3.1-x86_64-linux-20.04@Release.list
│   └── x86_64-linux-20.04@Release
│       ├── bin
│       ├── include
│       ├── lib
│       └── share
├── packages
│   ├── ffmpeg@3.4.13@x86_64-linux-20.04@Release
│   │   ├── include
│   │   ├── lib
│   │   └── share
│   ├── opencv@3.4.18@x86_64-linux-20.04@Release
│   │   ├── bin
│   │   ├── include
│   │   ├── lib
│   │   └── share
│   ├── x264@stable@x86_64-linux-20.04@Release
│   │   ├── include
│   │   └── lib
│   └── x265@4.0@x86_64-linux-20.04@Release
│       ├── include
│       ├── lib
│       └── share
├── ports
│    ├── ffmpeg
│    │      ├── 3.4.13
│    │      │   └── port.toml
│    │      └─── 5.1.6
│    │           └── port.toml
│    ├── opencv
│    ├── x265
│    └── ...
└── toolchain_file.cmake
```

主要由**celer.toml**，**conf**，**downloads**，**installed**，**packages**和**toolchain_file.cmake**等组成，详细成员介绍如下：

1. celer.toml: 这是celer的全局配置文件，用于定义`conf repo`，`current platform`，`current project`和`cache_dir`等，你可以通过celer的cli或者手动修改它。

2. conf: 如果需要通过celer进行交叉编译，以及工程化管理项目范围内的依赖，可以利用conf，我们推荐用一个仓库来管理此处的配置，所有可用的`platform`，`projects`，以及需要定制化的port都在其中定义。
    - platform: 这个文件夹包含所有可用的平台配置文件，每个文件定义了工具链和根文件系统。
    - projects: 这个文件夹包含所有可用的项目配置文件，每个文件定义了第三方库，全局CMake变量，环境变量，编译选项以及c++宏。

3. downloads: 所以编译资源以及三方库的压缩包都会下载到此目录下。
4. installed: 所有第三方库都会被编译和安装到这个目录下，并且会按照平台特定的子目录进行存储，例如`x86_64-linux-20.04-release`。
5. packages: 这是一个所有库编译安装的过渡目录，同时也是第三方库缓存包的检索和提取的目录。
6. ports: 这个文件夹包含所有可用的第三方库，每个库都有自己的配置文件，定义了库的版本，编译选项和依赖关系。
7. toolchain_file.cmake: 这是cmake进行交叉编译的配置文件，这也是celer执行deploy命令后生成的文件。

## 3. 一键部署

当执行`./celer --deploy`后，celer会做如下工作：

- 检查并修复当前选择的平台的工具链，根文件系统和其他工具。如果缺失，celer将下载它们并在进程中设置环境变量；
- 检查当前选择的项目是否安装了第三方库。如果缺失，celer将克隆它们的源代码，然后配置，构建和安装，甚至它们的子依赖项。

> **Tips:**
>
>实际上，`cmake configure`命令可以自动执行`./celer --setup`命令，从而实现一键部署。

## 4. 关于和使用

./celer about 打印如下：

```
Welcome to celer (v1.0.0).
---------------------------------------
This is a simple pkg-manager for C/C++.

How to use it to build cmake project: 
option1: set(CMAKE_TOOLCHAIN_FILE "/home/phil/celer/toolchain_file.cmake")
option2: cmake .. -DCMAKE_TOOLCHAIN_FILE=/home/phil/celer/toolchain_file.cmake

[ctrl+c/q -> quit]
```

**Tips:**

1. celer在deploy执行成功后也就意味着生成了toolchain_file.cmake, 同时意味着项目开发可以选择仅仅依赖此toolchain_file.cmake即可。  
2. 虽然celer还可以用来编译你的工程项目，但这不是是强制的，这也意味着当与他人合作开发，你可以将workspace下的downloaded（内部压缩包也可以删除）、installed、toolchain_file.cmake打包即可交付对方。