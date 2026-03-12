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

Celer is not just another wrapper around C/C++ dependency management. It is designed to unify **dependencies, toolchains, platforms, project configuration, and delivery environments** into one reusable engineering workflow.

It is not optimized for the simple case of “download a library and link it”. It is built for the harder cases that show up in real projects:

- 🎯 **Low intrusion**: integrate through `toolchain_file.cmake` without rewriting your business code or build logic.
- 🚀 **Cross-compile&nbsp;ready**: describe compilers, sysroots, ABIs, environment variables, and dependency sources in one platform model instead of stitching them together later.
- 📦 **Artifact&nbsp;reuse**: reduce rebuild cost and environment drift with hash-based artifact reuse.
- 🔧 **Multi-build**: supports CMake, Make, Meson, B2, QMake, and GYP without forcing a single build ecosystem.
- 🏢 **Project&nbsp;isolation**: dependency versions, macros, environment variables, and CMake variables stay scoped to the project instead of leaking globally.
- 🔗 **Team&nbsp;delivery**: generated `toolchain_file.cmake` files can be shared directly with teams and CI systems without requiring every user to operate Celer locally.

## 🚀 Quick Start

```bash
# 1. Install Celer (or download a prebuilt package from releases)
git clone https://github.com/celer-pkg/celer.git
cd celer && go build

# 2. Initialize with a configuration repository
celer init --url=https://github.com/celer-pkg/test-conf.git

# 3. Configure your platform and project
celer configure --platform=x86_64-linux-ubuntu-22.04-gcc-11.5.0
celer configure --project=my_project

# 4. Deploy and generate the toolchain file
celer deploy

# 5. Use it in your CMake project
cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/workspace/toolchain_file.cmake ..
cmake --build .
```

📖 [Full Quick Start Guide](./quick_start.md)

## 💡 How It Works

![workflow](../assets/workflow.svg)

Celer generates a platform-aware `toolchain_file.cmake` from the selected platform and project configuration. That file connects your project to predefined toolchains, dependencies, environment variables, and build settings.

In practice, this means:

- your project keeps its existing CMake structure;
- dependencies and platform settings are managed externally;
- the generated toolchain file can be reused for team collaboration, CI/CD, and reproducible debugging.

## 🌟 Core Capabilities

| Capability | Value |
| --- | --- |
| **🔧 Cross-Compile** | Define toolchains and build environments for ARM, x86, RISC-V, Windows, Linux, and more through TOML configuration. |
| **🎮 Embedded** | Supports MCU and bare-metal workflows through `embedded_system`, without assuming a traditional OS runtime. |
| **📁 Project Scope** | Each project owns its dependency versions, environment variables, macros, and CMake variables, reducing multi-project conflicts. |
| **🛠️ Build Systems** | Works with **CMake**, **Makefiles**, **Meson**, **B2**, **QMake**, and **GYP** without requiring heavy adapter scripts. |
| **📦 CMake Config** | Automatically generates CMake configuration for prebuilt binaries to reduce integration friction. |
| **⚡ Artifact Cache** | Uses hash-based artifact caching for private libraries, binary delivery, and large-scale repeated builds. |
| **💻 Dev Mode** | Run `celer deploy` once, then continue development in any IDE with the generated toolchain file. |
| **🔄 CI/CD** | Platform and project configuration can flow directly into pipelines, reducing drift between developer and CI environments. |
| **📸 Snapshots** | Export reproducible workspace snapshots for debugging, traceability, and handoff. |

## 🆚 Where Celer Wins Against Other C++ Package Managers

If your problem is only “how do I fetch an open-source library”, Conan, vcpkg, and XMake already provide strong answers.

Celer is stronger where **delivery efficiency and environment consistency** matter more than generic ecosystem breadth:

