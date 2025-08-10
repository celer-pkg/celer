# How to install library

&emsp;&emsp;Celer will clone the library code, then configure, build and install it. If the current library has sub-dependencies, the sub-dependencies will be cloned, configured, built and installed in advance.  
&emsp;&emsp;After that, all third-party libraries will be installed to the `installed` folder, and each third-party library also has a separate package in the `packages` folder. The independent package package is convenient for intuitively viewing the compiled install products of the current library.

You can execute the following command to install the library:

```
$ ./celer install name@version
```

The structure of the `installed` folder is as follows:

```
└─ installed
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

**1. installed/celer/hash：** Stores the hash key for each library in this folder. When `cache_dir` is configured in `celer.toml`, this hash will be stored as a key-value pair alongside the build artifacts. If a subsequent compilation finds a matching hash in the cache, it will directly reuse the corresponding build artifacts to avoid redundant recompilation.  

**2. installed/celer/info：** Stores the installation file manifest for each library in this folder. This file is the main credential for judging whether a library is installed, and also the basis for implementing the deletion of installed libraries.  

**3. installed/x86_64-windows-dev:** Many third-party libraries require extra tools(e.g., NASM for x264) during compilation. Celer manages such dependencies by installing these tools into this kind of directory. Celer would also adds this `installed/x86_64-windows-dev/bin` path in to PATH environment variable. On Linux, it also compiles and installs autoconf, automake, m4, libtool, and gettext from source into this folder. 

**4. installed/x86_64-windows-msvc-14.44@test_project_02@release:** All compiled artifacts of third-party libraries will be stored in this kind of folder. In the `toolchain_file.cmake`, the `CMAKE_PREFIX_PATH` will be set to this folder, so that CMake can find the third-party libraries in this folder.
