# Install command

&emsp;&emsp;The celer install command downloads, compiles, and installs packages with dependency resolution. It supports multiple build configurations and installation modes.

## Command Syntax

```shell
celer install [package_name] [flags]  
```

## Command Options

| Option	        | Short flag | Description                                              |
| ----------------- | ---------- | ---------------------------------------------------------|
| --build-type	    | -b	     | Install with specified build type, default is release.   |
| --dev             | -d         | Install in dev mode.                                     |
| --force	        | -f	     | Try to uninstall before installation.                    |
| --recurse	        | -r	     | Combine with --force, recursively reinstall dependencies.|
| --store-cache     | -s         | Store artifact into cache after installation.            |

## Usage Examples

**1. Standard Installation:**

```shell
celer install ffmpeg@5.1.6
```

**2. Install in dev mode:**

```shell
celer install pkgconf@2.4.3 --dev/-d  
```

**3. Install forcibly:**

```shell
celer install ffmpeg@5.1.6 --force/-f
```
>Removes installed package and configure, build, install again.

**4. Recursively reinstall dependencies:**

```shell
celer install ffmpeg@5.1.6 --force/-f --recurse/-r
```

**5. Store artifact into cache after installation:**

```shell
celer install ffmpeg@5.1.6 --store-cache/-f
```

## Structure of installed directory

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
&emsp;&emsp;Stores the hash key for each library in this folder. When `cache_dir` is configured in `celer.toml`, this hash will be stored as a key-value pair alongside the build artifacts. If a subsequent compilation finds a matching hash in the cache, it will directly reuse the corresponding build artifacts to avoid redundant recompilation.  

**2. installed/celer/info**   
&emsp;&emsp;Stores the installation file manifest for each library in this folder. This file is one of the main credentials for judging whether a library is installed, and also the basis for implementing the removing installed libraries.  

**3. installed/x86_64-windows-dev**   
&emsp;&emsp;Many third-party libraries require extra tools(e.g., NASM for x264) during compilation. Celer manages such dependencies by installing these tools into this kind of directory. Celer would also adds this `installed/x86_64-windows-dev/bin` path in to PATH environment variable. On Linux, it also compiles and installs autoconf, automake, m4, libtool, and gettext from source into this folder. 

**4. installed/x86_64-windows-msvc-14.44@test_project_02@release**   
&emsp;&emsp;All compiled artifacts of third-party libraries will be stored in this kind of folder. In the `toolchain_file.cmake`, the `CMAKE_PREFIX_PATH` will be set to this folder, so that CMake can find the third-party libraries in this folder.