| Dimension | Conan / vcpkg / XMake typical approach | ✅ Where Celer is stronger |
| --- | --- | --- |
| **Intrusion** | Often requires recipes, ports, or ecosystem-specific integration work | Integrates through `toolchain_file.cmake` with lower project intrusion |
| **Cross-compile&nbsp;model** | Toolchains, profiles, and triplets are often assembled separately | Platforms, toolchains, environment variables, and dependencies are modeled together |
| **Project&nbsp;isolation** | Shared/global configuration can become a conflict source | Dependency and build settings stay scoped at the project boundary |
| **Multi-project&nbsp;sync** | Often wired manually one project at a time | A single configuration can coordinate multiple subprojects |
| **Private&nbsp;binaries** | Usually needs extra packaging conventions and workflow glue | Better suited for enterprise artifact repositories and prebuilt internal packages |
| **Cache&nbsp;and&nbsp;rebuilds** | Caching exists, but is not always centered on engineering-level artifact reuse | Hash-based artifact caching is designed to maximize team-wide reuse and build stability |
| **Sharing&nbsp;and&nbsp;repro** | Users often need to understand the full local toolchain stack | Generated toolchain files and workspace snapshots are easier to share and reproduce |

In one sentence:

> **Celer is not primarily about winning on generic package ecosystem size. It wins on engineering delivery, cross-compilation, private dependency governance, and team workflow consistency.**

📖 [Learn more about the problems Celer is built to solve](./why_celer.md)

## 📚 Documentation

**Getting Started:**
- [Quick Start Guide](./quick_start.md) - Get started in 5 minutes
- [Create a New Platform](./cmd_create.md#1-create-a-new-platform) - Define custom cross-compilation environments
- [Create a New Project](./cmd_create.md#2-create-a-new-project) - Configure project-level settings
- [Add a New Port](./cmd_create.md#3-create-a-new-port) - Host and manage your own libraries

**Advanced Topics:**
- [Generate CMake Configs](./article_generate_cmake_config.md) - Auto-generate configuration for prebuilt binaries
- [Cache Build Artifacts](./article_pkgcache_artifacts.md) - Reuse built artifacts to reduce repeated integration cost
- [Cache Source Repositories](./article_pkgcache_repos.md) - Reuse source trees through repo cache when upstream access is unstable
- [Support CCache](./article_ccache.md) - Speed up repeated compilation by reusing compiler outputs
- [Expression Variables](./article_expvars.md) - Review the dynamic variables available in TOML configuration
- [Detect Version Conflicts and Circular Dependencies](./article_detect_conflict_circular.md) - Catch invalid dependency relationships before builds start
- [CUDA Auto Detection](./article_cuda_support.md) - Integrate CUDA toolchains for GPU-oriented projects
- [Export Snapshots](./cmd_deploy_export.md) - Export a reproducible workspace snapshot after deployment

## 📋 Commands

| Command | Description |
| --- | --- |
| [autoremove](./cmd_autoremove.md) | Clean the install directory and remove libraries no longer required by the current project |
| [clean](./cmd_clean.md) | Clean build cache for selected targets, or use `--all` to clean everything |
| [configure](./cmd_configure.md) | Update global configuration for the current workspace |
| [create](./cmd_create.md) | Create a platform, project, or port |
| [deploy](./cmd_deploy.md) | Deploy using the selected platform and project |
| [init](./quick_start.md#3-setup-conf) | Initialize Celer with a configuration repository |
| [install](./cmd_install.md) | Install a library |
| [integrate](./cmd_integrate.md) | Enable shell tab completion integration |
| [remove](./cmd_remove.md) | Remove installed libraries |
| [reverse](./cmd_reverse.md) | Query which projects or libraries depend on a given library |
| [search](./cmd_search.md) | Search available ports |
| [tree](./cmd_tree.md) | Show the dependency tree of a library or project |
| [update](./cmd_update.md) | Repo mode takes no port arguments; port mode requires at least one `name@version` |
| [version](./cmd_version.md) | Show Celer version information |

## 🤝 Contributing

Celer is an open-source project built for community collaboration. Contributions are welcome in:

- **[celer](https://github.com/celer-pkg/celer)** - the core package manager implementation
- **[ports](https://github.com/celer-pkg/ports)** - port definitions and build configurations

If you want to improve features, polish documentation, or add new ports, contributions are welcome.

## 📄 License

This project is released under the MIT License. See [LICENSE](../../LICENSE) for details.

Third-party libraries in the `ports` repository remain under their original licenses.

---

<div align="center">

**Built for complex C/C++ delivery workflows**

[⭐ Star us on GitHub](https://github.com/celer-pkg/celer) | [📖 Documentation](./quick_start.md) | [🐛 Report Issues](https://github.com/celer-pkg/celer/issues)

</div>
