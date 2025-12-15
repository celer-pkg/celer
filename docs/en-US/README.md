<div align="center">

# Celer

**A lightweight, non-intrusive C/C++ package manager for CMake projects**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Go Report Card](https://goreportcard.com/badge/github.com/celer-pkg/celer)](https://goreportcard.com/report/github.com/celer-pkg/celer)
[![GitHub release](https://img.shields.io/github/release/celer-pkg/celer.svg)](https://github.com/celer-pkg/celer/releases)

[English](./README.md) | [🌍 中文](../zh-CN/README.md)

</div>

---

## ✨ Why Celer?

Celer is the C/C++ accelerator that solves real-world dependency management challenges:

- 🎯 **Zero Project Intrusion** - Just one `toolchain_file.cmake`, no code changes needed
- 🚀 **True Cross-Compilation** - Platform-aware dependency management with pre-configured toolchains
- 📦 **Smart Caching** - Hash-based binary artifact caching saves hours of build time
- 🔧 **Multi-Build System** - Native support for CMake, Make, Meson, Bazel, B2, QMake, GYP
- 🏢 **Enterprise Ready** - Project-level configurations prevent dependency version conflicts
- 🔗 **Non-Intrusive Design** - Portable `toolchain_file.cmake` works standalone after generation

## 🚀 Quick Start

```bash
# 1. Install Celer (or download pre-built binary from releases)
git clone https://github.com/celer-pkg/celer.git
cd celer && go build

# 2. Initialize with configuration repository
celer init --url=https://github.com/celer-pkg/test-conf.git

# 3. Configure your platform and project
celer configure --platform=x86_64-linux-ubuntu-22.04-gcc-11.5.0
celer configure --project=my_project

# 4. Deploy and generate toolchain file
celer deploy

# 5. Use in your CMake project
cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/workspace/toolchain_file.cmake ..
cmake --build .
```

📖 [Full Quick Start Guide](./quick_start.md)

## 💡 How It Works

![workflow](../assets/workflow.svg)

Celer generates a platform-specific `toolchain_file.cmake` that bridges your project to pre-configured build environments and dependencies. This keeps Celer completely decoupled from your project - once the toolchain file is generated, you can share it with your team and they don't even need Celer installed!

## 🌟 Key Features

| Feature | What You Get |
| --- | --- |
| **🔧 Configurable Cross-Compilation Platforms** | Pre-define toolchains for ARM, x86, RISC-V, Windows, Linux, and more with friendly TOML configurations. |
| **📁 Project-Level Dependency Management** | Each project maintains its own dependency versions, environment variables, macros, and CMake variables - preventing global conflicts. |
| **🛠️ Multi-Build System Support** | Native support for **CMake**, **Makefiles**, **Meson**, **B2**, **Bazel**, **QMake**, **GYP** - no need to write complex scripts. |
| **📦 Auto CMake Config Generation** | Non-CMake libraries automatically get CMake config files generated, ensuring seamless integration. |
| **⚡ Intelligent Binary Caching** | Hash-based artifact caching via local network shares eliminates redundant builds. Supports pre-built binary distribution for private libraries. |
| **💻 Developer Mode** | Generate `toolchain_file.cmake` once with `celer deploy`, then use any IDE for development. |
| **🔄 CI/CD Integration** | Configure projects in `conf/projects` for seamless continuous integration pipelines. |

## 🆚 Celer vs Others

Celer has solved critical pain points that traditional C/C++ package managers struggle with:

| Challenge | Conan / Vcpkg / XMake | ✅ Celer |
|-----------|----------------------|---------|
| **📦 Simplified Library Integration** | Complex recipe scripts required | Just declare the build system type |
| **🏢 Project-Level Dependency Isolation** | Global configs cause conflicts | Project-level isolated configurations |
| **🔗 Platform Multi-Project Management** | Manual per-project setup | Single TOML, auto-sync sub-projects |
| **⚡ Intelligent Hash-Based Caching** | Limited or manual caching | Precision hash-based artifact caching |
| **🔍 Automatic Conflict Detection** | Runtime discovery | Build-time checks and reporting |
| **🤝 Seamless Cross-Company Collaboration** | Manual environment setup | Portable toolchain file - works out of the box |

📖 [Compare and learn: how celer solved problems.](./why_celer.md)

## 📚 Documentation

**Quick Start:**
- [Quick Start Guide](./quick_start.md) - Get started in 5 minutes
- [Create a New Platform](./cmd_create.md#1-create-a-new-platform) - Define custom cross-compilation environments
- [Create a New Project](./cmd_create.md#2-create-a-new-project) - Configure project-specific settings
- [Add a New Port](./cmd_create.md#3-create-a-new-port) - Host your own libraries

**Advanced Features:**
- [Generate CMake Configs](./article_generate_cmake_config.md) - Auto-generate configs for non-CMake libraries
- [Cache Build Artifacts](./article_binary_cache.md) - Accelerate builds with smart caching
- [Support CCache](./article_ccache.md) - Speeds up recompilation by caching previous compilations
- [Detect version conflict and circular dependencies](./article_detect_conflict_circular.md) - Auto detect version conflict and circular dependencies before building any libraries
- [Export snapshot](./cmd_deploy_export.md) - Export a reproducible workspace snapshot after deployed successfully.

## 📋 Supported Commands

| Command                               | Description |
| ------------------------------------- | ------------|
| [autoremove](./cmd_autoremove.md)     | Clean installed directory, remove project not required libraries.|
| [clean](./cmd_clean.md)               | Remove build cache and clean project repo.|
| [configure](./cmd_configure.md)       | Configure global settings for current workspace.|
| [create](./cmd_create.md)             | Create a platform, project or port. |
| [deploy](./cmd_deploy.md)             | Deploy with selected platform and project.|
| [init](./quick_start.md#3-setup-conf) | Initialize celer with a conf repo.|
| [install](./cmd_install.md)           | Install a package.|
| [integrate](./cmd_integrate.md)       | Integrate tab completion.|
| [remove](./cmd_remove.md)             | Remove installed package.|
| [reverse](./cmd_reverse.md)           | Reverse query dependencies on the specified library. |
| [search](./cmd_search.md)             | Search available ports from ports repository.|
| [tree](./cmd_tree.md)                 | Show dependencies of a port or project.|
| [update](./cmd_update.md)             | Update conf repo, ports config repo or project repo.|
| [version](./cmd_version.md)           | Show version info of celer.|

## 🤝 Contributing

Celer is an open source project built with community contributions. We welcome contributions to:

- **[celer](https://github.com/celer-pkg/celer)** - Core package manager implementation
- **[ports](https://github.com/celer-pkg/ports)** - Package definitions and build configurations

Whether you want to add new features, improve documentation, or contribute new package definitions, we welcome your help!

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](../../LICENSE) file for details.

Third-party libraries in the ports repository are licensed under their respective original terms.

---

<div align="center">

**Made with ❤️ for the C/C++ community**

[⭐ Star us on GitHub](https://github.com/celer-pkg/celer) | [📖 Documentation](./quick_start.md) | [🐛 Report Issues](https://github.com/celer-pkg/celer/issues)

</div>