# Build Tools

> **Automatically download and manage build-time toolchains and utilities across platforms**

## 🎯 What are Build Tools?

C/C++ projects often depend on a variety of build-time tools — CMake, Ninja, CCache, LLVM/Clang, Git, Python, and more. Manually installing and version-matching these tools across different platforms and team members is error-prone and time-consuming.

Celer's **Build Tools** system solves this by:

- 📦 **Built-in Tool Registry** — Celer bundles pre-configured tool definitions for common build tools, organized by platform (e.g., `x86_64-linux`, `x86_64-windows`).
- 🔧 **User Overrides** — Customize or add tools via `conf/buildtools/<arch>-<os>.toml` without modifying Celer itself.
- 🔄 **Automatic Download & Cache** — Tools are downloaded on first use and cached in the downloads directory.
- 🎯 **Version Pinning** — Each tool can have multiple versions; mark one as `default` for zero-config usage.
- 🛤️ **PATH Injection** — Tool binaries are automatically added to `PATH` during builds.
- 🌍 **Cross-Platform** — Different tool sets for Linux, Windows, and macOS, all described in the same TOML format.

**Why Do You Need Build Tools Management?**

- 🚫 **Version Drift** — Different team members may have different CMake/Ninja versions installed.
- 🔧 **Missing Dependencies** — New contributors spend hours installing build prerequisites.
- 🌍 **Platform Differences** — Linux and Windows need completely different tool sets (e.g., MSYS2 on Windows).
- 📦 **CI Reproducibility** — Pinned tool versions ensure CI and developer environments match exactly.

---

## 📂 Where Build Tools Are Defined

Celer looks up build tools from two sources, merged in order:

| Priority | Source | Location | Purpose |
|----------|--------|----------|---------|
| 1 (Low) | **Built-in** | `buildtools/static/<arch>-<os>.toml` | Celer's bundled defaults |
| 2 (High) | **User Config** | `conf/buildtools/<arch>-<os>.toml` | User overrides and additions |

> 💡 **Merge Logic**: If a tool with the same `name` and `version` exists in both sources, the user config **replaces** the built-in definition entirely. Tools with unique names or versions are appended.

**Current Built-in Tools by Platform:**

| Tool | Linux | Windows | Description |
|------|:-----:|:-------:|-------------|
| `cmake` | ✅ | ✅ | CMake build system |
| `ninja` | ✅ | ✅ | Ninja build system |
| `ccache` | ✅ | ✅ | Compiler cache |
| `git` | — | ✅ | Git for Windows (MinGit) |
| `git-lfs` | ✅ | ✅ | Git Large File Storage |
| `llvm` | ✅ | — | LLVM/Clang toolchain |
| `conda` | ✅ | ✅ | Miniforge (Python & packages) |
| `python3` | — | ✅ | Python interpreter on Windows |
| `msys2` | — | ✅ | MSYS2 environment on Windows |
| `strawberry-perl` | — | ✅ | Perl for Windows |
| `vswhere` | — | ✅ | Visual Studio locator |

---

## 📝 Configuration Format

Each build tool is defined as a `[[build_tools]]` entry in a TOML file.

### Complete Field Reference

```toml
[[build_tools]]
  name    = "cmake"            # Tool identifier (required)
  version = "4.3.2"           # Version string (required)
  default = true              # Mark as default version (optional)
  url     = "https://..."     # Download URL (required)
  sha256  = "abc123..."       # SHA-256 checksum (optional, recommended)
  archive = "cmake-4.3.2..." # Rename downloaded file (optional)
  paths   = ["cmake-4.3.2.../bin"]  # Sub-paths to add to PATH (optional)
  vars    = ["MY_VAR=value"]  # Global expression variables (optional)
  envs    = ["MY_ENV=value"]  # Environment variables to set (optional)
```

### Field Details

#### `name`
The tool identifier. Referenced as `name` or `name@version` when tools require a specific version. Must be unique among tools; use multiple `[[build_tools]]` entries with the same `name` but different `version` for multi-version support.

#### `version`
Any string that identifies the tool version. Used for version pinning and matching. When a dependency requests `"cmake"` (without version), the entry with `default = true` is selected.

#### `default`
Marks which version is used when no explicit version is specified. Only one entry per tool name should have `default = true`.

#### `url`
The download URL. Celer downloads the tool on first use and caches it.

#### `sha256`
SHA-256 hash of the downloaded file for integrity verification. Leave empty (`""`) to skip verification.

#### `archive`
Renames the downloaded file. Useful when the URL filename is generic (e.g., `download.zip`) and you want a descriptive name, or when the extracted folder name differs from the archive name.

#### `paths`
A list of sub-directories (relative to the extracted archive root) to add to the system `PATH`. Each path is joined with the tools directory.

- **With `paths`**: The archive is extracted to `downloads/tools/<first-path-component>/`, and each sub-path is added to `PATH`.
- **Without `paths`**: The tool is treated as a single-file download and placed directly in `downloads/`.

#### `vars`
Global expression variables made available via `${VAR_NAME}` in configuration expressions. Each entry is `KEY=VALUE` format. The variable name must not already be defined.

#### `envs`
Environment variables set globally when the tool is activated. Each entry is `KEY=VALUE` format. The variable name must not already exist in the current environment.

---

## 🔧 How Build Tools Are Resolved

