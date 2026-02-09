# Caching Build Artifacts

> **Save hours of build time with intelligent artifact caching**

## 🎯 Why Cache?

Compiling C/C++ libraries from source is time-consuming and can severely impact development efficiency. A typical project with 20+ dependencies might take **2-3 hours** for the first build. Celer's intelligent caching system can reduce this to **minutes** on subsequent builds.

**Key Benefits:**
- ⚡ **Dramatically faster builds** - Reuse compiled artifacts instead of rebuilding
- 🤝 **Team collaboration** - Share build artifacts across your team via network folders
- 🔒 **Private libraries** - Distribute pre-built binaries without exposing source code
- 🎯 **Precision invalidation** - Automatic cache invalidation when any dependency or configuration changes

## 💡 How It Works

Celer uses **hash-based caching** to store and retrieve build artifacts:

1. **Calculate Hash**: Generate a unique hash from build environment, options, dependencies, and patches
2. **Check Cache**: Search for matching artifacts in the configured cache directory
3. **Use or Build**: If found, extract and use; if not, build from source
4. **Store Result**: Optionally save the new build artifact for future use

## 🚀 Quick Start

### Step 1: Configure Cache Location

Add the `[package_cache]` section to your `celer.toml` to enable cache retrieval:

```toml
[global]
conf_repo = "https://github.com/celer-pkg/test-conf.git"
platform = "x86_64-linux-ubuntu-22.04-gcc-11.5.0.5"
project = "project_01"
jobs = 32

[package_cache]
dir = "/home/test/celer_cache"  # Local or network-mounted directory
```

**What happens now:**
- ✅ Celer searches for cached artifacts before building
- ✅ If a match is found, it's extracted and used instantly
- ✅ If not found, the library builds from source (as usual)

> 💡 **Tip**: Use a network-mounted folder (e.g., NFS, SMB) to share cache across your team

### Step 2: Store Build Artifacts (Optional)

To save build artifacts to the cache, add `token` and use the `--store-cache` flag:

```toml
[global]
conf_repo = "https://github.com/celer-pkg/test-conf.git"
platform = "x86_64-linux-ubuntu-22.04-gcc-11.5.0.5"
project = "project_01"
jobs = 32

[package_cache]
dir = "/home/test/celer_cache"
```

**Usage:**
```bash
# Build and store artifacts to cache
celer install opencv --store-cache --cache-token=xxxyyyzzz
```

**What happens:**
1. Library builds from source
2. Build artifact is packaged into a `.tar.gz` file
3. Hash-based filename is generated
4. Artifact is stored in the cache directory
5. Metadata (`.meta` file) is created in package and install directories for tracking

## 🔒 Private Library Distribution

### Distribute Pre-built Binaries Without Source Code

For private or proprietary libraries, you can distribute pre-built artifacts without exposing source code. By specifying the `commit` hash, Celer can retrieve the build artifact directly from cache:

```toml
[package]
url = "https://gitlab.com/libeigen/eigen.git"
ref = "3.4.0"
commit = "3147391d946bb4b6c68edd901f2add6ac1f31f8c"  # Enables cache-only mode

[[build_configs]]
build_system = "cmake"
options = ["-DEIGEN_TEST_NO_OPENGL=1", "-DBUILD_TESTING=OFF"]
```

**How it works:**
1. Celer calculates the cache key using the `commit` hash and build config
2. Searches for matching artifact in `package_cache`
3. If found, extracts and uses it **without cloning the repository**
4. If not found, falls back to building from source (if accessible)

**Use cases:**
- 📦 Distribute internal libraries to partners without source code
- 🚀 Speed up CI/CD pipelines by skipping git clones
- 🔐 Control access to proprietary code while sharing binaries

## 📁 Cache Directory Structure

Celer organizes cached artifacts in a hierarchical structure for easy management:

```
/home/test/celer_cache/
└── x86_64-linux-ubuntu-22.04-gcc-11.5.0/     # Platform
    └── project_01/                           # Project
        └── release/                          # Build type (release/debug)
            ├── ffmpeg@3.4.13/                # Library name@version
            │   ├── d536728...09068.tar.gz    # Build artifact (compressed)
            │   ├── f466728...a0906.tar.gz    # Different config variant
            │   └── meta/                     # Metadata directory
            │       ├── d536728...09068.meta  # Hash key + build info
            │       └── f466728...a0906.meta
            │
            ├── opencv@4.5.1/
            │   ├── li98343...39a8.tar.gz
            │   ├── 4324324...sfdf.tar.gz
            │   └── meta/
            │       ├── li98343...39a8.meta
            │       └── 4324324...sfdf.meta
            └── ...
```

