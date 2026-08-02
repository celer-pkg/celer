# Generate CMake Config Files

> **Automatically generate standard CMake config files for any library**

## Why Do You Need This?

Many third-party libraries (like FFmpeg, x264) don't use CMake as their build system and don't generate CMake config files after installation. Even some CMake-based libraries forget to export their targets. This creates integration challenges for projects using CMake:

**Problems with Traditional Approaches:**
- **Hard to Find**: Need to manually write `FindXXX.cmake` modules
- **Platform Differences**: Using `pkg-config` on Windows is difficult
- **Complex Dependencies**: Hard to manage dependencies for multi-component libraries
- **High Maintenance Cost**: Each library needs custom find scripts

**Celer's Solution:**
- Automatically generate standard CMake config files for **any** build system
- Zero-config auto-detect for simple single-target libraries
- Consistent cross-platform experience
- Automatically handle inter-component dependencies

---

## Configuration Types Overview

| Type | Use Case | Typical Examples | Complexity |
|------|----------|------------------|------------|
| **Single Target** | Single library, no sub-modules | x264, zlib, sqlite | ⭐ Simple |
| **Multi-Component** | Multiple independent modules | FFmpeg, Boost, OpenCV | ⭐⭐⭐ Medium |

---

## 1. Single Target Library Configuration

### Use Case

Suitable for simple libraries with only one main library file, such as:
- **x264**: Video encoding library
- **zlib**: Compression library
- **sqlite3**: Database engine

### Configuration Steps

#### Step 1: Create Configuration File

Create a `cmake_config.toml` file in the port's version directory:

```
x264/
└── stable/
    ├── cmake_config.toml  # ← Create this file
    └── port.toml
```

#### Step 2: Write Configuration (Minimal)

The simplest form — namespace only, library filenames are auto-detected from `lib/`:

```toml
namespace = "x264"

[linux]

[windows]
```

> 💡 **Auto-Detect**: When `filename` / `filenames` is omitted and no `components` are defined, celer automatically scans the installed `lib/` directory for library files (`.a`, `.lib`, `.so*`, `.dylib`). DLLs under `bin/` are runtime dependencies and excluded from the link list.

#### Step 3: Explicit Filenames (Optional)

You can explicitly list the library files if needed:

```toml
namespace = "x264"

[linux]
filename = "libx264.a"

[windows]
filename = "libx264.lib"
```

Or list multiple files:

```toml
namespace = "zlib"

[linux]
filenames = ["libz.a", "libz.so.1"]

[windows]
filenames = ["zlib.lib"]
```

**Field Descriptions:**

| Field | Description | Required |
|-------|-------------|----------|
| `namespace` | CMake namespace and config file prefix | No — defaults to library name |
| `filename` | Single library filename | No — auto-detects from `lib/` |
| `filenames` | Multiple library filenames | No — auto-detects from `lib/` |

#### Step 4: Generated Files

After installation, the following will be generated in `lib/cmake/`:

```
lib/cmake/x264/
├── x264Config.cmake
├── x264ConfigVersion.cmake
└── x264Targets.cmake
```

#### Step 5: Use in Your Project

```cmake
find_package(x264 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE x264::x264)
```

---

## 2. Multi-Component Library Configuration

### Use Case

Suitable for libraries containing multiple independent modules that can be used separately:
- **FFmpeg**: avcodec, avformat, avutil, and more
- **Boost**: Numerous independent sub-libraries
- **OpenCV**: core, imgproc, video, and other modules

### Configuration Steps

#### Step 1: Create Configuration File

```
ffmpeg/
└── 5.1.6/
    ├── cmake_config.toml
    └── port.toml
```

#### Step 2: Write Configuration

```toml
namespace = "FFmpeg"

[linux]
[[linux.components]]
  component = "avutil"
  filename = "libavutil.so.57"
  dependencies = []

[[linux.components]]
  component = "avcodec"
  filename = "libavcodec.so.59"
  dependencies = ["avutil"]

[[linux.components]]
  component = "avdevice"
  filename = "libavdevice.so.59"
  dependencies = ["avformat", "avutil"]

[windows]
[[windows.components]]
  component = "avutil"
  filename = "avutil.lib"
  dependencies = []

[[windows.components]]
  component = "avcodec"
  filename = "avcodec.lib"
  dependencies = ["avutil"]
```

> **Note:** Auto-detect is disabled when `components` is present — every component must explicitly declare its `filename`.

#### Step 3: Generated Files

```
lib/cmake/FFmpeg/
├── FFmpegConfig.cmake
├── FFmpegConfigVersion.cmake
└── FFmpegTargets.cmake
```

#### Step 4: Use in Your Project

```cmake
find_package(FFmpeg REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
  FFmpeg::avutil
  FFmpeg::avcodec
  FFmpeg::avdevice
)
```

---

## 3. How It Works

### Supported Build Systems

CMake config generation works with **all** build systems. celer generates the config files as a post-install step, after the library files are already in `PackageDir/lib/`.

| Build System | When Config is Generated |
|-------------|--------------------------|
| `prebuilt`  | During configure phase (uses `RepoDir`) |
| `makefiles`, `cmake`, `meson`, `b2`, `gyp`, `qmake`, `bazel`, `custom` | After install (uses `PackageDir`) |

### Auto-Detect Logic

When `cmake_config.toml` has no `filename` / `filenames` and no `components`:

1. Scan `PackageDir/lib/` for `.a`, `.lib`, `.so`, `.so.*`, `.dylib`
2. Namespace defaults to the library name from `port.toml`
3. DLLs in `bin/` are **not** included (runtime, not link-time)
