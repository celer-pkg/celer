# Generate CMake Config Files

> **Automatically generate standard CMake config files for non-CMake libraries**

## 🎯 Why Do You Need This?

Many excellent third-party libraries (like FFmpeg, x264) don't use CMake as their build system and don't generate CMake config files after installation. This creates integration challenges for projects using CMake:

**Problems with Traditional Approaches:**
- 🔍 **Hard to Find**: Need to manually write `FindXXX.cmake` modules
- 🪟 **Platform Differences**: Using `pkg-config` on Windows is difficult
- 🔗 **Complex Dependencies**: Hard to manage dependencies for multi-component libraries
- ⚙️ **High Maintenance Cost**: Each library needs custom find scripts

**Celer's Solution:**
- ✅ Automatically generate standard CMake config files
- ✅ Consistent cross-platform experience
- ✅ Automatically handle inter-component dependencies
- ✅ Support for static, shared, and interface libraries

## 📚 Configuration Types Overview

Choose the appropriate configuration type based on your library's characteristics:

| Type | Use Case | Typical Examples | Complexity |
|------|----------|------------------|------------|
| **🎯 Single Target** | Single library file, no sub-modules | x264, zlib, sqlite | ⭐ Simple |
| **📦 Multi-Component** | Multiple independent modules, can be used separately | FFmpeg, Boost, OpenCV | ⭐⭐⭐ Medium |
| **🔗 Interface Library** | Pre-built libraries or header-only libraries | Pre-built SDK, header-only libs | ⭐⭐ Simple |

---

## 1️⃣ Single Target Library Configuration

### Use Case

Suitable for simple libraries with only one main library file, such as:
- **x264**: Video encoding library
- **zlib**: Compression library  
- **sqlite3**: Database engine

### Configuration Steps

#### Step 1: Create Configuration File

Create a `cmake_config.toml` file in the port's version directory:

```shell
x264/
└── stable/
    ├── cmake_config.toml  # ← Create this file
    └── port.toml
```

#### Step 2: Write Configuration

`cmake_config.toml` content example:

```toml
# Namespace, also the prefix for CMake config files
namespace = "x264"

# Linux static library configuration
[linux_static]
filename = "libx264.a"  # Library filename

# Linux shared library configuration
[linux_shared]
filename = "libx264.so.164"  # Actual filename (with version)
soname = "libx264.so"        # Symbol link name (SONAME)

[windows_static]
filename = "x264.lib"

# Windows shared library configuration
[windows_shared]
filename = "libx264-164.dll"  # DLL filename
impname = "libx264.lib"       # Import library name (.lib)
```

**Field Descriptions:**

| Field | Description | Platform | Required |
|-------|-------------|----------|----------|
| `namespace` | CMake namespace and config file prefix | All | No* |
| `filename` | Actual library filename | All | Yes |
| `soname` | Shared library symbol name (symlink) | Linux | Required for shared |
| `impname` | Import library filename | Windows | Required for shared |

> 💡 *If `namespace` is not specified, the library name will be used as default

#### Step 3: Generated Files

After compilation and installation, the following will be generated in `lib/cmake/`:

```shell
lib/cmake/x264/
├── x264Config.cmake           # Main config file
├── x264ConfigVersion.cmake    # Version information
├── x264Targets.cmake          # Target definitions
└── x264Targets-release.cmake  # Release configuration
```

#### Step 4: Use in Your Project

```cmake
# Find the library
find_package(x264 REQUIRED)

# Link to your target
target_link_libraries(${PROJECT_NAME} PRIVATE x264::x264)
```

---

## 2️⃣ Multi-Component Library Configuration

## 2️⃣ Multi-Component Library Configuration

### Use Case

Suitable for libraries containing multiple independent modules that can be used separately, such as:
- **FFmpeg**: Contains avcodec, avformat, avutil, and more
- **Boost**: Contains numerous independent sub-libraries
- **OpenCV**: Contains core, imgproc, video, and other modules

### Configuration Steps

#### Step 1: Create Configuration File

```shell
ffmpeg/
└── 5.1.6/
    ├── cmake_config.toml  # ← Create this file
    └── port.toml
```

#### Step 2: Write Configuration

`cmake_config.toml` content example (showing partial components):