**Directory breakdown:**
- **Platform level**: Separates artifacts by target platform and toolchain
- **Project level**: Isolates different projects to prevent conflicts
- **Build type**: Separates debug and release builds
- **Library folders**: One folder per library with version
- **Artifacts**: Hash-named `.tar.gz` files containing built libraries
- **Metadata**: `.meta` files store hash keys and build configurations

## 🔑 How Cache Keys Work

Celer generates a **unique hash** for each build configuration. This hash acts as the cache key, ensuring that artifacts are only reused when the build is truly identical.

### Cache Key Components

The hash is calculated from multiple factors:

#### 1. 🛠️ Build Environment
- **Toolchain**: URL, path, name, version, system architecture
- **Sysroot**: URL, path, `pkg_config_path`
- **Compiler**: GCC/Clang version, target triple

#### 2. ⚙️ Build Parameters
- **Library options**: e.g., FFmpeg's `--enable-cross-compile`, `--enable-shared`, `--with-x264`
- **Environment variables**: `CFLAGS`, `LDFLAGS`, `CXXFLAGS`
- **Build type**: Debug vs Release, static vs shared

#### 3. 📝 Source Modifications
- **Applied patches**: Patch file contents are hashed
- **Source commit**: Git commit hash (if specified)

#### 4. 🔗 Dependency Graph
- **Recursive dependency hashes**: Hashes of all dependencies (x264, nasm, etc.)
- **Dependency configurations**: Their build options, versions, and patches

### 🔄 Automatic Cache Invalidation

**Any change to these factors triggers a new hash:**

```
Old config:  FFmpeg + x264 1.0 + --enable-shared  → Hash: abc123...
New config:  FFmpeg + x264 2.0 + --enable-shared  → Hash: def456...  (x264 version changed)
```

When the hash changes:
1. ❌ Old cache is not used (it's for a different configuration)
2. 🔨 Library rebuilds from source
3. 💾 New artifact is stored with the new hash

> ✅ **Result**: You never use stale or incompatible cached artifacts!

---

## 💼 Real-World Scenarios

### Scenario 1: Team Collaboration

**Setup:**
```toml
[package_cache]
dir = "/mnt/shared/celer_cache"  # Network drive
token = "team_build_token"
```

**Workflow:**
1. Developer A builds OpenCV with CUDA support → stores to cache
2. Developer B needs the same OpenCV → retrieves from cache in seconds
3. CI/CD pipeline uses the same cache → consistent builds

**Time saved**: First build: tens of minutes → Subsequent builds: tens of seconds

### Scenario 2: Cross-Platform Development

**Multiple platforms in one cache:**
```
celer_cache/
├── x86_64-linux-ubuntu-22.04-gcc-11.5.0/
├── aarch64-linux-gnu-gcc-9.2/
└── x86_64-windows-msvc-14.44/
```

Each platform maintains separate cached artifacts, preventing conflicts.

### Scenario 3: Private SDK Distribution

Distribute SDK with pre-built dependencies:
1. Build all dependencies with `--store-cache` and `--cache-token=xxxyyyzzz`
2. Package the cache folder with your SDK
3. Partners configure `package_cache.dir` to the packaged cache
4. They get instant builds without compiling dependencies

---

## 🎯 Best Practices

### ✅ Do's

- **Use network storage** for team caches (NFS, SMB, cloud storage)
- **Set appropriate tokens** to control who can write to cache
- **Monitor cache size** and clean old artifacts periodically
- **Include cache in CI/CD** to speed up pipelines
- **Document cache location** for team members

### ❌ Don'ts

- **Don't put cache on slow storage** (will negate speed benefits)
- **Don't share tokens publicly** (controls write access)
- **Don't manually edit** cached artifacts or metadata
- **Don't delete cache** without checking with your team

---

## 🔧 Troubleshooting

### Cache not being used?

**Check these:**
1. ✓ Is `package_cache.dir` configured in `celer.toml`?
2. ✓ Does the directory exist and have read permissions?
3. ✓ Are you using the exact same platform and project settings?
4. ✓ Did any build options or dependencies change?

### Can't store to cache?

**Possible causes:**
1. ✗ The specified cache write token is correct
2. ✗ No write permissions to cache directory
3. ✗ Disk space full
4. ✗ Forgot `--store-cache` flag

### Cache taking too much space?

**Solutions:**
- Clean old artifacts for outdated library versions
- Use separate caches for different projects
- Implement retention policies (keep last N versions)

---

## 📚 Related Documentation

- [Quick Start Guide](./quick_start.md) - Get started with Celer
- [Why Celer?](./why_celer.md) - Learn about hash-based caching benefits
- [Project Configuration](./cmd_create.md#2-create-a-new-project) - Configure projects

---

**Need help?** [Report an issue](https://github.com/celer-pkg/celer/issues) or check our [documentation](../../README.md)