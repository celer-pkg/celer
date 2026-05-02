# Caching Build Artifacts

> **Save hours of build time with intelligent artifact caching**

## Why Cache?

Compiling C/C++ libraries from source is time-consuming and can severely impact development efficiency. A typical project with 20+ dependencies may take **2-3 hours** for the first build. Celer's intelligent caching system can reduce later builds to **just a few minutes**.

**Key benefits:**
- **Significantly faster builds** - Reuse compiled artifacts instead of rebuilding from source
- **Team collaboration** - Share build artifacts across a team through a network folder
- **Private libraries** - Distribute prebuilt binaries without exposing source code
- **Precise invalidation** - Cache entries are invalidated automatically whenever dependencies or configuration change

## How It Works

Celer uses **hash-based caching** to store and retrieve build artifacts:

1. **Compute a hash**: Generate a unique hash from the build environment, options, dependencies, and patches
2. **Check the cache**: Search the configured cache directory for a matching artifact
3. **Use or build**: If found, extract and use it; if not, build from source
4. **Store the result**: Automatically save the new build artifact for future reuse when conditions allow

## Quick Start

### Step 1: Configure the Cache Location

Add a `[pkgcache]` section to `celer.toml` to enable cache lookup:

```toml
[global]
	conf_repo = "https://github.com/celer-pkg/test-conf.git"
	platform = "x86_64-linux-ubuntu-22.04-gcc-11.5.0.5"
	project = "project_01"
	jobs = 32

[pkgcache]
	dir = "/home/test/pkgcache"  # Local or network-mounted directory
	writable = false             # Read-only cache by default; artifacts are written only when true
```

**What happens now:**
- Celer searches for cached artifacts before building
- If a match is found, it is extracted and used immediately
- If not found, the library is built from source as usual

> **Tip**: Use a network-mounted folder such as NFS, SMB, or FTP to share cache across a team

### Step 2: Store Build Artifacts Automatically

```toml
[package]
	url = "https://gitlab.com/libeigen/eigen.git"
	ref = "3.4.0"

[[build_configs]]
	build_system = "cmake"
	options = ["-DEIGEN_TEST_NO_OPENGL=1", "-DBUILD_TESTING=OFF"]
```

**Usage:**

```bash
celer install eigen@3.4.0
```

**What happens:**
1. The library is built from source
2. The build output is packaged into a `.tar.gz` artifact
3. A hash-based filename is generated from the full build configuration
4. The artifact is stored in the cache directory
5. Metadata (`.meta` files) is created in the package and install directories to track which files were installed

**Common cases where writing to cache is skipped automatically:**
- `pkgcache` is not configured
- `pkgcache.dir` is not configured
- `pkgcache.writable=false` makes the cache read-only
- The source repository has local manual modifications before the build starts

**How Celer looks up a matching stored artifact:**
- Check whether `pkgcache` and `pkgcache.dir` are configured; if not, stop looking
- Check whether the current repository has local modifications; if it does, stop looking
- Read the current git commit hash and generate a cache hash from the current build configuration
- Search under `pkgcache/artifacts` using that hash and the current build configuration
- If a matching artifact archive is found, reuse it by simulating the normal post-build install flow

## Private Library Distribution

### Distribute Prebuilt Binaries Without Exposing Source Code

For private or proprietary libraries, you can distribute prebuilt artifacts without exposing source code. By specifying a git commit hash in `checksum`, Celer can retrieve the build artifact directly from cache:

```toml
[package]
	url = "https://gitlab.com/libeigen/eigen.git"
	ref = "3.4.0"
	checksum = "3147391d946bb4b6c68edd901f2add6ac1f31f8c" # Setting checksum enables artifact cache support

[[build_configs]]
	build_system = "cmake"
	options = ["-DEIGEN_TEST_NO_OPENGL=1", "-DBUILD_TESTING=OFF"]
```

**How it works:**
1. Celer calculates the cache key from the `checksum` in `port.toml` plus the full build configuration
2. It searches for a matching artifact in `pkgcache.artifacts`
3. If found, it extracts and uses the artifact **without cloning the repository**
4. If not found, it falls back to building from source if the source is accessible

**Use cases:**
- Distribute internal libraries to partners without exposing source code
- Speed up CI/CD pipelines by skipping git clone
- Control access to proprietary code while still sharing binaries

## Cache Directory Structure

Celer organizes cached artifacts in a hierarchical layout that is easy to manage:

```
/home/test/pkgcache/
    └── artifacts
        └── x86_64-linux-ubuntu-22.04-gcc-11.5.0/     # Platform
            └── project_01/                           # Project
                └── release/                          # Build type (release/debug)
                    ├── ffmpeg@3.4.13/                # Library name@version
                    │   ├── d536728...09068.tar.gz    # Build artifact (compressed)
                    │   ├── f466728...a0906.tar.gz    # Different configuration variant
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
- **Project level**: Isolates different projects to avoid conflicts
- **Build type**: Separates `debug`, `release`, and other build variants
- **Library folders**: One folder per library, including its version
- **Artifacts**: Hash-named `.tar.gz` files containing built outputs
- **Metadata**: `.meta` files storing the hash key and build configuration

## How Cache Keys Work

Celer generates a **unique hash** for every build configuration. This hash is the cache key, so artifacts are reused only when the build is truly identical.

### Cache Key Components

The hash is calculated from multiple factors:

#### 1. Build Environment
- **Toolchain**: URL, path, name, version, system architecture
- **Sysroot**: URL, path, `pkg_config_path`
- **Compiler**: GCC or Clang version, target triple

#### 2. Build Parameters
- **Library options**: For example, FFmpeg options such as `--enable-cross-compile`, `--enable-shared`, and `--with-x264`
- **Environment variables**: `CFLAGS`, `LDFLAGS`, `CXXFLAGS`
- **Build type**: Debug vs Release, static vs shared

#### 3. Source Modifications
- **Applied patches**: Patch file contents are included in the hash
- **Source checksum**: A git commit hash or archive checksum

#### 4. Dependency Graph
- **Recursive dependency hashes**: Hashes of all dependencies, such as x264 and nasm
- **Dependency configuration**: Their build options, versions, and patches

### Automatic Cache Invalidation

**Any change to these factors produces a new hash:**

```
Old config: FFmpeg + x264 1.0 + --enable-shared  -> Hash: abc123...
New config: FFmpeg + x264 2.0 + --enable-shared  -> Hash: def456...  (x264 version changed)
```

When the hash changes:
1. The old cache entry is not reused because it belongs to a different build
2. The library is rebuilt from source
3. The new artifact is stored under the new hash

> **Result**: You never reuse stale or incompatible cached artifacts.
