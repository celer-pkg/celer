# Project Configuration

> **Define independent build environments and dependencies for different projects**

## What is Project Configuration?

Project configuration defines how Celer manages dependencies and build environments for a specific project. Each project configuration contains six core components:

- **target_platform** - Project target deployment platform. In a new workspace, when the platform is set via the configure command, it will be automatically configured.
- **Ports (Dependencies)** - Sub-dependencies required by the project. It is recommended to configure sub-project dependencies, as third-party libraries are basically dependent on sub-projects.
- **Vars (CMake Variables)** - Global CMake build variables needed during build
- **Envs (Environment Variables)** - Global environment variables needed during build
- **Macros (Macro Definitions)** - Global C/C++ preprocessor macros needed during build

**Why do we need project configuration?**

Each project has its unique configuration characteristics. Project configuration allows Celer to:
- Manage project dependencies uniformly
- Share consistent build environments across teams
- Quickly switch between different project build configurations
- Independently manage optimization strategy and macro definitions for each project

**Project File Location:** All project configuration files are stored in the `conf/projects` directory.

---

## Configuration Field Details

### Complete Example Configuration

Let's look at a complete project configuration file `project_xxx.toml`:

```toml
target_platform = "aarch64-linux-ubuntu-22.04-gcc-11.5.0"

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

macros = [
  "MICRO_VAR1=111",
  "MICRO_VAR2"
]
```

### Configuration Field Descriptions

| Field | Required | Description | Example |
|------|------|------|------|
| `target_platform` | ❌ | Project target deployment platform, must be a platform that exists in conf/platform | `aarch64-linux-ubuntu-22.04-gcc-11.5.0` |
| `ports` | ❌ | Define third-party libraries the current project depends on. Format: `package@version` | `["x264@stable", "zlib@1.3.1"]` |
| `vars` | ❌ | Define global CMake variables required by the current project. Format: `variable=value` | `["CMAKE_BUILD_TYPE=Release"]` |
| `envs` | ❌ | Define global environment variables required by the current project. Format: `variable=value` | `["xorg_cv_malloc0_returns_null=yes"]` |
| `macros` | ❌ | Define C/C++ macro definitions required by the current project. Format: `macro=value` or `macro` | `["DEBUG=1", "ENABLE_LOGGING"]` |

> **Note**: All fields are optional. You can configure them selectively based on project needs.

### 1. Ports (Dependencies)

Specify third-party libraries that the project depends on. Celer will automatically download, compile, and install these dependencies.

**Format:** `"package@version"`

**Example:**
```toml
ports = [
  "zlib@1.3.1",
  "openssl@3.0.0",
  "sqlite3@3.49.0",
  "x264@stable"
]
```

**Version Notes:**
- Specify exact version: `@3.49.0`
- Use specific tag: `@stable`, `@latest`
- Version format must match the versions defined in the `ports` directory

> **Tip**: Use `celer search <package>` to view available version lists.

### 2. Vars (CMake Variables)

Define global CMake variables that will be passed to the build process of all dependent libraries and app development projects.

**Format:** `"variable=value"`

**Example:**
```toml
vars = [
  "PROJECT_NAME=telsa/model3",
  "PROJECT_CODE=0033FF"
]
```

### 3. Envs (Environment Variables)

Define environment variables needed during build, affecting compilation behavior.

**Format:** `"variable=value"`

**Example:**
```toml
envs = [
  "PKG_CONFIG_PATH=/opt/sdk/pkgconfig",
  "QNX_CONFIGURATION=/opt/qnx/.qnx"
]
```

### 4. Macros (Macro Definitions)

Define C/C++ preprocessor macros to be injected into code during compilation.

**Format:** `"macro=value"` or `"macro"` (value-less macro)

**Example:**
```toml
macros = [
  "DEBUG=1",              # Enable debug mode (macro with value)
  "ENABLE_LOGGING",       # Enable logging (value-less macro)
  "MAX_BUFFER_SIZE=4096", # Define buffer size
  "_GNU_SOURCE"           # Enable GNU extensions
]
```
---

## Using Project Configuration

### Create New Project

Use the `celer create` command to create a new project based on project configuration:

```bash
# Use specified project configuration
celer create --project=x86_64-linux-ubuntu-22.04-gcc-11.5.0
```

### Switch Project Configuration

Use the `celer configure` command to switch projects:

```bash
celer configure --project=project_001
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