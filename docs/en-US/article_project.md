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

---

## Project-local Ports

&emsp;&emsp;Besides depending on ports from the global `ports/` repository, a project may also keep ports inside its own directory — to override global versions or maintain project-specific ports. A port is looked up in three locations, in priority order:

1. **Project top-level**: `conf/projects/<project>/<lib>/<version>/port.toml`
2. **Project vendor dir**: `conf/projects/<project>/ports/<lib>/<version>/port.toml`
3. **Global ports repo**: `ports/<first-letter>/<lib>/<version>/port.toml`

&emsp;&emsp;**Locations 1 and 2 are mutually exclusive**: if the same `<lib>@<version>` appears in both the project top-level and the `ports/` subdir, Celer errors out naming both conflicting paths — you must remove one. Location 3 is the fallback used when neither project-local layout has the port.

### Top-level vs vendor dir

- **Top-level**: project-owned ports (business components, internal libraries). Opening the project directory, these are immediately visible as "ours".
- **`ports/` subdir**: third-party ports pulled in or forked from the global repo, kept physically separate from project-owned ports so it's easy to tell "which ones are imported".

&emsp;&emsp;Both layouts are optional — using only top-level, only `ports/`, or a mix all work the same.

### Example

```
conf/projects/my_project/
├── my_project.toml
├── algorithm_base/          # project-owned port (top-level)
│   └── 1.0.0/port.toml
└── ports/                   # imported third-party ports (vendor)
    ├── boost/
    │   └── 1.82.0/port.toml
    └── eigen/
        └── 3.4.0/port.toml
```

&emsp;&emsp;A project-local port overrides the global one of the same name — e.g. if both global `ports/b/boost/1.82.0/` and project `ports/boost/1.82.0/` exist, the project version wins (useful for patching a third-party lib or pinning a different ref).