# 1. How Celer works

&emsp;&emsp;Different C++ projects often require distinct build environments and dependencies. Celer recommends using conf to define build environments and third-party dependencies for each project. The conf structure consists of:

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

- **Platform Definitions (platforms/*.toml):** Defines toolchains and rootfs. Celer reads and parses these during execution, with every configuration item affecting subsequent library compilation.

- **Project Configuration (projects/*.toml):** Defines project dependencies, CMake variables, C++ macros, and build settings - the `deploy` command compiles these dependencies and generates **toolchain_file.cmake** containing all global configurations.

- **Library Customization (projects/*/port.toml):** Handles project-specific library versions, custom build parameters, and private libraries scoped to individual projects.

&emsp;&emsp;If the conf file is missing, Celer can still work with locally installed toolchains:

- In Windows, Celer locates installed Visual Studio via vswhere as the default toolchain. For makefile-based libraries, it automatically downloads and configures MSYS2.

- In Linux, Celer automatically uses locally installed x86_64 gcc/g++ toolchains.

## 2. One-click deployment

When executing `./celer deploy`, Celer performs the following operations:

- Verifies and repairs the toolchain, root filesystem, and other tools for the selected platform. If missing, Celer automatically downloads and configures them.
- Checks whether third-party libraries are installed for the selected project. If missing, Celer automatically clones their source code, compiles them, and handles their sub-dependencies.

>Celer can generate **toolchain_file.cmake** with selected **platform** and **project** that configured in **celer.toml**. Then set `-DCMAKE_TOOLCHAIN_FILE` with the path of **toolchain_file.cmake** in your CMake project, you can now develop CMake project.  

## 3. About and usage

`./celer about` print message as below：

```
Welcome to celer (v1.0.0).
---------------------------------------
This is a lightweight pkg-manager for C/C++.

How to use it to build cmake project: 
option1: set(CMAKE_TOOLCHAIN_FILE "/home/phil/workspace/toolchain_file.cmake")
option2: cmake .. -DCMAKE_TOOLCHAIN_FILE=/home/phil/workspace/toolchain_file.cmake
```

&emsp;&emsp;A successful celer deploy generates toolchain_file.cmake, allowing projects to depend solely on this file - making Celer no longer required thereafter. Furthermore, you can pack workspace with **installed folder**, **downloaded folder** and **toolchain_file.cmake** inside, this can be the build environment with others.  
&emsp;&emsp;This highlights a key distinction between Celer and other package managers: Celer is explicitly positioned as a non-intrusive CMake assistant for any CMake projects.