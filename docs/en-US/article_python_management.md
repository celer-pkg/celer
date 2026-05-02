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
{workspace}/installed/venv-{minor_version}/
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
{workspace}/installed/venv-{minor_version}/
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
