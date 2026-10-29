# Project Configuration

> **Define independent build environments and dependencies for different projects**

## What is Project Configuration?

Project configuration defines how Celer manages dependencies and build environments for a specific project. Each project configuration contains the following core components:

- **ports_baseline** (required) - The ports repo revision this project is built against. A commit hash locks it; a branch name follows that branch. `celer configure --project` checks the ports repo out at this baseline, so the ports of a project never drift.
- **target_platform** - Project target deployment platform. In a new workspace, when the platform is set via the configure command, it will be automatically configured.
- **Ports (Dependencies)** - Sub-dependencies required by the project. It is recommended to configure sub-project dependencies, as third-party libraries are basically dependent on sub-projects.
- **Vars (CMake Variables)** - Global CMake build variables needed during build
- **Envs (Environment Variables)** - Global environment variables needed during build
- **Macros (Macro Definitions)** - Global C/C++ preprocessor macros needed during build
- **python (Python build environment)** - Python version and pip index sources for the build, see [Python Version Management](./article_python_management.md)

**Why do we need project configuration?**

Each project has its unique configuration characteristics. Project configuration allows Celer to:
- Manage project dependencies uniformly
- Share consistent build environments across teams
- Quickly switch between different project build configurations
- Independently manage optimization strategy and macro definitions for each project

**Project File Location:** All project configuration files are stored in the `conf/projects` directory.

**No project needed to start:** when `celer.toml` names no project, celer uses a built-in
one called `default`. It lists no ports and pins no baseline, so `celer install <name@version>`
works right after `celer init` — packages simply land under
`installed/<platform>/default/<build_type>`. Configure a real project when you want a fixed
dependency list and a pinned ports repo.

---

## Configuration Field Details

### Complete Example Configuration

Let's look at a complete project configuration file `project_xxx.toml`:

```toml
ports_baseline = "9c1f3a2b8e4d5f60718293a4b5c6d7e8f90a1b2c"
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

# Python configuration (optional; the table must come last)
[python]
  version = "3.11.5"
  index_url = "https://pypi.example.com/simple"
```

### Configuration Field Descriptions

| Field | Required | Description | Example |
|------|------|------|------|
| `ports_baseline` | ✅ | Ports repo revision this project is built against: a commit hash to lock it, or a branch name to follow that branch. Never empty. | `9c1f3a2b8e4d5f60718293a4b5c6d7e8f90a1b2c` |
| `target_platform` | ❌ | Project target deployment platform, must be a platform that exists in conf/platform | `aarch64-linux-ubuntu-22.04-gcc-11.5.0` |
| `ports` | ❌ | Define third-party libraries the current project depends on. Format: `package@version` | `["x264@stable", "zlib@1.3.1"]` |
| `vars` | ❌ | Define global CMake variables required by the current project. Format: `variable=value` | `["CMAKE_BUILD_TYPE=Release"]` |
| `envs` | ❌ | Define global environment variables required by the current project. Format: `variable=value` | `["xorg_cv_malloc0_returns_null=yes"]` |
| `macros` | ❌ | Define C/C++ macro definitions required by the current project. Format: `macro=value` or `macro` | `["DEBUG=1", "ENABLE_LOGGING"]` |
| `python` | ❌ | Python build environment for the project (version and pip indexes), see below | `[python]` table |

> **Note**: `ports_baseline` is required; every other field is optional and can be configured as the project needs.

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

### 5. features (Feature Flags)

Temporary feature toggles for controlling celer validation behavior during development
or special scenarios. These flags may change or be removed in the future.

**Format:** `"flag_name = true/false"`

**Available flags:**

| Flag | Default | Description |
|------|---------|-------------|
| `ignore_check_cmake_abs_path` | `false` | Skip checking cmake config files for absolute workspace paths (non-relocatable packages cannot reuse pkgcache) |

**Example:**
```toml
[features]
  ignore_check_cmake_abs_path = true
```

### 6. python (Python Build Environment)

Defines the Python version used at build time for this project, plus optional pip index
sources. Python configuration is project-level: it no longer lives in the workspace's
`celer.toml`, so different projects can use different Python versions and private PyPI mirrors.

**Format:** `[python]` table

**Example:**
```toml
[python]
  version = "3.11.5"
  index_url = "https://pypi.example.com/simple"
  extra_index_urls = ["https://pypi.internal/simple"]
  trusted_hosts = ["pypi.internal"]
```

| Field | Required | Description |
|------|------|------|
| `version` | ❌ | Python version (e.g. `3.11.5`) used to select/provision the virtual environment |
| `index_url` | ❌ | pip primary index URL (equivalent to `pip install -i`) |
| `extra_index_urls` | ❌ | pip extra index URLs (equivalent to `--extra-index-url`) |
| `trusted_hosts` | ❌ | pip trusted hosts (equivalent to `--trusted-host`) |

> **Note**: `[python]` is a TOML table — every key after it belongs to the table, so keep it at
> the end of the file. For version selection strategy, venv locations and fallback behavior, see
> [Python Version Management](./article_python_management.md).

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