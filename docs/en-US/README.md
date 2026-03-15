<div align="center">

# Celer

**A lightweight, non-intrusive, delivery-oriented C/C++ package management tool for projects centered on CMake**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Go Report Card](https://goreportcard.com/badge/github.com/celer-pkg/celer)](https://goreportcard.com/report/github.com/celer-pkg/celer)
[![GitHub release](https://img.shields.io/github/release/celer-pkg/celer.svg)](https://github.com/celer-pkg/celer/releases)

[English](./README.md) | [🌍 中文](../zh-CN/README.md)

</div>

---

## ✨ Why Celer?

Celer is designed to lower the barrier to hosting, handoff, and reuse in C/C++ projects by hiding the implementation details behind **build**, **compile**, and **install** steps. At the same time, it separates **dependency management** and **build environments** from business code. This kind of multi-dimensional decoupling makes project boundaries clearer and engineering workflows easier to manage.

In real C/C++ projects, the common pain points usually look like this:

- 🎯 **High integration cost**: existing projects often depend on specific build conventions, so taking over a project means touching code and build scripts.
- 🚀 **Complex cross-compilation environments**: compilers, sysroots, ABIs, environment variables, and dependency sources are scattered, so switching platforms and reproducing environments is expensive.
- 📦 **Built artifacts are hard to reuse**: repeated builds are common, outputs vary across machines and stages, and environment drift is easy to introduce.
- 🔧 **Third-party libraries use many build tools**: CMake, Make, Meson, B2, and QMake all need different handling.
- 🏢 **Projects contaminate each other**: dependency versions, macros, and environment variables leak globally and create conflicts.
- 🔗 **Team collaboration and delivery are difficult**: environments are tightly bound to individual machines, making handoff, collaboration, and CI rollout expensive.

## 🚀 Quick Start

```bash
# 1. Install Celer (or download a prebuilt package from releases)
git clone https://github.com/celer-pkg/celer.git
cd celer && go build

# 2. Initialize with a configuration repository
celer init --url=https://github.com/celer-pkg/test-conf.git

# 3. Configure your platform and project
celer configure --platform=x86_64-linux-ubuntu-22.04-gcc-11.5.0
celer configure --project=project_test_01

# 4. Test clone, build and install a library.
celer install glog@0.6.0
```

📖 [Full Quick Start Guide](./quick_start.md)

## 💡 How It Works

![workflow](../assets/workflow.svg)

Celer generates a platform-aware `toolchain_file.cmake` from the selected platform and project configuration. That file connects your project to predefined toolchains, dependencies, environment variables, and build settings.

That means:

- your project can keep its existing CMake structure;
- dependencies and platform configuration are managed externally in a unified way;
- once generated, the toolchain file can be reused independently for team collaboration, CI/CD, and issue reproduction.

## 🌟 Core Capabilities

| Capability | Value |
| --- | --- |
| **Cross&nbsp;Compilation** | Use TOML to describe toolchains and build environments for ARM, x86, QNX, Windows, Linux, and more in one place. |
| **Project&nbsp;Isolation** | Each project owns its own dependency versions, environment variables, macros, and CMake variables, reducing conflict risk in parallel development. |
| **Build&nbsp;Systems** | Works natively with **CMake**, **Makefiles**, **Meson**, **B2**, **QMake**, and **GYP**, reducing the cost of hosting third-party libraries. |
| **CMake&nbsp;Config** | Automatically fills in CMake configuration for prebuilt binaries, lowering the integration barrier. |
| **Artifact&nbsp;Cache** | Uses hash-based caching of build artifacts, suitable for private libraries, binary distribution, and large-scale repeated builds. |
| **Repo&nbsp;Cache** | Reuses source code by caching source repositories, reducing integration cost when external networks or GitHub/GitLab access are unavailable. |
| **Embedded** | Supports MCU and bare-metal environments through `embedded_system`, without depending on a traditional OS runtime. |
| **Development&nbsp;Mode** | After `celer deploy`, continue development directly in any IDE using the generated toolchain file. |
| **CI/CD&nbsp;Integration** | Platform and project configuration can flow directly into pipelines, reducing drift between developer and CI environments. |
| **Snapshots** | Export reproducible workspace snapshots for debugging, traceability, and handoff. |

## 🆚 Where Celer Differs from Other C++ Package Managers

If your only need is “how do I fetch an open-source library”, Conan, vcpkg, and XMake already provide mature solutions.

Celer is stronger where **delivery efficiency and consistency in complex engineering environments** matter more:

| Dimension | Conan / vcpkg / XMake common approach | ✅ Where Celer is stronger |
| --- | --- | --- |
| **Intrusion** | Often requires adapting recipes, ports, or ecosystem-specific integration | Integrates existing CMake projects through `toolchain_file.cmake` with low intrusion |
| **Cross-compile** | Toolchains, profiles, and triplets are often assembled separately | Platforms, toolchains, environment variables, and dependencies are described in one unified configuration |
| **Project&nbsp;isolation** | Shared or global configuration often becomes a conflict source | Dependencies, variables, and build settings are maintained at project scope, with clearer boundaries |
| **Multi&nbsp;project&nbsp;coordination** | Frequently wired manually one project at a time | One configuration can coordinate multiple subprojects |
| **Private&nbsp;binaries** | Usually needs extra packaging conventions and workflow glue | Better suited for internal artifact repositories, prebuilt packages, and custom delivery flows |
| **Caching&nbsp;and&nbsp;rebuilds** | Caching exists, but is not usually centered on engineering-level artifact reuse | Hash-based artifact caching emphasizes team-wide reuse and build stability |
| **Sharing&nbsp;and&nbsp;reproduction** | Users often need to understand the full local toolchain stack | Generated toolchain files and workspace snapshots are easier to share, reproduce, and hand off |

In one sentence:

> **Generic ecosystem size is not Celer's main selling point; engineering delivery, cross-compilation, private dependency governance, and team collaboration efficiency are.**

📖 [Learn more about the problems Celer is built to solve](./why_celer.md)

## 📚 Documentation

**Getting Started:**
- [Quick Start Guide](./quick_start.md) - Get started in 5 minutes
- [Create a New Platform](./cmd_create.md) - Define custom cross-compilation environments
- [Create a New Project](./cmd_create.md) - Configure project-level settings
- [Add a New Port](./cmd_create.md) - Host and manage your own libraries

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

If you want to add features, improve documentation, or contribute new port definitions, we would be glad to have your help.

## 📄 License

This project is released under the MIT License. See [LICENSE](../../LICENSE) for details.

Third-party libraries in the `ports` repository remain under their original licenses.

---

<div align="center">

**Built for complex C/C++ engineering delivery**

[⭐ Star us on GitHub](https://github.com/celer-pkg/celer) | [📖 Documentation](./quick_start.md) | [🐛 Report Issues](https://github.com/celer-pkg/celer/issues)

</div>
