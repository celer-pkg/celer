
# 🚀 Deploy Command

> **One-click deployment of all third-party library dependencies and generate toolchain configuration file**

&emsp;&emsp;The `deploy` command performs a complete build and deployment cycle for all required third-party libraries based on the current platform and project configuration. After deployment, it automatically generates a `toolchain_file.cmake` file for seamless integration with CMake-based projects.

## 📝 Command Syntax

```shell
celer deploy
```

## 🔄 Execution Flow

The `deploy` command executes in the following order:

### 1️⃣ Initialization and Configuration Check
- Read `celer.toml` global configuration
- Load selected platform configuration (`conf/platforms/`)
- Load selected project configuration (`conf/projects/`)
- Verify configuration file integrity

### 2️⃣ Platform Environment Setup
- Prepare target platform toolchain environment
- Configure cross-compilation toolchain (if applicable)
- Set up compilers, linkers, and other build tools
- Initialize system root directory (sysroot)

### 3️⃣ Dependency Check
- **Circular Dependency Detection**: Check all ports configured in the project and their dependency tree to ensure no circular dependencies exist
- **Version Conflict Detection**: Check if multiple ports depend on different versions of the same library to avoid version conflicts

### 4️⃣ Automated Build and Installation
- Install all ports configured in the project one by one in dependency order
- Automatically download source code (if not cached)
- Apply patches
- Execute configuration (configure)
- Build compilation (build)
- Install to `installed/` directory
- Package to `packages/` directory

### 5️⃣ Generate Toolchain Configuration File
- Generate `toolchain_file.cmake` in the project root directory
- Include all installed libraries' header file paths and library file paths
- Configure cross-compilation toolchain information (if applicable)

## ✅ After Successful Deployment

When deployment succeeds, the `toolchain_file.cmake` file will be generated in the project root directory. You can use it to develop your project with any CMake-based IDE:

- **Visual Studio** - CMake project support
- **CLion** - Native CMake support
- **Qt Creator** - CMake toolchain integration
- **Visual Studio Code** - CMake Tools extension

### Using in CMake Projects

```cmake
# Specify toolchain file in CMakeLists.txt
cmake_minimum_required(VERSION 3.15)

# Method 1: Set in CMakeLists.txt
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/toolchain_file.cmake")

project(YourProject)
```

Or specify on the command line:

```shell
cmake -DCMAKE_TOOLCHAIN_FILE=toolchain_file.cmake -B build
cmake --build build
```

## ⚠️ Notes

1. **Ensure Complete Configuration**: Before executing `deploy`, make sure you have configured the platform and project via `celer configure`
2. **Dependency Check**: If circular dependencies or version conflicts exist, deployment will fail with error messages
3. **Build Time**: First deployment may take a long time as it needs to download and compile all dependency libraries
4. **Disk Space**: Ensure sufficient disk space for source code, build directories, and installation files

---

## 📚 Related Documentation

- [Quick Start](./quick_start.md)
- [Configure Command](./cmd_configure.md) - Configure platform and project
- [Install Command](./cmd_install.md) - Install individual ports
- [Project Configuration](./article_project.md) - Configure project dependencies
- [Platform Configuration](./article_platform.md) - Configure target platform

---

**Need Help?** [Report an Issue](https://github.com/celer-pkg/celer/issues) or check our [Documentation](../../README.md)