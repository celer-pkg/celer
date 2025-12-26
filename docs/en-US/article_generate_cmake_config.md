# Generate CMake Config Files

> **Automatically generate standard CMake config files for prebuilt libraries**

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

> 💡 *If `namespace` is not specified, the library name will be used as default.

#### Step 3: Generated Files

After compilation and installation, the following will be generated in `lib/cmake/`:

```shell
lib/cmake/x264/
├── x264Config.cmake           # Main config file
├── x264ConfigVersion.cmake    # Version information
└── x264Targets.cmake          # Release configuration
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

[linux]
# avutil component - Basic utility library (no dependencies)
[[linux.components]]
component = "avutil"                    # Component name
filename = "libavutil.so.55"            # Lib filename
dependencies = []                       # No dependencies

# avcodec component - Codec library (depends on avutil)
[[linux.components]]
component = "avcodec"
filename = "libavcodec.so.57"
dependencies = ["avutil"]              # Depends on avutil

[[linux.components]]
component = "avdevice"
filename = "libavdevice.so.57"
dependencies = ["avformat", "avutil"]

[[linux.components]]
...

[windows]
...
```

> **Note:**  
> Note that different components may have different dependencies, CMake will generate cmake config files with dependency relation inside.

After compiling and installing, you can see the generated cmake config files as follows:

```
lib
└── cmake
    └─── FFmpeg
        ├── FFmpegConfig.cmake
        ├── FFmpegConfigVersion.cmake
        └── FFmpegTarget.cmake
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
```

```toml
namespace = "FFmpeg"

[linux]
filenames = [
    "libavutil.so.57",
    "libavcodec.so.59",
    "libavdevice.so.59",
    "libavfilter.so.8",
    "libavformat.so.59",
    "libpostproc.so.56",
    "libswresample.so.4",
    "libswscale.so.6",
]

[windows]
filenames = [
    "avutil.lib",
    "avcodec.lib",
    "avdevice.lib",
    "avfilter.lib",
    "avformat.lib",
    "postproc.lib",
    "swresample.lib",
    "swscale.lib",
]
```

> 💡 **Tip**: For Interface type, just list all the libraries that need to be linked. No need to specify components or dependencies.

**Step 3: Generated Files**

```
lib/cmake/FFmpeg/
├── FFmpegConfig.cmake
└── FFmpegConfigVersion.cmake
```

**Step 4: Use in Your Project**

```cmake
find_package(FFmpeg REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE FFmpeg::prebuilt-ffmpeg)
```

> **Note:**  
> **1.** If namespace is not specified, it will default to the library name.