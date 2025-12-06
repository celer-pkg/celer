# 📦 Install Command

&emsp;&emsp;The `install` command is used to download, compile, and install specified third-party libraries (ports), supporting multiple build configurations and installation modes with flexible control over build behavior and caching strategies.

## Command Syntax

```shell
celer install [package_name] [options]
```

## ⚙️ Command Options

| Option            | Short | Description                                        |
|-------------------|-------|----------------------------------------------------|
| --dev             | -d    | Install in dev mode (for build-time dependencies only) |
| --force           | -f    | Force reinstall (uninstall first, then install)    |
| --jobs            | -j    | Specify number of parallel build jobs              |
| --recursive       | -r    | With --force, recursively reinstall all dependencies |
| --store-cache     | -s    | Store build artifacts into cache after installation |
| --cache-token     | -t    | With --store-cache, specify cache token            |

## 💡 Usage Examples

### 1️⃣ Standard Installation

```shell
celer install ffmpeg@5.1.6
```

> Install the specified version of FFmpeg and all its dependencies.

### 2️⃣ Install as Development Dependency

```shell
celer install pkgconf@2.4.3 --dev
# Or use shorthand
celer install pkgconf@2.4.3 -d
```

> Development dependencies are only used when building other libraries and are not included in the final project deployment.

### 3️⃣ Force Reinstall

```shell
celer install ffmpeg@5.1.6 --force
# Or use shorthand
celer install ffmpeg@5.1.6 -f
```

> Force mode removes the installed library first, then reconfigures, builds, and installs.

### 4️⃣ Specify Parallel Build Jobs

```shell
celer install ffmpeg@5.1.6 --jobs 8
# Or use shorthand
celer install ffmpeg@5.1.6 -j 8
```

> Setting an appropriate number of parallel jobs based on CPU cores can speed up compilation.

### 5️⃣ Recursively Force Reinstall (Including Dependencies)

```shell
celer install ffmpeg@5.1.6 --force --recursive
# Or use shorthand
celer install ffmpeg@5.1.6 -f -r
```

> Recursive mode reinstalls the target library and all its dependencies.

### 6️⃣ Store into Cache After Installation

```shell
celer install ffmpeg@5.1.6 --store-cache --cache-token token_xxx
# Or use shorthand
celer install ffmpeg@5.1.6 -s -t token_xxx
```

> A text file named `token` must be stored in the `binary_cache` root directory to record the verification token. Only when the token verification passes can build artifact caches be uploaded.

---

## 📁 Installation Directory Structure

```
└─ installed
    ├── celer
    │   ├── hash
    │   │   ├── nasm@2.16.03@x86_64-windows-dev.hash
    │   │   └── x264@stable@x86_64-windows-msvc-community-14.44@project_test_02@release.hash
    │   └── info
    │       ├── nasm@2.16.03@x86_64-windows-dev.trace
    │       └── x264@stable@x86_64-windows-msvc-community-14.44@project_test_02@release.trace
    ├── x86_64-windows-dev
    │   ├── LICENSE
    │   └── bin
    │       ├── nasm.exe
    │       └── ndisasm.exe
    └── x86_64-windows-msvc-community-14.44@project_test_02@release
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

### Directory Description

#### 1️⃣ `installed/celer/hash/`

Stores the hash key for each library. When `binary_cache` is configured in `celer.toml`, this hash will be stored as a key-value pair alongside the build artifacts. If a subsequent compilation finds a matching hash in the cache, it will directly reuse the corresponding build artifacts to avoid redundant recompilation.

#### 2️⃣ `installed/celer/info/`

Stores the installation file manifest (`.trace` files) for each library. This file is the primary credential for determining whether a library is installed, and also the basis for removing installed libraries.

#### 3️⃣ `installed/x86_64-windows-dev/`

Many third-party libraries require additional build tools during compilation (e.g., x264 needs NASM). Celer manages development dependencies by installing these tools into this directory and automatically adds the `installed/x86_64-windows-dev/bin` path to the `PATH` environment variable.

On Linux systems, Celer also compiles and installs autoconf, automake, m4, libtool, and gettext from source into this directory.

#### 4️⃣ `installed/x86_64-windows-msvc-community-14.44@project_test_02@release/`

Storage directory for all third-party library build artifacts. In `toolchain_file.cmake`, `CMAKE_PREFIX_PATH` will be set to this directory so that CMake can find third-party libraries here.

The directory name includes:
- Platform architecture (`x86_64-windows`)
- Toolchain version (`msvc-community-14.44`)
- Project name (`project_test_02`)
- Build type (`release`)

---

## 📚 Related Documentation

- [Quick Start](./quick_start.md)
- [Deploy Command](./cmd_deploy.md) - Deploy entire project
- [Remove Command](./cmd_remove.md) - Remove installed libraries
- [Port Configuration](./advance_port.md) - Configure third-party libraries
- [Binary Cache](./advance_cache_artifacts.md) - Configure build cache

---

**Need Help?** [Report an Issue](https://github.com/celer-pkg/celer/issues) or check our [Documentation](../../README.md)
