<div align="center">

# Celer

**A lightweight, non-intrusive C/C++ package manager for CMake projects**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Go Report Card](https://goreportcard.com/badge/github.com/celer-pkg/celer)](https://goreportcard.com/report/github.com/celer-pkg/celer)
[![GitHub release](https://img.shields.io/github/release/celer-pkg/celer.svg)](https://github.com/celer-pkg/celer/releases)

[English](./README.md) | [🌍 中文](./docs/zh-CN/README.md)

</div>

---

## ✨ Why Celer?

Celer is the C/C++ accelerator that solves real-world dependency management challenges:

- 🎯 **Zero Project Intrusion** - Just one `toolchain_file.cmake`, no code changes needed
- 🚀 **True Cross-Compilation** - Platform-aware dependency management with pre-configured toolchains
- 📦 **Package Caching** - Hash-based package artifact caching saves hours of build time
- 🔧 **Multi-Build System** - Native support for CMake, Make, Meson, B2, QMake, GYP
- 🏢 **Enterprise Ready** - Project-level configurations prevent dependency version conflicts
- 🔗 **Non-Intrusive Design** - Portable `toolchain_file.cmake` works standalone after generation

## 🚀 Quick Start

```bash
# 1. Install Celer (or download pre-built package from releases)
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

📖 [Full Quick Start Guide](./docs/en-US/quick_start.md)

## 💡 How It Works

![workflow](./docs/assets/workflow.svg)

Celer generates a platform-specific `toolchain_file.cmake` that bridges your project to pre-configured build environments and dependencies. This keeps Celer completely decoupled from your project - once the toolchain file is generated, you can share it with your team and they don't even need Celer installed!

## 🌟 Key Features

| Feature | What You Get |
| --- | --- |
| **🔧 Configurable Cross-Compilation Platforms** | Pre-define toolchains for ARM, x86, RISC-V, Windows, Linux, and more with friendly TOML configurations. |
| **🎮 Embedded System Support** | First-class support for MCU and bare-metal environments with `embedded_system` flag - no OS dependencies required. |
| **📁 Project-Level Dependency Management** | Each project maintains its own dependency versions, environment variables, macros, and CMake variables - preventing global conflicts. |
| **🛠️ Multi-Build System Support** | Native support for **CMake**, **Makefiles**, **Meson**, **B2**, **QMake**, **GYP** - no need to write complex scripts. |
| **📦 Auto CMake Config Generation** | Prebuilt libraries automatically get CMake config files generated, ensuring seamless integration. |
| **⚡ Intelligent Package Caching** | Hash-based artifact caching via local network shares eliminates redundant builds. Supports pre-built package distribution for private libraries. |
| **💻 Developer Mode** | Generate `toolchain_file.cmake` once with `celer deploy`, then use any IDE for development. |
| **🔄 CI/CD Integration** | Configure projects in `conf/projects` for seamless continuous integration pipelines. |
| 📸 **Workspace Snapshot** | A reproducible workspace snapshot makes it easy to fix bugs and add features. |

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

📖 [Compare and learn: how celer solved problems.](./docs/en-US/why_celer.md)

## 📚 Documentation

**Quick Start:**
- [Quick Start Guide](./docs/en-US/quick_start.md) - Get started in 5 minutes
- [Create a New Platform](./docs/en-US/cmd_create.md#1-create-a-new-platform) - Define custom cross-compilation environments
- [Create a New Project](./docs/en-US/cmd_create.md#2-create-a-new-project) - Configure project-specific settings
- [Add a New Port](./docs/en-US/cmd_create.md#3-create-a-new-port) - Host your own libraries

**Advanced Features:**
- [Generate CMake Configs](./docs/en-US/article_generate_cmake_config.md) - Auto-generate configs for prebuilt libraries
- [Caching Build Artifacts](./docs/en-US/article_pkgcache_artifacts.md) - Accelerate integration by reusing built artifacts for each library
- [Caching Source Repositories](./docs/en-US/article_pkgcache_repos.md) - Reuse source trees through repo cache when upstream access is unstable
- [Support CCache](./docs/en-US/article_ccache.md) - Speeds up recompilation by caching previous compilations
- [Detect version conflict and circular dependencies](./docs/en-US/article_detect_conflict_circular.md) - Auto detect version conflict and circular dependencies before building any libraries
- [CUDA auto detect](./docs/en-US/article_cuda_support.md) - Seamless CUDA toolkit integration for GPU-accelerated projects
- [Export snapshot](./docs/en-US/cmd_deploy_export.md) - Export a reproducible workspace snapshot after deployed successfully.

## 📋 Commands

| Command                                         | Description|
| ----------------------------------------------- | -----------|
| [autoremove](./docs/en-US/cmd_autoremove.md)    | Clean installed directory, remove project not required libraries.|
| [clean](./docs/en-US/cmd_clean.md)              | Remove build cache for targets, or clean all with `--all`.|
| [configure](./docs/en-US/cmd_configure.md)      | Configure global settings for current workspace.|
| [create](./docs/en-US/cmd_create.md)            | Create a platform, project or port. |
| [deploy](./docs/en-US/cmd_deploy.md)            | Deploy with selected platform and project.|
| [init](./docs/en-US/quick_start.md#3-setup-conf)| Initialize celer with a conf repo.|
| [install](./docs/en-US/cmd_install.md)          | Install a package.|
| [integrate](./docs/en-US/cmd_integrate.md)      | Integrate tab completion.|
| [remove](./docs/en-US/cmd_remove.md)            | Remove installed packages.|
| [reverse](./docs/en-US/cmd_reverse.md)          | Reverse query dependencies on the specified library. |
| [search](./docs/en-US/cmd_search.md)            | Search available ports from ports repository.|
| [tree](./docs/en-US/cmd_tree.md)                | Show the dependencies of a port or a project.|
| [update](./docs/en-US/cmd_update.md)            | Repo mode takes no ports; port mode requires at least one `name@version`.|
| [version](./docs/en-US/cmd_version.md)          | Show version info of celer.|

## 🤝 Contributing

Celer is an open source project built with community contributions. We welcome contributions to:

- **[celer](https://github.com/celer-pkg/celer)** - Core package manager implementation
- **[ports](https://github.com/celer-pkg/ports)** - Package definitions and build configurations

Whether you want to add new features, improve documentation, or contribute new package definitions, we welcome your help!

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

Third-party libraries in the ports repository are licensed under their respective original terms.

---

<div align="center">

**Made with ❤️ for the C/C++ community**

[⭐ Star us on GitHub](https://github.com/celer-pkg/celer) | [📖 Documentation](./docs/en-US/quick_start.md) | [🐛 Report Issues](https://github.com/celer-pkg/celer/issues)

</div>
