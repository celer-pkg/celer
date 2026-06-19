# Python Version Management

> **Automatically manage Python versions for build dependencies across different platforms and configurations**

## 🎯 What is Python Version Management?

Python is increasingly used as a build-time dependency for C/C++ projects. Many modern libraries (like Boost, CMake plugins, and various build tools) require Python during compilation. Celer provides intelligent Python version management to:

- 📦 **Project-Specific Python** - Configure Python versions per-project
- 🔄 **Automatic Fallback** - Use system Python when versions match, or conda when they don't
- 🪟 **Platform-Aware** - Different strategies for Windows vs Linux/macOS
- 🎯 **Virtual Environments** - Isolate Python packages per version
- 🔗 **Seamless Integration** - Automatic setup when dependencies require Python

**Why Do You Need Python Version Management?**

- 🚫 **Dependency Conflicts** - Different projects may require different Python versions
- 🔧 **Build Tool Compatibility** - Build tools need specific Python minor versions
- 🌍 **Cross-Platform** - System Python availability and versions vary across platforms
- 📦 **Package Isolation** - Prevent conflicts between project dependencies

---

## 🔧 How Python Version Selection Works

### Decision Tree

Celer uses a smart decision tree to select which Python to use:

```
Is Python version specified in project config?
│
├─ YES → Does system Python version match? 
│        │
│        ├─ YES → Use system Python ✅
│        └─ NO  → Use conda Python 📥
│
└─ NO  → Is there a default Python configured?
         │
         ├─ YES (Windows) → Use default Python from buildtools/static 🔧
         ├─ YES (Linux) → Use system Python if available 🖥️
         └─ NO → Error: No Python version available ❌
```

### Platform-Specific Behavior

#### Windows

**Default Python Source:**
- Celer reads the default Python version from `buildtools/static/x86_64-windows.toml`
- This file specifies a pre-configured Python version bundled with Celer
- On first use, Celer downloads and sets up this Python version

**Python Resolution:**
```
1. Check if project specifies python_version
   ├─ If YES, compare with default Windows Python
   │  ├─ If versions match → Use system Python
   │  └─ If versions differ → Download via conda
   └─ If NO, use default Windows Python version
```

**Virtual Environment Storage:**
```
{workspace}/installed/venv-{minor_version}@{project_name}/
├── Scripts/
│   ├── python.exe
│   ├── pip.exe
│   └── ... other tools
└── Lib/
    └── site-packages/
```

#### Linux/macOS

**Default Python Source:**
- Celer attempts to detect system Python3 version
- If no system Python, falls back to conda

**Python Resolution:**
```
1. Detect system Python3 version
2. Check if project specifies python_version
   ├─ If YES, compare with system Python
   │  ├─ If versions match → Use system Python
   │  └─ If versions differ → Download via conda
   └─ If NO, use detected system Python version
```

**Virtual Environment Storage:**
```
{workspace}/installed/venv-{minor_version}@{project_name}/
├── bin/
│   ├── python3
│   ├── pip
│   └── ... other tools
└── lib/
    └── python{version}/site-packages/
```

---

## 📝 Configuring Python Version in Projects

### Project Configuration File Format

Add `python_version` to your project configuration file:

```toml
# conf/projects/project_xxx.toml

# Specify Python version (optional)
python_version = "3.11.5"

# Other project configuration
ports = [
    "boost@1.83.0",
    "openssl@3.0.1",
    "zlib@1.3.1"
]

vars = [
    "CMAKE_VAR1=value1"
]
```

### Version Specification Format

Python versions should be specified in the following formats:

- **Full version:** `3.11.5` - Uses exact version 3.11.5
- **Minor version:** `3.11` - Uses any patch version of 3.11.x
- **Major version:** `3` - Uses any version of Python 3 (not recommended)

**Examples:**
```toml
python_version = "3.11.5"      # Exact version
python_version = "3.10.12"     # Another exact version
python_version = "3.12"        # Minor version
```

### Default Behavior When Not Specified

If `python_version` is not specified in your project:

**On Windows:**
- Uses the default Python version from `buildtools/static/x86_64-windows.toml`
- Example: If the static config specifies Python 3.9.0, all projects without explicit python_version will use 3.9.0

**On Linux/macOS:**
- Attempts to use detected system Python3
- Falls back to conda if system Python cannot be detected

---

## 📦 Installing Python Packages

Celer supports installing pure Python packages into the virtual environment using `build_system = "custom"`. This follows the same lifecycle as C++ packages: **staging → venv → trace → remove**.

### Port TOML Template

```toml
[package]
  url = "https://example.com/my_python_pkg.git"
  ref = "v1.0.0"

[[build_configs]]
  build_system = "custom"
  build_in_source = true
  build = ["${PYTHON_VENV_EXE} setup.py build"]
  install = [
    "cmake -E make_directory ${PACKAGE_DIR}",
    "${PYTHON_VENV_EXE} setup.py install --prefix=${PACKAGE_DIR} --single-version-externally-managed --record=${PACKAGE_DIR}/install.log",
  ]
```

### Available Placeholders

| Placeholder | Description |
| --- | --- |
| `${PYTHON_VENV_EXE}` | Absolute path to the venv's `python3` executable. |
| `${PYTHON_VENV_DIR}` | Absolute path to the venv root directory. |
| `${PACKAGE_DIR}` | Staging directory for the package (like C++ packages). |

### How It Works

1. **Build**: Runs `${PYTHON_VENV_EXE} setup.py build` in the source directory.
2. **Install**: Runs `setup.py install --prefix=${PACKAGE_DIR}` to stage files into the package directory.
3. **Copy**: Celer copies staged files from `${PACKAGE_DIR}` to the virtual environment.
4. **Trace**: A trace file is written recording all installed files (relative to `installed/`).
5. **Remove**: `celer remove` reads the trace and deletes the files from the venv.

### Auto-Detection

When a `build_system = "custom"` port references `${PYTHON_VENV_EXE}` or `${PYTHON_VENV_DIR}` in its commands, Celer automatically:

- Triggers Python virtual environment setup (no need to declare `python3` in `build_tools`).
- Copies installed files to the venv instead of `InstalledDir`.
- Writes a venv-aware trace file for later removal.

### Example: ament_package

```toml
[package]
  url = "https://github.com/ament/ament_package.git"
  ref = "humble"

[[build_configs]]
  build_system = "custom"
  build_in_source = true
  build = ["${PYTHON_VENV_EXE} setup.py build"]
  install = [
    "cmake -E make_directory ${PACKAGE_DIR}",
    "${PYTHON_VENV_EXE} setup.py install --prefix=${PACKAGE_DIR} --single-version-externally-managed --record=${PACKAGE_DIR}/install.log",
  ]
```

After installation, the package is available in the venv's `site-packages` and can be removed with `celer remove ament_package@humble`.