```toml
namespace = "FFmpeg"

[linux_shared]
# avutil component - Basic utility library (no dependencies)
[[linux_shared.components]]
component = "avutil"                    # Component name
soname = "libavutil.so.55"             # Symbol name
filename = "libavutil.so.55.78.100"    # Actual filename
dependencies = []                       # No dependencies

# avcodec component - Codec library (depends on avutil)
[[linux_shared.components]]
component = "avcodec"
soname = "libavcodec.so.57"
filename = "libavcodec.so.57.107.100"
dependencies = ["avutil"]              # Depends on avutil

[[linux_shared.components]]
component = "avdevice"
soname = "libavdevice.so.57"
filename = "libavdevice.so.57.10.100"
dependencies = ["avformat", "avutil"]

[[linux_shared.components]]
component = "avfilter"
soname = "libavfilter.so.6"
filename = "libavfilter.so.6.107.100"
dependencies = ["swscale", "swresample"]

[[linux_shared.components]]
component = "avformat"
soname = "libavformat.so.57"
filename = "libavformat.so.57.83.100"
dependencies = ["avcodec", "avutil"]

[[linux_shared.components]]
component = "postproc"
soname = "libpostproc.so.54"
filename = "libpostproc.so.54.7.100"
dependencies = ["avcodec", "swscale", "avutil"]

[[linux_shared.components]]
component = "swresample"
soname = "libswresample.so.2"
filename = "libswresample.so.2.9.100"
dependencies = ["avcodec", "swscale", "avutil", "avformat"]

[[linux_shared.components]]
component = "swscale"
soname = "libswscale.so.4"
filename = "libswscale.so.4.8.100"
dependencies = ["avcodec", "avutil", "avformat"]

[windows_shared]
[[windows_shared.components]]
component = "avutil"
impname = "avutil.lib"
filename = "avutil-55.dll"
dependencies = []

[[windows_shared.components]]
component = "avcodec"
impname = "avcodec.lib"
filename = "avcodec-57.dll"
dependencies = ["avutil"]

[[windows_shared.components]]
component = "avdevice"
impname = "avdevice.lib"
filename = "avdevice-57.dll"
dependencies = ["avformat", "avutil"]

[[windows_shared.components]]
component = "avfilter"
impname = "avfilter.lib"
filename = "avfilter-6.dll"
dependencies = ["swscale", "swresample"]

[[windows_shared.components]]
component = "avformat"
impname = "avformat.lib"
filename = "avformat-57.dll"
dependencies = ["avcodec", "avutil"]

[[windows_shared.components]]
component = "postproc"
impname = "postproc.lib"
filename = "postproc-54.dll"
dependencies = ["avcodec", "swscale", "avutil"]

[[windows_shared.components]]
component = "swresample"
impname = "swresample.lib"
filename = "swresample-2.dll"
dependencies = ["avcodec", "swscale", "avutil", "avformat"]

[[windows_shared.components]]
component = "swscale"
impname = "swscale.lib"
filename = "swscale-4.dll"
dependencies = ["avcodec", "avutil", "avformat"]
```

> **Note:**  
> Note that different components may have different dependencies, Celer supports defining the dependency relation for them in the `dependencies` field.

After compiling and installing, you can see the generated cmake config files as follows:

```
lib
└── cmake
    └─── FFmpeg
        ├── FFmpegConfig.cmake
        ├── FFmpegConfigVersion.cmake
        ├── FFmpegModules-release.cmake
        └── FFmpegModules.cmake
```

Finally, you can use it in your cmake project as follows:

```cmake
find_package(FFmpeg REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
    FFmpeg::avutil
    FFmpeg::avcodec
    FFmpeg::avdevice
    FFmpeg::avfilter
    FFmpeg::avformat
    FFmpeg::postproc
    FFmpeg::swresample
    FFmpeg::swscale
)
```

**3. How to generate cmake config files for interface target**

For example, prebuilt-ffmpeg, you should create a **cmake_config.toml** file in the version directory of the port.

```
prebuilt-ffmpeg
└── 5.1.6
    ├── cmake_config.toml
    └── port.toml
```

```toml
[package]
ref = "5.1.6"

[[build_configs]]
url = "https://github.com/celer-pkg/test-conf/releases/download/resource/prebuilt-ffmpeg@5.1.6@x86_64-linux.tar.gz"
pattern = "x86_64-linux*"
build_system = "prebuilt"
library_type = "interface"  # ---- it should be changed to `interface`
```
>To generate interface type cmake configs, you need to set `library_type` as **interface**.

```toml
namespace = "FFmpeg"

[linux_interface]
libraries = [
    "libavutil.so.57",
    "libavcodec.so.59",
    "libavdevice.so.59",
    "libavfilter.so.8",
    "libavformat.so.59",
    "libpostproc.so.56",
    "libswresample.so.4",
    "libswscale.so.6",
]
```

> Because it is an interface type cmake config file, we only need to list all the libraries that need to be linked in the **libraries** field.

After compiling and installing, you can see the generated cmake config files as follows:

```
lib
└── cmake
    └── FFmpeg
        ├── FFmpegConfig.cmake
        └── FFmpegConfigVersion.cmake
```

Finally, you can use it in your cmake project as follows:

```cmake
find_package(FFmpeg REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE FFmpeg::prebuilt-ffmpeg)
```

> **Note:**  
> **1.** If namespace is not specified, it will be the same as the library name. And the namespace is also the prefix of the config file name.  
> **2.** The installed libraries files would be removed when there is any wrong in the cmake config file.
