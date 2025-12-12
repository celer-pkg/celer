# Dependency Conflict and Circular Dependency Detection

> **Automatically detect version conflicts and circular dependencies in your project**

## 🎯 Overview

Celer provides powerful dependency detection mechanisms that automatically discover and report two critical types of issues before project builds:

- 🔴 **Version Conflict Detection** - Multiple versions of the same library coexisting in the project
- 🔄 **Circular Dependency Detection** - Circular reference relationships between libraries

These detection mechanisms ensure clear and consistent dependency relationships in your project, avoiding hard-to-debug errors during builds.

---

## 📦 Version Conflict Detection

### What is a Version Conflict?

A version conflict occurs when different dependencies in a project reference different versions of the same library. This situation can lead to:
- Linking errors
- Runtime crashes
- Undefined behavior

### When Conflict Detection Occurs

Celer automatically performs version conflict detection during the following operations:
- Executing `celer install` to install packages
- Executing `celer tree` to view dependency trees
- Executing `celer reverse` to view reverse dependencies
- Executing `celer deploy` for project deployment

### Conflict Report Example

When a version conflict is detected, Celer outputs detailed conflict information:

```
[✘] failed to check circular dependency and version conflict.
[☛] conflicting versions of ports detected:
--> ffmpeg@5.1.6 is defined in opencv@4.11.0, ffmpeg@3.4.13 is defined in project_xxx.
```

**Report Interpretation:**
The `ffmpeg` library has two versions (5.1.6 and 3.4.13) referenced simultaneously in the project - one is depended on by opencv@4.11.0, and the other is depended on by the root project.

---

## 🔄 Circular Dependency Detection

### What is a Circular Dependency?

A circular dependency refers to a closed loop in the dependency relationships between libraries:
- A depends on B
- B depends on C
- C depends back on A

This circular reference can lead to:
- Inability to determine the correct build order
- Potential infinite recursion
- Build system entering a deadlock

### Detection Scope

Celer detects two types of circular dependencies:

1. **Runtime Dependency Loop** - Circular relationships among regular dependencies
2. **Dev Dependency Loop** - Circular relationships among dev_dependencies

### Circular Dependency Report Example

```
Error: circular dev_dependency detected: m4@1.4.19 -> automake@1.18 [dev] -> autoconf@2.72 [dev] -> m4@1.4.19 [dev] -> automake@1.18 [dev]
```

**Report Interpretation:**
- `m4@1.4.19` depends on `automake@1.18` (dev dependency)
- `automake@1.18` depends on `autoconf@2.72` (dev dependency)
- `autoconf@2.72` depends back on `m4@1.4.19` (dev dependency)
- Forms a closed loop: m4 → automake → autoconf → m4 → automake

The `[dev]` marker indicates this is compiled by the local toolchain and serves as a tool library.

---

## 🔍 Detection Principles

### Conflict Detection Algorithm

1. **Collect Dependency Information** - Traverse all project dependencies and record each library's version
2. **Build Version Mapping** - Create a `library name → [version list]` mapping
3. **Detect Conflicts** - Find libraries with version lists longer than 1
4. **Generate Report** - Output detailed conflict information including sources and types

### Circular Detection Algorithm

Celer uses a **Depth-First Search (DFS) + Path Recording** algorithm:

1. **Initialization** - Create visit markers and path stack
2. **DFS Traversal** - Depth-first traversal starting from root dependencies
3. **Path Detection** - If current node is already in path stack, a circular dependency is detected
4. **Separate Detection** - Dev dependencies and runtime dependencies are detected separately
5. **Report Path** - Output complete circular dependency path

**Key Features:**
- ✅ Distinguishes between dev dependencies and runtime dependencies
- ✅ Supports dev dependency markers
- ✅ Provides complete circular path information
- ✅ Efficient caching mechanism to avoid duplicate detection
