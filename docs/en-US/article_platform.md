# Platform Configuration

> **Configure cross-compilation environments for different target platforms**

## 🎯 What is Platform Configuration?

Platform configuration defines how Celer compiles C/C++ libraries for specific target systems. Each platform configuration contains two core components:

- 🔧 **Toolchain** - Compilers, linkers, and other build tools
- 📦 **Rootfs (Root Filesystem)** - Header files and libraries for the target system

**Why Do You Need Platform Configuration?**

Building C/C++ projects requires the correct compiler and system libraries. Platform configuration enables Celer to:
- ✅ Build for different operating systems (Linux, Windows, macOS)
- ✅ Support cross-compilation (e.g., build ARM binaries on x86)
- ✅ Use specific compiler versions (GCC 9.5, Clang 14, MSVC 2022)
- ✅ Manage multi-platform build environments

**Platform File Location:** All platform configuration files are stored in the `conf/platforms` directory.

---

## 📝 Platform Naming Convention

Platform configuration files follow a unified naming format:

```
<architecture>-<system>-<distribution>-<compiler>-<version>.toml
```

**Examples:**
- `x86_64-linux-ubuntu-22.04-gcc-11.5.0.toml`
- `aarch64-linux-gnu-gcc-9.2.toml`
- `x86_64-windows-msvc-14.44.toml`

**Naming Components:**

| Component | Description | Examples |
|-----------|-------------|----------|
| Architecture | CPU architecture | `x86_64`, `aarch64`, `arm` |
| System | Operating system | `linux`, `windows`, `darwin` |
| Distribution | System distribution (optional) | `ubuntu-22.04`, `centos-7` |
| Compiler | Toolchain type | `gcc`, `clang`, `msvc` |
| Version | Compiler version | `11.5.0`, `14.44` |

> 💡 **Tip**: Consistent naming helps teams quickly identify and select the correct platform configuration.

## 🛠️ Configuration Field Details

### Complete Example Configuration

Let's look at a complete Linux platform configuration file `x86_64-linux-ubuntu-22.04-gcc-9.5.toml`:

```toml
[rootfs]
  url = "https://github.com/celer-pkg/test-conf/releases/download/resource/ubuntu-base-20.04.5-base-amd64.tar.gz"
  name = "gcc"
  version = "9.5"
  path = "ubuntu-base-20.04.5-base-amd64"
  pkg_config_path = [
      "usr/lib/x86_64-linux-gnu/pkgconfig",
      "usr/share/pkgconfig",
      "usr/lib/pkgconfig"
  ]

[toolchain]
  url = "https://github.com/celer-pkg/test-conf/releases/download/resource/gcc-9.5.0.tar.gz"
  path = "gcc-9.5.0/bin"
  system_name = "Linux"
  system_processor = "x86_64"
  host = "x86_64-linux-gnu"
  crosstool_prefix = "x86_64-linux-gnu-"
  cc = "x86_64-linux-gnu-gcc"
  cxx = "x86_64-linux-gnu-g++"
  fc = "x86_64-linux-gnu-gfortran"            # Optional field
  ranlib = "x86_64-linux-gnu-ranlib"          # Optional field
  ar = "x86_64-linux-gnu-ar"                  # Optional field
  nm = "x86_64-linux-gnu-nm"                  # Optional field
  objdump = "x86_64-linux-gnu-objdump"        # Optional field
  strip = "x86_64-linux-gnu-strip"            # Optional field
```

### 1️⃣ Toolchain Configuration Fields

| Field | Required | Description | Examples |
|-------|----------|-------------|----------|
| `url` | ✅ | Toolchain download URL or local path. Supports http/https/ftp protocols. Local paths must start with `file:///` | `https://...gcc-9.5.0.tar.gz`<br>`file:///C:/toolchains/gcc.tar.gz` |
| `path` | ✅ | Relative path to the toolchain bin directory. Celer adds it to PATH environment variable and CMake's `$ENV{PATH}` | `gcc-9.5.0/bin` |
| `system_name` | ✅ | Target operating system name | `Linux`, `Windows`, `Darwin` |
| `system_processor` | ✅ | Target CPU architecture | `x86_64`, `aarch64`, `arm`, `i386` |
| `host` | ✅ | Toolchain target triple, defines the target platform for compiler-generated code | `x86_64-linux-gnu`<br>`aarch64-linux-gnu`<br>`i686-w64-mingw32` |
| `crosstool_prefix` | ✅ | Prefix for toolchain executables, used to locate compiler tools | `x86_64-linux-gnu-`<br>`arm-none-eabi-` |
| `cc` | ✅ | C compiler executable name | `x86_64-linux-gnu-gcc`<br>`clang` |
| `cxx` | ✅ | C++ compiler executable name | `x86_64-linux-gnu-g++`<br>`clang++` |
| `name` | ✅ | Toolchain name (for identification) | `gcc`, `clang`, `msvc` |
| `version` | ✅ | Toolchain version number | `9.5`, `11.3`, `14.0.0` |
| `embedded_system` | ❌ | Whether this is for embedded systems (like MCU or bare-metal) | `true` (MCU/bare-metal)<br>`false` or omit (regular systems) |
| `fc` | ❌ | Fortran compiler (if needed) | `x86_64-linux-gnu-gfortran` |
| `ranlib` | ❌ | Library index generator | `x86_64-linux-gnu-ranlib` |
| `ar` | ❌ | Static library archiver | `x86_64-linux-gnu-ar` |
| `nm` | ❌ | Symbol table viewer | `x86_64-linux-gnu-nm` |
| `objdump` | ❌ | Object file analyzer | `x86_64-linux-gnu-objdump` |
| `strip` | ❌ | Symbol stripping tool | `x86_64-linux-gnu-strip` |

