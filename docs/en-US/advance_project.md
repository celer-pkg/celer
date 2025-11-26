# Project Configuration

> **Define independent build environments and dependencies for different projects**

## 🎯 What is Project Configuration?

Project configuration defines how Celer manages dependencies and build environments for a specific project. Each project configuration contains five core components:

- 📦 **Ports (Dependencies)** - Third-party libraries required by the project
- 🔧 **Vars (CMake Variables)** - Global CMake build variables
- 🌍 **Envs (Environment Variables)** - Environment variables needed during build
- 🏷️ **Micros (Macro Definitions)** - C/C++ preprocessor macros
- ⚙️ **Compile Options (Optimization Flags)** - Compiler flags and optimization options

**Why do we need project configuration?**

Each project has its unique configuration characteristics. Project configuration allows Celer to:
- ✅ Manage project dependencies uniformly
- ✅ Share consistent build environments across teams
- ✅ Quickly switch between different project build configurations
- ✅ Independently manage compilation options and macro definitions for each project

**Project File Location:** All project configuration files are stored in the `conf/projects` directory.

---

## 📝 Project Naming Convention

Project configuration files follow a unified naming format:

```
project_<name>.toml
```

**Examples:**
- `project_001.toml` - First project configuration
- `project_opencv.toml` - OpenCV project configuration
- `project_multimedia.toml` - Multimedia project configuration

> 💡 **Tip**: It's recommended to use meaningful names or numbers to identify different projects for easy team recognition and management.

---

## 🛠️ Configuration Field Details

### Complete Example Configuration

Let's look at a complete project configuration file `project_xxx.toml`:

```toml
ports = [
    "x264@stable",
    "sqlite3@3.49.0",
    "ffmpeg@3.4.13",
    "zlib@1.3.1",
    "opencv@4.5.1"
]

vars = [
    "CMAKE_VAR1=value1",
    "CMAKE_VAR2=value2"
]

envs = [
    "ENV_VAR1=001"
]

micros = [
    "MICRO_VAR1=111",
    "MICRO_VAR2"
]

[optimize]
    debug = "-O0 -g3 -fno-omit-frame-pointer"
    release = "-O3 -DNDEBUG"
    relwithdebinfo = "-O2 -g -DNDEBUG"
    minsizerel = "-Os -DNDEBUG"
```

> The optimize section can configure multiple compiler scenarios at once, allowing a single project to share one toml configuration across multiple systems and compilers, as shown below:

```toml
[optimize_msvc]
    debug = "/MDd /Zi /Od /Ob0 /RTC1"
    release = "/MD /O2 /Ob2 /DNDEBUG"
    relwithdebinfo = "/MD /Zi /O2 /Ob1 /DNDEBUG"
    minsizerel = "/MD /O1 /Ob1 /DNDEBUG"

[optimize_gcc]
    debug = "-O0 -g3 -fno-omit-frame-pointer"
    release = "-O3 -DNDEBUG"
    relwithdebinfo = "-O2 -g -DNDEBUG"
    minsizerel = "-Os -DNDEBUG"

[optimize_clang]
    debug = "-O0 -g3 -fno-omit-frame-pointer"
    release = "-O3 -DNDEBUG"
    relwithdebinfo = "-O2 -g -DNDEBUG"
    minsizerel = "-Oz -DNDEBUG"
```

### Configuration Field Descriptions

| Field | Required | Description | Example |
|------|------|------|------|
| `ports` | ❌ | Define third-party libraries the current project depends on. Format: `package@version` | `["x264@stable", "zlib@1.3.1"]` |
| `vars` | ❌ | Define global CMake variables required by the current project. Format: `variable=value` | `["CMAKE_BUILD_TYPE=Release"]` |
| `envs` | ❌ | Define global environment variables required by the current project. Format: `variable=value` | `["xorg_cv_malloc0_returns_null=yes"]` |
| `micros` | ❌ | Define C/C++ macro definitions required by the current project. Format: `macro=value` or `macro` | `["DEBUG=1", "ENABLE_LOGGING"]` |
| `optimize` | ❌ | Define compilation optimization options for a fixed compiler | See detailed description below |
| `optimize_gcc` | ❌ | Define optimization options for GCC compiler | See detailed description below |
| `optimize_clang` | ❌ | Define optimization options for Clang compiler | See detailed description below |
| `optimize_msvc` | ❌ | Define optimization options for MSVC compiler | See detailed description below |

