# 如何安装第三方库

**./celer install name@version**: celer将克隆库的代码，然后配置，构建和安装它。如果当前库有子依赖，则子依赖项将被提前克隆，配置，构建和安装。
最终，所有第三方都将安装到`installed`文件夹中，每个第三方也都有一个单独的包在`packages`文件夹中，独立的package包便于直观的查看当前的库编译install产物有哪些。

installed内的目录结构如下：

```
- installed
    ├── celer
    │   ├── hash
    │   │   ├── nasm@2.16.03@x86_64-windows-dev.hash
    │   │   └── x264@stable@x86_64-windows-msvc-14.44@test_project_02@release.hash
    │   └── info
    │       ├── nasm@2.16.03@x86_64-windows-dev.list
    │       └── x264@stable@x86_64-windows-msvc-14.44@test_project_02@release.list
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

1. installed/celer/hash：存放每个库的hash key，实际内容为当前库的编译环境以及子依赖的描述，当配置了cache_dir, 此hash将和库的编译产物作为key-value进行存储，后续如果新发起的编译发现hash能在存储的hash里匹配得上，则直接取对应了编译产物，以此避免重复编译；  
2. installed/celer/info：存放每个库的install文件清单，此文件是用于判断库是否已经安装的主要凭证，同时也是实现已安装库删除的依据；  
3. installed/x86_64-windows-dev: 很多三方库在编译器需要提供一些工具，比如: 编译x264默认需要nasm，因此celer会管理这类工具，并将它们install到此目录下，如果是在linux，还会将autoconf, automake, m4, libtool, gettext等以源码编译安装到此类目录, 此目录会被celer加入到环境变量PATH中，在生成的toolchain_file.cmake里也会将其路径添加到$ENV{PATH}；  
4. installed/x86_64-windows-msvc-14.44@test_project_02@release: 此类目录是编译安装的最终目录，所有三方库的编译产物都会存入此目录，在toolchain_file.cmake里也会通过CMAKE_PREFIX_PATH指向这里寻找三方库。