> ⚠️ **Note**: Optional tools (fc, ranlib, etc.) will be automatically located using `crosstool_prefix` if not specified.

### 2️⃣ Rootfs (Root Filesystem) Configuration Fields

| Field | Required | Description | Examples |
|-------|----------|-------------|----------|
| `url` | ✅ | Rootfs download URL or local path. Supports http/https/ftp protocols. Local paths must start with `file:///` | `https://...ubuntu-base.tar.gz`<br>`file:///D:/sysroots/ubuntu.tar.gz` |
| `path` | ✅ | Directory name after rootfs extraction | `ubuntu-base-20.04.5-base-amd64` |
| `pkg_config_path` | ✅ | List of pkg-config search paths, relative to rootfs root directory | `["usr/lib/x86_64-linux-gnu/pkgconfig", "usr/share/pkgconfig"]` |

---

## 💼 Real-World Configuration Examples

### Linux Platform Configurations

#### GCC Toolchain

```toml
[rootfs]
  url = "https://github.com/celer-pkg/test-conf/releases/download/resource/ubuntu-base-22.04-amd64.tar.gz"
  path = "ubuntu-base-22.04-amd64"
  pkg_config_path = [
      "usr/lib/x86_64-linux-gnu/pkgconfig",
      "usr/share/pkgconfig"
  ]

[toolchain]
  url = "https://github.com/celer-pkg/test-conf/releases/download/resource/gcc-11.3.0.tar.gz"
  path = "gcc-11.3.0/bin"
  system_name = "Linux"
  system_processor = "x86_64"
  host = "x86_64-linux-gnu"
  crosstool_prefix = "x86_64-linux-gnu-"
  cc = "x86_64-linux-gnu-gcc"
  cxx = "x86_64-linux-gnu-g++"
```

#### Clang Toolchain

```toml
[toolchain]
  url = "file:///opt/llvm-14.0.0"
  path = "bin"
  system_name = "Linux"
  system_processor = "x86_64"
  host = "x86_64-linux-gnu"
  cc = "clang"
  cxx = "clang++"
```

### Embedded System Platform Configurations

#### ARM Cortex-M MCU Configuration

Embedded systems (like MCUs or bare-metal environments) require special configuration because they don't have a full operating system:

```toml
[toolchain]
  url = "https://developer.arm.com/-/media/Files/downloads/gnu-rm/gcc-arm-none-eabi-10.3.tar.bz2"
  path = "gcc-arm-none-eabi-10.3/bin"
  system_name = "Generic"
  system_processor = "arm"
  host = "arm-none-eabi"
  crosstool_prefix = "arm-none-eabi-"
  embedded_system = true
  cc = "arm-none-eabi-gcc"
  cxx = "arm-none-eabi-g++"
  ar = "arm-none-eabi-ar"
  objcopy = "arm-none-eabi-objcopy"
  objdump = "arm-none-eabi-objdump"
```

> 💡 **Key Points**:
> - `embedded_system = true` tells Celer this is an embedded environment
> - `system_name = "Generic"` indicates no specific operating system
> - `host = "arm-none-eabi"` is the standard triple for bare-metal ARM toolchain
> - No rootfs configuration needed, as MCUs don't have a filesystem

### Windows Platform Configurations

#### MSVC 2022 Configuration

Windows uses MSVC to compile C/C++ projects. MSVC configuration differs from Linux GCC:
- ✅ Compiler filenames are fixed (`cl.exe`, `link.exe`)
- ✅ Header files and libraries are scattered across multiple directories
- ✅ Celer automatically handles all path configurations

**Simplified MSVC configuration:**

```toml
[toolchain]
url = "file:///C:/Program Files/Microsoft Visual Studio/2022/Community"
name = "msvc"
version = "14.44.35207"
system_name = "Windows"
system_processor = "x86_64"
```

> 💡 **Tip**: Celer automatically detects MSVC installation paths, including Windows SDK, UCRT, and compiler tools.

---

## 📚 Related Documentation

- [Quick Start Guide](./quick_start.md) - Get started with Celer
- [Project Configuration](./cmd_create.md#2-create-a-new-project) - Select platform in celer.toml
- [Build Configuration](./article_buildconfig.md) - Configure build options and dependencies

---

**Need help?** [Report an issue](https://github.com/celer-pkg/celer/issues) or check our [documentation](../../README.md)