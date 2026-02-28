# Expression Variables

> **All dynamic variables supported by Celer and where they can be used**

## Syntax

- `${VAR}`: recommended form.
- `$VAR`: also supported.
- `$ENV{NAME}`: read from process environment variables.
- `~`: expanded to value of `$ENV{HOME}`.

## Built-in Variables

| Variable | Source | Notes |
| --- | --- | --- |
| `${BUILDTREES_DIR}` | Celer context | Buildtrees root directory. |
| `${INSTALLED_DIR}` | Celer context | Installed directory for current platform/project/build type. |
| `${INSTALLED_DEV_DIR}` | Celer context | Installed dev directory for host tools. |

## Platform Variables

These are injected after platform loading.

| Variable | Source | Notes |
| --- | --- | --- |
| `${HOST}` | `toolchain.host` | Target host triple. |
| `${SYSTEM_NAME}` | platform toolchain metadata | System selector variable used by configs. |
| `${SYSTEM_PROCESSOR}` | `toolchain.system_processor` | CPU architecture selector. |
| `${CROSSTOOL_PREFIX}` | `toolchain.crosstool_prefix` | Prefix for toolchain executables. |
| `${TOOLCHAIN_DIR}` | toolchain root dir | Toolchain root directory used by platform expression expansion. |
| `${SYSROOT_DIR}` | `rootfs` | Available only when rootfs is configured. |

## Optional Global Variables

| Variable | Source | Notes |
| --- | --- | --- |
| `${PYTHON3_PATH}` | detected Python3 | Available when Python3 is detected. |
| `${LLVM_CONFIG}` | detected LLVM | Available when LLVM is detected. |

## Port Build Variables

These are injected per matched `build_config` before build execution.

| Variable | Source | Notes |
| --- | --- | --- |
| `${REPO_DIR}` | current port | Port source repo path. |
| `${SRC_DIR}` | current port | Port source extraction path. |
| `${BUILD_DIR}` | current port | Port build directory. |
| `${PACKAGE_DIR}` | current port | Port package output directory. |
| `${DEPS_DEV_DIR}` | workspace tmp deps | Host dev dependency directory. |
| `${DEPS_DIR}` | workspace tmp deps | Dependency directory for current mode (dev or target). |

## Where Replacement Happens

- `toolchain.envs` in platform TOML.
- `project.vars`, `project.envs`, `project.macros`, `project.flags`.
- Port `build_config` fields such as `envs` and `options`.

## Extra Placeholders in Build Options

These are not part of `ExprVars`, but are also replaced during build option expansion:

- `${CC}`
- `${CXX}`

