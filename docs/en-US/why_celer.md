# Why Choose Celer?

> *Celer is a C/C++ package manager built for engineering delivery — an Accelerator for C/C++ projects, focused on the most expensive, painful, and repeatedly error-prone parts of dependency management.*

## 🎯 Celer's Core Design

### 1. TOML Declarative Configuration — Host a Library in Seconds

`port.toml` + `project.toml` + `platform.toml` are all TOML declarative — no scripting. Changing a port's build options = editing one line of TOML, no programming required.

The biggest benefit of declarative configuration is **low barrier**: no need to learn Python or CMake script syntax. Any developer who can read a config file can host a new library. This drastically shortens the new-library integration cycle — from "find an expert to write a recipe" to "edit one line of TOML."

### 2. Delivery-Oriented — One-Command Build, Install, and Reproducible Delivery

`celer deploy` builds and installs all dependencies for an entire project in one command, producing a deliverable installation directory. Built-in snapshot export (`--snapshot`) records the exact commit + build environment for every port, enabling reproducible delivery.

This means build results go beyond "compilation passed" — they directly produce deliverable, traceable artifacts. The same snapshot can reproduce an identical build environment on any machine.

### 3. Project-Platform-BuildType Three-Dimensional Isolation

`installed/<platform>@<project>@<buildType>/` directory structure provides natural isolation. The same machine can hold `x86_64-linux@project_xx@release` and `aarch64-linux@project_xx@debug` simultaneously without interference. Switch with `celer configure --project=project_xx --build-type=debug`.

No naming conventions or profile files needed for isolation — the directory structure itself is the isolation boundary, visible at a glance, impossible to confuse.

### 4. Project-Level Port Override + Vendor Directory

Projects can place their own ports in `conf/projects/<proj>/ports/` to override versions from the global ports repository. Third-party and project-owned ports are physically separated — clear at a glance. Three lookup locations are supported (project top-level → project vendor → global ports), with conflict reporting for same-named ports.

This makes "customizing a third-party library's build options for a specific project" trivial — drop a port.toml in the project vendor directory without affecting the global repository or polluting other projects.

### 5. Native Cross-Compilation Support

Platform configuration (`conf/platforms/*.toml`) defines toolchain, sysroot, and rootfs. `celer configure --platform=aarch64-linux-xxx` sets everything up. Built-in toolchain file generation and cross-compiler auto-detection.

All cross-compilation complexity (toolchain paths, sysroot, ABI compatibility, environment variables) is consolidated into one TOML file — no need to assemble toolchains, profiles, or triplets manually.

### 6. Precise Cache Management — Meta-Driven Cache Key

One of the biggest pain points in C/C++ projects is slow compilation. Maximizing cache utilization is a core design goal of Celer.

Every library compiled through Celer records its build environment, commit hash, dependency information, and dependency commit hashes. These are combined into a single hash used to find matching cache. When a transitive dependency's hash changes, dependent libraries are automatically recompiled.

Celer computes **meta** — a metadata string containing **every factor that influences the compiled result** — and uses `sha256(meta)` as the cache key. Meta is collected automatically and recursively:

| Factor | Description |
| --- | --- |
| **Full port.toml content** | url, ref, checksum, patches, build_options, envs, dependencies... |
| **Transitive deps' port.toml** | Recursively expanded — any dependency change at any depth affects the hash |
| **Exact commit hash** | Resolved from git, not the ref name (same branch, different commit = different artifact) |
| **Platform toolchain config** | compiler, sysroot, flags... |
| **Build type** | release / debug / relwithdebinfo... |
| **Build tool versions** | cmake version, ninja version... |

**Any one of these changes → meta changes → hash changes → cache automatically misses** — zero false-hit risk, with no manual declaration required.

On cache restore, Celer also verifies that the `.meta` file content has not been tampered with (`sha256(meta file content) == buildhash`), ensuring the cache cannot be silently corrupted.

### 7. Non-Intrusive — No Need to Adapt Business Code to the Package Manager

Business code does not need to modify CMakeLists.txt to accommodate Celer. Celer injects paths through toolchain file + environment variables, and `find_package` naturally finds dependencies. Business CMake is written just like any normal CMake project.

This means existing projects can adopt Celer without restructuring their CMake — set `CMAKE_TOOLCHAIN_FILE` to point at Celer's generated file, and existing build flows work immediately.

### 8. Multi-Level Cache System — From Build Tools to Build Artifacts

To make Celer usable as a daily development tool for real projects, Celer provides a multi-level cache system:

| Cache Level | Description |
| --- | --- |
| **Build tool cache** | cmake, msys2, ninja, git and other build tools are auto-downloaded and cached — no manual setup needed |
| **Source repository cache** | Clone results from GitHub and other external repos are cached on the LAN — unstable external networks don't block builds |
| **Artifact cache (pkgcache)** | Meta-hash-based build artifact cache with NFS sharing support for team-wide reuse |
| **Developer local cache (devcache)** | devDep/hostDep build artifacts cached in `~/.celer`, reused across workspaces without polluting the shared cache |

By configuring pkgcache for LAN sharing, after one team member's initial build, subsequent members pull caches directly from the LAN — eliminating repeated compilation and external network downloads.

---

## 🚀 Who Celer Is For

Celer is designed for teams that need:
- Frequent integration of both third-party libraries and internally developed shared libraries
- Enterprise-grade dependency management across multiple platforms and sub-projects
- Fast, reproducible builds
- A shift from experience-driven dependency handling to an engineering workflow

[Get Started →](./quick_start.md) | [Back to README →](../../README.md)