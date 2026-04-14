# Why Choose Celer?

> *Celer is a C/C++ package manager built for enterprise scenarios, focused on the most expensive, painful, and repeatedly error-prone parts of dependency management.*

You may already be using **Conan**, **Vcpkg**, or **XMake**. They are powerful tools, but in enterprise projects, teams are usually slowed down not by "can we fetch the package," but by these recurring issues:

## 🎯 The Pain Points Celer Targets

### 1. New Library Integration Is Slow Because the Process Is Heavy

**What hurts:**

- Existing libraries are easy to consume, but introducing one without a ready-made recipe can suddenly stretch lead time
- Developers are forced to learn tool-specific scripts, patch build details, and handle install paths
- Every new library integration feels like rebuilding the process from scratch

**Real cost:**

- Feature delivery gets blocked and schedules become less predictable
- Integration quality depends heavily on a few experts, making team scaling harder
- Configuration quality drifts, and long-term maintenance cost keeps rising

**How Celer addresses it:**

Declare build system type (CMake/Make/Meson, etc.) and required options in TOML, and Celer standardizes the rest of the integration flow.  
You focus on what the library needs, not on taming toolchain internals.

### 2. Cross-Project Contamination Causes Rework

**What hurts:**

- Different projects need different, sometimes mutually exclusive options for the same library
- In global/shared-directory setups, a change for one project can break another
- Boundaries get blurry when mixing private and public libraries

**Real cost:**

- More intermittent failures like "it built yesterday, now it doesn't"
- Teams spend more time diffing environments than building product value
- Upgrades become high-risk operations people avoid

**How Celer addresses it:**

Isolate dependency versions, build options, and private library definitions per project.  
Each project has its own reproducible dependency configuration, with no cross-project contamination.

### 3. Dependency Drift Across Sub-Projects Gets Harder to Control as the Platform Grows

**What hurts:**

- Each sub-project manages dependencies independently, so configuration is scattered
- Over time, version drift appears: one platform in name, different stacks in practice
- Unified upgrades require repo-by-repo alignment and expensive manual checking

**Real cost:**

- The same feature passes in sub-project A but fails in sub-project B, creating "reproduces only in this repo" churn during integration
- Every dependency upgrade requires sub-project-by-sub-project regression, scaling test/build time linearly
- Releases are often squeezed by last-minute patches or rollbacks when one sub-project lags on dependency versions

**How Celer addresses it:**

Use one TOML file for centralized dependency definitions and auto-generate a unified `toolchain_file.cmake` inherited by all sub-projects.  
Update once, sync globally, and reduce manual alignment work.

### 4. Uncontrolled Build Caching Wastes Time on Rebuilds

**What hurts:**

- Multi-platform dependencies are often maintained via manual prebuilds and shared folders
- Small config changes can trigger full rebuilds, and it's hard to tell what is reusable
- Storing artifacts in Git or archives bloats size, slows transfer, and stays coarse-grained

**Real cost:**

- CI/CD and local build times keep climbing
- Storage and bandwidth costs increase passively
- Teams lose predictability around "how long will this change take to build"

**How Celer addresses it:**

Generate hash keys from environment, compiler options, and dependency chains to reuse artifacts precisely.  
Reuse when safe, invalidate when needed, and cut repeated builds plus manual cache cleanup.

### 5. Conflicts Are Found Too Late and Explode During Integration

**What hurts:**

- Deep dependency conflicts (especially diamond dependencies) are hard to detect early
- Problems often surface only during integration or runtime, with long debug paths
- Manual dependency-tree inspection is slow and error-prone

**Real cost:**

- Rollbacks and hotfixes become routine
- Rework at critical milestones delays release dates
- Teams become conservative about dependency upgrades

**How Celer addresses it:**

Run dependency version consistency checks at build time and report actionable conflict details.  
Move "pre-release failures" forward to "visible during build."

### 6. Cross-Company Collaboration Has a High Environment Handoff Cost

**What hurts:**

- External collaboration often requires shipping a full build environment
- Partner machines are inconsistent, so onboarding takes longer
- "Works on your side, fails on mine" loops keep recurring

**Real cost:**

- Longer integration cycles and higher communication overhead
- Core engineers get pulled into remote firefighting
- Delivery quality is affected by environment inconsistency

**How Celer addresses it:**

Ship dependency context through a portable `toolchain_file.cmake` (relative paths, self-contained layout).  
Partners only set `CMAKE_TOOLCHAIN_FILE` to get a consistent build context quickly.

---

## 🚀 Who Celer Is For

Celer is designed for teams that need:
- Frequent integration of both third-party libraries and internally developed shared libraries
- Enterprise-grade dependency management
- Fast, reproducible builds
- A shift from experience-driven dependency handling to an engineering workflow

[Get Started →](./quick_start.md) | [Back to README →](../../README.md)