> ⚠️ **Note**: All fields are optional. You can configure them selectively based on project needs.

### 1️⃣ Ports (Dependencies)

Specify third-party libraries that the project depends on. Celer will automatically download, compile, and install these dependencies.

**Format:** `"package@version"`

**Example:**
```toml
ports = [
    "zlib@1.3.1",           # Compression library
    "openssl@3.0.0",        # Encryption library
    "sqlite3@3.49.0",       # Database
    "x264@stable"           # Video encoding (using stable version)
]
```

**Version Notes:**
- Specify exact version: `@3.49.0`
- Use specific tag: `@stable`, `@latest`
- Version format must match the versions defined in the `ports` directory

> 💡 **Tip**: Use `celer search <package>` to view available version lists.

### 2️⃣ Vars (CMake Variables)

Define global CMake variables that will be passed to the build process of all dependent libraries and app development projects.

**Format:** `"variable=value"`

**Example:**
```toml
vars = [
    "PROJECT_NAME=telsa/model3",
    "PROJECT_CODE=0033FF"
]
```

### 3️⃣ Envs (Environment Variables)

Define environment variables needed during build, affecting compilation behavior.

**Format:** `"variable=value"`

**Example:**
```toml
envs = [
    "CFLAGS=-march=native",         # Set C compiler flags
    "CXXFLAGS=-march=native"        # Set C++ compiler flags
]
```

### 4️⃣ Micros (Macro Definitions)

Define C/C++ preprocessor macros to be injected into code during compilation.

**Format:** `"macro=value"` or `"macro"` (value-less macro)

**Example:**
```toml
micros = [
    "DEBUG=1",              # Enable debug mode (macro with value)
    "ENABLE_LOGGING",       # Enable logging (value-less macro)
    "MAX_BUFFER_SIZE=4096", # Define buffer size
    "_GNU_SOURCE"           # Enable GNU extensions
]
```

### 5️⃣ Optimize (Compilation Optimization Options)

Define compiler optimization flags for different build types. Celer supports configuring independent optimization options for different compilers, achieving consistent build configuration across platforms and compilers.

#### 🎯 Configuration Methods

**Method 1: Generic Configuration (Applicable to All Compilers)**

Use `[optimize]` to configure generic compilation optimization options:

```toml
[optimize]
    debug = "-O0 -g3 -fno-omit-frame-pointer"
    release = "-O3 -DNDEBUG"
    relwithdebinfo = "-O2 -g -DNDEBUG"
    minsizerel = "-Os -DNDEBUG"
```

**Method 2: Compiler-Specific Configuration (Recommended)**

Configure independent optimization options for different compilers. A single project can share the same configuration file across multiple systems and compilers:

```toml
# GCC compiler optimization configuration
[optimize_gcc]
    debug = "-O0 -g3 -fno-omit-frame-pointer"
    release = "-O3 -DNDEBUG"
    relwithdebinfo = "-O2 -g -DNDEBUG"
    minsizerel = "-Os -DNDEBUG"

# Clang compiler optimization configuration
[optimize_clang]
    debug = "-O0 -g3 -fno-omit-frame-pointer"
    release = "-O3 -DNDEBUG"
    relwithdebinfo = "-O2 -g -DNDEBUG"
    minsizerel = "-Oz -DNDEBUG"  # Clang uses -Oz for smaller size

# MSVC compiler optimization configuration
[optimize_msvc]
    debug = "/MDd /Zi /Od /Ob0 /RTC1"
    release = "/MD /O2 /Ob2 /DNDEBUG"
    relwithdebinfo = "/MD /Zi /O2 /Ob1 /DNDEBUG"
    minsizerel = "/MD /O1 /Ob1 /DNDEBUG"
```

