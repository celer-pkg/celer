<div align="center">

# Celer

A lightweight, non-intrusive C/C++ package manager for engineering delivery, aimed at CMake-first projects.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Go Report Card](https://goreportcard.com/badge/github.com/celer-pkg/celer)](https://goreportcard.com/report/github.com/celer-pkg/celer)
[![GitHub release](https://img.shields.io/github/release/celer-pkg/celer.svg)](https://github.com/celer-pkg/celer/releases)

[English](./README.md) | [🌍 中文](./docs/zh-CN/README.md)

</div>

---

Celer installs, builds, and caches C/C++ libraries — isolated by platform × project × build type.  
The same libraries can be built for Windows MSVC, Linux aarch64, and QNX without versions leaking across targets.  
Your CMake project stays unchanged. Celer generates a `toolchain_file.cmake` outside the tree and injects the toolchain, dependencies, and flags. Hand that file to a teammate or CI and the build environment goes with it.

## 🚀 30-second start

Download a binary from [Releases](https://github.com/celer-pkg/celer/releases), or `git clone` and `go build`.

```bash
# Example config repo — swap in your team's conf
celer init --url=https://github.com/celer-pkg/test-conf.git

# Pick a platform and project (one platform.toml = toolchain + sysroot)
celer configure --platform=x86_64-linux-ubuntu-22.04-gcc-11.5.0
celer configure --project=project_test_01

# Install a dependency, switch to aarch64 / Windows / QNX by changing --platform
celer install glog@0.6.0
```

Then in your own project:

```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=<toolchain_file.cmake from Celer>
cmake --build build
```

📖 [Full quick start](./docs/en-US/quick_start.md) · [Why Celer?](./docs/en-US/why_celer.md)

## VS Code extension

Recommended: [celer-vscode](https://github.com/celer-pkg/celer-vscode), then you can use celer graphically.

## 💡 Non-intrusive: the project does not change

![workflow](./docs/assets/workflow.svg)

Dependencies, platforms, and compiler flags live in Celer's conf — not in your application repo.

- Keep your existing CMake layout; no recipe rewrite for the package manager
- Switch targets by swapping a `platform.toml`, not rewriting CLI flags and profiles
- The generated toolchain file travels on its own — CI, handoff, and bug reproduction use the same file

## 🎯 Who this is for

- You ship one product across **Windows / Linux / embedded** with **MSVC, GCC, and Clang**
- You need a **reproducible build** for teammates or CI, not "works on my machine"
- You distribute **private binaries** or run an internal artifact store
- Several projects share a machine and you are done with dependency versions colliding

If you only want a few open-source libraries on a laptop, [Conan](https://conan.io) / [vcpkg](https://vcpkg.io) / [XMake](https://xmake.io) are ready-made. Celer is for teams that put **cross-compilation, isolation, and delivery** first.

## 🌟 What you get

| When you need… | What Celer does |
| --- | --- |
| **Switch cross-compile targets** | One `platform.toml` holds toolchain, sysroot, and env vars — ARM / x86 / QNX / Windows / Linux |
| **Keep projects from colliding** | Isolate versions, macros, and CMake vars by platform × project × build type |
| **Mixed upstream build systems** | CMake, Makefiles, Meson, B2, QMake, Bazel, GYP |
| **Private / prebuilt libraries** | Hash-based artifact cache shared by the team and CI |
| **Flaky or restricted GitHub access** | Git repos of third-party libraries and tools you already fetched are cached |
| **Reproducible CI** | Export a workspace snapshot; the same config enters the pipeline |

MSVC / Clang / GCC are ready on Windows and Linux; macOS is still in progress. Additional targets (including QNX) are defined as platform configs.

## 📚 Documentation

**Get started in 5 minutes:**
- [Quick Start Guide](./docs/en-US/quick_start.md) · [Init a Workspace](./docs/en-US/quick_start.md#3-setup-conf)
- [Create a Platform / Project / Port](./docs/en-US/cmd_create.md)
- [Install a Library](./docs/en-US/cmd_install.md) · [Deploy](./docs/en-US/cmd_deploy.md)

**Deep dives:**
- [Generate CMake Config Files](./docs/en-US/article_generate_cmake_config.md)
- [Platform Config Deep Dive](./docs/en-US/article_platform.md) · [Port Config Deep Dive](./docs/en-US/article_port.md) · [Project Config Deep Dive](./docs/en-US/article_project.md)
- [PkgCache: Shared Cache (fs / MinIO)](./docs/en-US/article_pkgcache.md) · [Artifact Cache](./docs/en-US/article_pkgcache_artifacts.md) · [Repo Cache](./docs/en-US/article_pkgcache_repos.md) · [Download Cache](./docs/en-US/article_pkgcache_downloads.md)
- [CCache Integration](./docs/en-US/article_ccache.md)
- [CUDA Auto-detection](./docs/en-US/article_cuda_support.md)
- [Expression Variables](./docs/en-US/article_expvars.md)
- [Dependency Conflict Detection](./docs/en-US/article_detect_conflict_circular.md)
- [Python Version Management](./docs/en-US/article_python_management.md)
- [Build Tools](./docs/en-US/article_build_tools.md)
- [Export Snapshots](./docs/en-US/cmd_deploy_snapshot.md)

**Reference:**
- [configure](./docs/en-US/cmd_configure.md) · [install](./docs/en-US/cmd_install.md) · [remove](./docs/en-US/cmd_remove.md) · [update](./docs/en-US/cmd_update.md) · [search](./docs/en-US/cmd_search.md) · [tree](./docs/en-US/cmd_tree.md) · [clean](./docs/en-US/cmd_clean.md) · [autoremove](./docs/en-US/cmd_autoremove.md) · [reverse](./docs/en-US/cmd_reverse.md) · [integrate](./docs/en-US/cmd_integrate.md) · [version](./docs/en-US/cmd_version.md)

## 🤝 Contributing

Contributions are welcome in the core, ports, and extension:

- **[celer](https://github.com/celer-pkg/celer)** — core package manager
- **[ports](https://github.com/celer-pkg/ports)** — port definitions & build configs
- **[celer-vscode](https://github.com/celer-pkg/celer-vscode)** — VS Code extension

## 📄 License

MIT. See [LICENSE](./LICENSE). Third-party libraries in `ports/` remain under their original licenses.

---

<div align="center">

**A C/C++ package manager: don't change the project. Make cross-compilation a toolchain file you can ship.**

[⭐ Star us on GitHub](https://github.com/celer-pkg/celer) | [📖 Documentation](./docs/en-US/quick_start.md) | [🐛 Report Issues](https://github.com/celer-pkg/celer/issues)

</div>
