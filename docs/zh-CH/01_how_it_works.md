# 1. Celer的工作原理

&emsp;&emsp;不同的C++项目往往意味着可能需要不同的编译环境和依赖库，Celer推荐定义conf作为不同项目的项目的编译环境和编译选项的配置文件，conf的组成结构如下：

```
├── platforms
│   ├── aarch64-linux-gnu-gcc-9.2.toml
│   ├── x86_64-linux-ubuntu-20.04.toml
│   ├── x86_64-linux-ubuntu-22.04.toml
│   └── x86_64-windows-msvc-14.44.toml
├── projects
│   ├── test_project_01 --------------- first project's dependencies
│   │   ├── boost --------------------- override default options
│   │   │   └── 1.87.0
│   │   │       └── port.toml
│   │   └── sqlite3 ------------------- override default options
│   │       └── 3.49.0
│   │           └── port.toml
│   ├── test_project_01.toml ---------- first project
│   ├── test_project_02 --------------- second project's dependencies
│   │   ├── ffmpeg -------------------- override default options
│   │   │   └── 5.1.6
│   │   │       └── port.toml
│   │   ├── lib_001 ------------------- second project's private library
│   │   │   └── port.toml
│   │   └── lib_002 ------------------- second project's private library
│   │       └── port.toml
│   └── test_project_02.toml ---------- second project
└── README.md
```

- 平台定义（platforms/*.toml）：用于定义toolchain和rootfs，在Celer运行期间会读取并解析它们，里面的每一项配置都会体现在后面编译库的过程中；
- 项目配置（projects/*.toml）：用于定义项目的依赖库列表，CMake全局变量，C++全局宏，C++全局编译选项，全局环境变量等，当执行deploy命令时会编译配置的依赖库，同时生成**toolchain_file.cmake**文件，所有的C++全局CMake变量、C++全局宏等都会定义在其中；
- 库定制化（projects/*/port.toml）：不同的项目往往依赖库的不同版本，编译参数也需要定制，甚至还包含项目范围的私有库。

&emsp;&emsp;其实，当conf缺失时，Celer依然可以工作，只不过Celer会调用本地已安装的toolchain进行编译：

- 在Windows系统下，Celer会通过vswhere寻找系统里安装的VisualStudio作为toolchain，当目标库是makefile编译的，还会自动下载msys2并配置；
- 在Linux系统下，Celer则会直接找本地安装的x86_64位的gcc和g++。

## 2. 一键部署

当执行`./celer deploy`后，Celer会做如下工作：

- 检查并修复当前选择的平台的工具链，根文件系统和其他工具。如果缺失，Celer将下载它们并配置；
- 检查当前选择的项目是否安装了第三方库。如果缺失，Celer将克隆它们的源代码，然后编译，同时包含它们的子依赖项。

Celer会根据conf目录下的配置文件，生成**toolchain_file.cmake**文件，这个文件可以直接用于cmake项目的编译。

## 3. 关于和使用

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

1. Celer在deploy执行成功后也就意味着生成了**toolchain_file.cmake**，同时意味着项目开发可以选择仅仅依赖此**toolchain_file.cmake**即可。  
2. 虽然Celer还可以用来编译你的工程项目，但这不是是强制的，这也意味着当与他人合作开发，你可以将workspace下的downloaded（内部压缩包也可以删除）、installed、toolchain_file.cmake打包即可交付对方。