> 💡 **Priority Rule**: Celer will prioritize using compiler-specific configurations (such as `optimize_gcc`). If undefined, it will use the generic configuration `optimize`.

#### 📋 Build Type Descriptions

| Build Type | Field Name | Purpose | Typical Scenarios |
|---------|--------|------|----------|
| Debug | `debug` | Debug build, no optimization, contains full debug information | Development, debugging, problem diagnosis |
| Release | `release` | Release build, maximum optimization, no debug information | Production environment, performance testing |
| RelWithDebInfo | `relwithdebinfo` | Release build + debug information | Performance profiling, production environment debugging |
| MinSizeRel | `minsizerel` | Minimum size optimization | Embedded systems, storage-constrained environments |

#### 🔧 GCC/Clang Common Optimization Options

**Optimization Levels:**

| Option | Description | Applicable Scenarios |
|------|------|----------|
| `-O0` | No optimization, fast compilation | Debug builds |
| `-O1` | Basic optimization | Balance compilation speed and performance |
| `-O2` | Medium optimization (recommended) | RelWithDebInfo builds |
| `-O3` | Maximum optimization | Release builds |
| `-Os` | Optimize code size | MinSizeRel builds (GCC) |
| `-Oz` | More aggressive size optimization | MinSizeRel builds (Clang) |

**Debug Options:**

| Option | Description |
|------|------|
| `-g` | Generate basic debug information |
| `-g3` | Generate most detailed debug information (including macro definitions) |
| `-fno-omit-frame-pointer` | Preserve frame pointer for easier debugging and performance profiling |

**Other Common Options:**

| Option | Description |
|------|------|
| `-DNDEBUG` | Disable assertions (assert) |
| `-Wall` | Enable all common warnings |
| `-Wextra` | Enable extra warnings |
| `-fPIC` | Generate position-independent code |
| `-march=native` | Optimize for current CPU |
| `-flto` | Enable Link-Time Optimization (LTO) |

#### 🪟 MSVC Common Optimization Options

**Optimization Levels:**

| Option | Description | Applicable Scenarios |
|------|------|----------|
| `/Od` | Disable optimization | Debug builds |
| `/O1` | Minimize code size | MinSizeRel builds |
| `/O2` | Maximize speed | Release/RelWithDebInfo builds |

**Debug Options:**

| Option | Description |
|------|------|
| `/Zi` | Generate complete debug information (PDB file) |
| `/Z7` | Embed debug information in .obj files |

**Runtime Libraries:**

| Option | Description |
|------|------|
| `/MD` | Multithreaded DLL runtime (Release) |
| `/MDd` | Multithreaded DLL runtime (Debug) |
| `/MT` | Multithreaded static runtime (Release) |
| `/MTd` | Multithreaded static runtime (Debug) |

**Inlining Options:**

| Option | Description |
|------|------|
| `/Ob0` | Disable inlining |
| `/Ob1` | Only inline functions marked as inline |
| `/Ob2` | Compiler automatic inlining |

**Other Options:**

| Option | Description |
|------|------|
| `/RTC1` | Runtime checks (detect stack corruption, uninitialized variables) |
| `/DNDEBUG` | Define NDEBUG macro (disable assertions) |
| `/GL` | Whole program optimization |
| `/std:c++17` | Use C++17 standard |

---

## 🚀 Using Project Configuration

### Create New Project

Use the `celer create` command to create a new project based on project configuration:

```bash
# Use specified project configuration
celer create --project x86_64-linux-ubuntu-22.04-gcc-11.5.0
```

### Switch Project Configuration

Use the `celer configure` command to switch projects:

```bash
celer configure --project project_001
```

Or modify the project configuration in `celer.toml`:

```toml
project = "project_001"
```

### View Project Dependencies

View the dependency tree of the current project:

```bash
celer tree project_001
```

---

## 📚 Related Documentation

- [Quick Start Guide](./quick_start.md) - Get started with Celer
- [Create Project](./cmd_create.md) - Using the celer create command
- [Platform Configuration](./advance_platform.md) - Configure compilation toolchains

---

**Need Help?** [Report Issues](https://github.com/celer-pkg/celer/issues) or check our [Documentation](../../README.md)
