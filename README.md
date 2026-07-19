<div align="center">

# Celer

**A lightweight, non-intrusive, delivery-oriented C/C++ package manager centered on CMake**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Go Report Card](https://goreportcard.com/badge/github.com/celer-pkg/celer)](https://goreportcard.com/report/github.com/celer-pkg/celer)
[![GitHub release](https://img.shields.io/github/release/celer-pkg/celer.svg)](https://github.com/celer-pkg/celer/releases)

[English](./README.md) | [🌍 中文](./docs/zh-CN/README.md)

</div>

---

> **Celer focuses on delivery efficiency, cross-compilation, and team collaboration — not on generic ecosystem size.**

## 🚀 Quick Start

```bash
# 1. Install
git clone https://github.com/celer-pkg/celer.git
cd celer && go build

# 2. Init with a config repo
celer init --url=https://github.com/celer-pkg/test-conf.git

# 3. Choose your platform & project
celer configure --platform=x86_64-linux-ubuntu-22.04-gcc-11.5.0
celer configure --project=project_test_01

# 4. Install a library
celer install glog@0.6.0
```

📖 [Full Quick Start Guide](./docs/en-US/quick_start.md)

## 🎯 Who Is This For?

Celer is built for teams facing **real-world C/C++ delivery challenges**:

- You maintain projects across **multiple platforms** (Windows, Linux, embedded) with **different compilers** (MSVC, GCC, Clang).
- You need to **hand off reproducible builds** to teammates or CI — without "works on my machine".
- You ship **private binaries** or maintain internal artifact repositories.
- You're tired of dependency version conflicts leaking across projects.

If you just need to fetch an open-source library, [Conan](https://conan.io) / [vcpkg](https://vcpkg.io) / [XMake](https://xmake.io) are mature options. Celer is for when **delivery consistency** matters more than ecosystem breadth.

## 💡 How It Works

![workflow](./docs/assets/workflow.svg)

Celer generates a `toolchain_file.cmake` from your platform and project config. Your CMake project stays untouched — dependencies, toolchains, env vars, and build flags are injected externally.

- ✅ Keep your existing CMake structure
- ✅ Dependencies and platform config live outside your codebase
- ✅ The generated toolchain file is self-contained — share it for CI, handoff, or bug reproduction

## 🖥️ Platform & Compiler Support

Celer aims to provide first-class cross-compilation support across major platforms and compilers. The matrix below shows the current status:

|              | 💻 **Windows** | 🐧 **Linux** | 🍎 **macOS** |
| ------------ | :---: | :---: | :---: |
| **MSVC**     | ✅   | —    | —    |
| **Clang-CL** | ✅   | —    | —    |
| **Clang**    | ✅   | ✅   | 🚧   |
| **GCC**      | —    | ✅   | 🚧   |

> ✅ = Supported &nbsp;&nbsp; 🚧 = Planned &nbsp;&nbsp; — = Not applicable

## 🌟 Features at a Glance

| When you need… | Celer handles it |
| --- | --- |
| **Cross-compilation** | Unified TOML config for toolchains, sysroots, env vars across ARM / x86 / QNX / Windows / Linux |
| **Project isolation** | Per-project dependency versions, macros, and CMake variables — no global leaks |
| **Multi-buildsystem** | Native support for CMake, Makefiles, Meson, B2, QMake, GYP |
| **Binary distribution** | Hash-based artifact cache for private libraries and prebuilt packages |
| **Air-gapped / offline** | Repo cache reuses source trees when GitHub/GitLab access is unavailable |
| **Reproducible CI** | Export workspace snapshots; config flows directly into pipelines |
| **Embedded / MCU** | `embedded_system` support without depending on a traditional OS runtime |

## 🆚 How Celer Compares

| Dimension | Conan / vcpkg / XMake | ✅ Celer |
| --- | --- | --- |
| **Intrusion** | Requires adapting recipes or ecosystem conventions | Integrates via `toolchain_file.cmake` — your project stays unchanged |
| **Cross-compile** | Toolchains, profiles, triplets assembled separately | One unified config: platform + toolchain + deps + env |
| **Project isolation** | Shared config risks version conflicts | Dependencies scoped per project |
| **Multi-project** | Often wired one project at a time | One config coordinates multiple subprojects |
| **Private binaries** | Extra packaging glue needed | Built for internal artifact repos and custom delivery |
| **Caching** | Less focused on team-wide reuse | Hash-based caching for team-wide build stability |
| **Reproduction** | Users must understand full local toolchain stack | Self-contained toolchain files and snapshots |

## 📚 Documentation

**Get started in 5 minutes:**
- [Quick Start Guide](./docs/en-US/quick_start.md) · [Init a Workspace](./docs/en-US/quick_start.md#3-setup-conf)
- [Create a Platform / Project / Port](./docs/en-US/cmd_create.md)
- [Install a Library](./docs/en-US/cmd_install.md) · [Deploy](./docs/en-US/cmd_deploy.md)

**Deep dives:**
- [Generate CMake Configs for Prebuilts](./docs/en-US/article_generate_cmake_config.md)
- [PkgCache: Shared Cache & NFS](./docs/en-US/article_pkgcache.md) · [Artifact Cache](./docs/en-US/article_pkgcache_artifacts.md) · [Repo Cache](./docs/en-US/article_pkgcache_repos.md) · [Download Cache](./docs/en-US/article_pkgcache_downloads.md)
- [CCache Integration](./docs/en-US/article_ccache.md) · [CUDA Detection](./docs/en-US/article_cuda_support.md)
- [Expression Variables](./docs/en-US/article_expvars.md) · [Dependency Conflict Detection](./docs/en-US/article_detect_conflict_circular.md)
- [Python Version Management](./docs/en-US/article_python_management.md) · [Build Tools](./docs/en-US/article_build_tools.md)
- [Export Snapshots](./docs/en-US/cmd_deploy_snapshot.md)

**Reference:**
- [All Commands](./docs/en-US/cmd_configure.md) — `configure` · `install` · `remove` · `update` · `search` · `tree` · `clean` · `autoremove` · `reverse` · `integrate` · `version`

## 🤝 Contributing

Contributions are welcome in both the core and ports:

- **[celer](https://github.com/celer-pkg/celer)** — core package manager
- **[ports](https://github.com/celer-pkg/ports)** — port definitions & build configs

## 📄 License

MIT. See [LICENSE](./LICENSE). Third-party libraries in `ports/` remain under their original licenses.

---

<div align="center">

**Built for complex C/C++ engineering delivery**

[⭐ Star us on GitHub](https://github.com/celer-pkg/celer) | [📖 Documentation](./docs/en-US/quick_start.md) | [🐛 Report Issues](https://github.com/celer-pkg/celer/issues)

</div>