### Tool Selection Rules

1. **Exact version match**: `"cmake@3.31.12"` matches the `cmake` entry with `version = "3.31.12"`.
2. **Default version**: `"cmake"` (without version) matches the `cmake` entry with `default = true`.
3. **Error on ambiguity**: If a tool name is found but has no `default` entry and no version was specified, Celer reports an error.
4. **System fallback (Linux only)**: Tools not found in the build tools registry are checked against system package managers.

---

## 🛠️ Customizing Build Tools

### Adding a New Tool

Create or edit `conf/buildtools/<arch>-<os>.toml` in your workspace:

```toml
# conf/buildtools/x86_64-linux.toml
[[build_tools]]
  name = "doxygen"
  version = "1.14.0"
  default = true
  url = "https://www.doxygen.nl/files/doxygen-1.14.0.linux.bin.tar.gz"
  archive = "doxygen-1.14.0.linux.bin.tar.gz"
  paths = ["doxygen-1.14.0/bin"]
  envs = ["DOXYGEN_DIR=${WORKSPACE_DIR}/downloads/tools/doxygen-1.14.0"]
  vars = ["DOXYGEN_ENABLED=true"]
```

This adds `doxygen` to the tool registry for Linux x86_64.

### Adding a Tool with Environment Variables

Some tools need environment variables set to work correctly:

```toml
[[build_tools]]
  name = "my-custom-tool"
  version = "1.0"
  default = true
  url = "https://example.com/my-tool.tar.gz"
  sha256 = "..."
  paths = ["my-tool/bin"]
  vars = ["TOOL_DIR=${root}"]
  envs = ["MY_TOOL_HOME=/path/to/tool"]
```

---

## 📋 Example Configurations

### Complete Linux Configuration

```toml
# conf/buildtools/x86_64-linux.toml

[[build_tools]]
  name = "cmake"
  version = "4.3.2"
  default = true
  url = "https://github.com/Kitware/CMake/releases/download/v4.3.2/cmake-4.3.2-linux-x86_64.tar.gz"
  sha256 = "791ae3604841ca03cb3889a3ad89165346e4b180ae3448efd4b0caa9ef46d245"
  archive = "cmake-4.3.2-linux-x86_64.tar.gz"
  paths = ["cmake-4.3.2-linux-x86_64/bin"]

[[build_tools]]
  name = "ninja"
  version = "v1.12.1"
  default = true
  url = "https://github.com/ninja-build/ninja/releases/download/v1.12.1/ninja-linux.zip"
  sha256 = "6f98805688d19672bd699fbbfa2c2cf0fc054ac3df1f0e6a47664d963d530255"
  archive = "ninja-linux-x86_64-v1.12.1.zip"
  paths = ["ninja-linux-x86_64-v1.12.1"]

[[build_tools]]
  name = "ccache"
  version = "4.12.1"
  default = true
  url = "https://github.com/ccache/ccache/releases/download/v4.12.1/ccache-4.12.1-linux-x86_64.tar.xz"
  sha256 = "742e6a6e17c0a060046874eece2949b221c228e1119698a4c6e0b096cbc87152"
  archive = "ccache-4.12.1-linux-x86_64.tar.xz"
  paths = ["ccache-4.12.1-linux-x86_64"]
```

### Complete Windows Configuration

```toml
# conf/buildtools/x86_64-windows.toml

[[build_tools]]
  name = "cmake"
  version = "4.3.2"
  default = true
  url = "https://github.com/Kitware/CMake/releases/download/v4.3.2/cmake-4.3.2-windows-x86_64.zip"
  sha256 = "83d20c23f5c5f64b3b328785e35b23c532e33057a97ed6294acaca3781b78a01"
  archive = "cmake-4.3.2-windows-x86_64.zip"
  paths = ["cmake-4.3.2-windows-x86_64/bin"]

[[build_tools]]
  name = "ninja"
  version = "v1.12.1"
  default = true
  url = "https://github.com/ninja-build/ninja/releases/download/v1.12.1/ninja-win.zip"
  sha256 = "f550fec705b6d6ff58f2db3c374c2277a37691678d6aba463adcbb129108467a"
  archive = "ninja-v1.12.1-windows-x86_64.zip"
  paths = ["ninja-v1.12.1-windows-x86_64"]

[[build_tools]]
  name = "git"
  version = "2.49.0"
  default = true
  url = "https://github.com/git-for-windows/git/releases/download/v2.49.0.windows.1/MinGit-2.49.0-64-bit.zip"
  sha256 = "971cdee7c0feaa1e41369c46da88d1000a24e79a6f50191c820100338fb7eca5"
  archive = "MinGit-2.49.0-64-bit.zip"
  paths = ["MinGit-2.49.0-64-bit/cmd"]

[[build_tools]]
  name = "msys2"
  version = "2.16.03"
  default = true
  url = "https://github.com/msys2/msys2-installer/releases/download/2025-02-21/msys2-base-x86_64-20250221.tar.xz"
  sha256 = "850589091e731d14b234447084737ca62aee1cc1e3c10be62fcdc12b8263d70b"
  archive = "msys2-base-x86_64-20250221.tar.xz"
  paths = [
    "msys2-base-x86_64-20250221/msys64/usr/bin",
    "msys2-base-x86_64-20250221/msys64/bin",
  ]
```
