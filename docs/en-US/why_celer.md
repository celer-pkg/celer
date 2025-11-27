# Why Choose Celer?

> *Solving the hard problems in C/C++ dependency management*

You may already be familiar with popular C/C++ package managers like **Conan**, **Vcpkg**, and **XMake**. These tools have made significant progress in package management and continue to grow rapidly. However, several critical challenges remain unsolved in real-world enterprise development:

## 🎯 The Problems Celer Solves

### 1. 📦 Simplified Library Integration

**The Problem:**  
While using existing libraries is straightforward, adding new ones is complex. Developers must:
- Learn proprietary scripting languages or APIs
- Manually handle build workflows (configure/build/install)
- Write complex recipe files for each library

**Celer's Solution:**  
Just declare the build system type (CMake, Make, Meson, etc.) in a simple TOML config. Celer handles the rest automatically. Focus on compile options and dependencies—not build system internals.

### 2. 🏢 Project-Level Dependency Isolation

**The Problem:**  
Third-party libraries offer numerous optional features, and different projects often need:
- Different configuration combinations (sometimes mutually exclusive)
- Custom builds to avoid dependency bloat
- Integration of private/internal libraries alongside public ones

The traditional approach might be to pre-compile required libraries for different projects into specific isolated directories, then independently compile and update them when library changes are needed - this is a typical way of managing build artifacts.

**Celer's Solution:**  
Project-level dependency management with isolated configurations. Each project maintains its own:
- Library versions and build options
- Private library definitions in project-specific directories
- No global conflicts, no cross-contamination

### 3. 🔗 Platform Multi-Project Management

**The Problem:**  
Platform projects with multiple sub-projects face:
- Scattered dependency lists across sub-projects
- Version drift and inconsistencies over time
- High manual verification costs
- Difficult synchronization when dependencies update

**Celer's Solution:**  
Centralized dependency control through a single project TOML file:

1. **Define once**: Declare all dependencies in one place
2. **Auto-generate**: Celer creates a global `toolchain_file.cmake`
3. **Inherit everywhere**: Sub-projects automatically get consistent dependencies
4. **Update seamlessly**: Modify the root config, all sub-projects sync automatically

This maintains sub-project independence while ensuring dependency consistency across your entire platform.

### 4. ⚡ Intelligent Hash-Based Caching

**The Problem:**  
C/C++ builds are notoriously slow, especially initial builds with many dependencies. Traditional precompiled solutions:
- Difficult to maintain across multiple platforms
- Prone to errors with manual lib/include management
- No automatic invalidation when dependencies change
- Risk of using stale or incompatible binaries

**Celer's Solution:**  
Precision hash-based artifact caching:

- **Smart Keys**: Hash generated from environment, compiler options, dependency chain, and more
- **Auto-Invalidation**: Any change triggers a new hash—no stale builds
- **Shared Caching**: Team-wide artifact sharing via network folders
- **Zero Redundancy**: Identical configurations never rebuild

Result: Dramatically reduced build times while eliminating manual cache management risks.

### 5. 🔍 Automatic Conflict Detection

**The Problem:**  
Deep dependency trees create version conflicts:
- Hard to discover until runtime failures
- Painful manual debugging and resolution
- Diamond dependency problems

**Celer's Solution:**  
Automatic conflict detection at build time:

- **Manifest-based**: All dependencies tracked explicitly
- **Version consistency checks**: Automatic validation across the dependency tree
- **Clear reporting**: Conflicts reported immediately with actionable details
- **Build-time safety**: Catch issues before they reach production

### 6. 🤝 Seamless Cross-Company/Team Collaboration

**The Problem:**  
When collaborating with external partners:
- Lead companies must distribute complete build environments
- Partners shouldn't need manual installation or configuration
- Toolchain setup should be portable and self-contained
- Build environment consistency is critical

**Celer's Solution:**  
Portable, self-contained `toolchain_file.cmake`:

- **Relative Paths**: All tools and dependencies located via relative paths
- **Single File Integration**: Partners just set `CMAKE_TOOLCHAIN_FILE`
- **No Installation Required**: Complete environment travels with the toolchain file
- **Instant Productivity**: Clone, point to toolchain file, build—done

---

## 📊 Quick Comparison

| Feature | Traditional Tools | Celer |
|---------|------------------|-------|
| **Adding Libraries** | Write complex recipes | Declare build system type |
| **Project Isolation** | Global configs (conflicts) | Project-level isolation |
| **Multi-Project Sync** | Manual per-project setup | Single TOML, auto-sync |
| **Build Caching** | git or archive storage | Hash-based precision |
| **Conflict Detection** | Runtime discovery | Build-time checks |
| **Collaboration** | Manual env setup | Portable toolchain file |

---

## 💼 Real-World Use Cases

### Embedded Systems Team
*“We support unlimited chip platforms. Celer's platform configs let us switch targets instantly without polluting each other's dependencies.”*

### Enterprise Platform Team
*“Our platform engineering has multiple sub-projects. One project TOML ensures all sub-projects use the same library versions—no more version drift.”*

### Open Source Contributors
*“Adding a new library to Celer took 1 minute. With other tools, learning their recipe language took hours.”*

### Cross-Company Partnership
*“We ship our SDK with a toolchain file. Partners are productive in minutes, not days setting up environments.”*

---

## 🚀 Ready to Try?

Celer is designed for teams that need:
- ✅ Simplified cross-compilation configuration
- ✅ Enterprise-grade dependency management
- ✅ Fast, reproducible builds
- ✅ Simple library integration

[Get Started →](./quick_start.md) | [Back to README →](../../README.md)