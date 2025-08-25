# Install 命令

&emsp;&emsp;**Install**命令下载、编译和安装软件包，同时解决依赖关系。它支持多个构建配置和安装模式。

## 命令语法

```shell
celer install [package_name] [flags]  
```

## 命令选项

| Option	        | Short flag | Description                                              |
| ----------------- | ---------- | ---------------------------------------------------------|
| --build-type	    | -b	     | Specify build type, default is release.                  |
| --dev             | -d         | Install as dev runtime dependency.                       |
| --force	        | -f	     | Uninstall and install again.                             |
| --recurse	        | -r	     | Combine with --force, recursively reinstall dependencies.|

## 命令示例

**1. 标准安装**

```shell
celer install ffmpeg@5.1.6
```

**2. 安装为开发时依赖项**

```shell
celer install pkgconf@2.4.3 --dev  
```

**3. 强制安装**

```shell
celer install ffmpeg@5.1.6 --force/-f
```
>移除已安装的软件包并重新配置、构建和安装。

**4. 强制重新安装，包含依赖项**

```shell
celer install ffmpeg@5.1.6 --force/-f --recurse/-r
```

## 安装目录结构

```
└─ installed
    ├── celer
    │   ├── hash
    │   │   ├── nasm@2.16.03@x86_64-windows-dev.hash
    │   │   └── x264@stable@x86_64-windows-msvc-14.44@test_project_02@release.hash
    │   └── info
    │       ├── nasm@2.16.03@x86_64-windows-dev.trace
    │       └── x264@stable@x86_64-windows-msvc-14.44@test_project_02@release.trace
    ├── x86_64-windows-dev
    │   ├── LICENSE
    │   └── bin
    │       ├── nasm.exe
    │       └── ndisasm.exe
    └── x86_64-windows-msvc-14.44@test_project_02@release
        ├── bin
        │   ├── libx264-164.dll
        │   └── x264.exe
        ├── include
        │   ├── x264.h
        │   └── x264_config.h
        └── lib
            ├── cmake
            │   └── x264
            │       ├── x264ConfigVersion.cmake
            │       ├── x264Targets-release.cmake
            │       ├── x264Targets.cmake
            │       └── x264config.cmake
            ├── libx264.lib
            └── pkgconfig
                └── x264.pc
```

**1. installed/celer/hash**  

&emsp;&emsp;存储每个库在此目录下的哈希键值。当 **celer.toml** 中配置了 `cache_dir` 时，该哈希值将与构建产物一起以键值对形式存储。如果后续编译时在缓存中发现匹配的哈希值，将直接复用对应的构建产物，以避免不必要的重复编译。 

**2. installed/celer/info** 

&emsp;&emsp;存储每个库在此目录下的安装文件清单。此文件是判断库是否已安装的主要凭证，也是实现移除已安装库的基础。  

**3. installed/x86_64-windows-dev** 

&emsp;&emsp;许多第三方库在编译时需要额外工具（例如x264需要NASM）。Celer通过将这类工具安装至该目录来管理相关依赖，同时会自动将installed/x86_64-windows-dev/bin路径添加至PATH环境变量。在Linux系统下，Celer还会从源码编译安装autoconf、automake、m4、libtool和gettext等工具至此目录。

**4. installed/x86_64-windows-msvc-14.44@test_project_02@release** 

&emsp;&emsp;所有第三方库的编译产物将存储在此目录下。在 **toolchain_file.cmake** 中，`CMAKE_PREFIX_PATH` 会被设置为该目录，以便CMake能够在该目录下找到第三方库。
