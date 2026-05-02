# Caching Downloaded Build Tools

> **Reduce external network dependency with intelligent download caching**

## Why Cache Downloads?

Downloading build tools and dependency sources can be **slow and unreliable**, especially in regions with poor network connectivity. A typical project with multiple toolchains and large dependencies may spend **hours** on downloads. Celer's intelligent download cache helps you:

- **Speed up builds globally** - Skip redownloading identical files across multiple builds
- **Improve reliability** - Retrieve cached files when remote servers are unavailable or slow
- **Save bandwidth** - Share cached downloads across teams without re-downloading
- **Enable offline builds** - Work efficiently in environments with limited or intermittent connectivity

## How It Works

Celer caches downloaded files using **SHA-256-based verification** to ensure integrity:

1. **Check local cache** - Look for the file in the local downloads directory
2. **Verify remote size** - Compare file size with the remote server to detect incomplete or outdated files
3. **Restore from cache** - If a cached file matches the remote size, restore it from `pkgcache/downloads`
4. **Download if needed** - If the file is missing or outdated, download from the remote server
5. **Verify and cache** - After download, verify SHA-256 hash and store for future use

## Quick Start

### Step 1: Configure the Cache Directory

Add a `[pkgcache]` section to `celer.toml` to enable download caching:

```toml
[global]
	conf_repo = "https://github.com/celer-pkg/test-conf.git"
	platform = "x86_64-linux-ubuntu-22.04-gcc-11.5.0"
	project = "project_01"
	jobs = 32

[pkgcache]
	dir = "/home/test/pkgcache"  # Local or network-mounted directory, can be FTP, SMB, NFS, etc.
	writable = true              # Must be true to write cached downloads
```

**Important**: `writable = true` is required for downloads to be cached automatically.

### Step 2: Add SHA-256 Checksums to Build Tools

For each build tool or dependency source, provide a SHA-256 checksum in your buildtools configuration:

**buildtools/static/x86_64-linux.toml**
```toml
[[build_tools]]
  name = "cmake"
  version = "3.30.5"
  default = true
  url = "https://github.com/Kitware/CMake/releases/download/v3.30.5/cmake-3.30.5-linux-x86_64.tar.gz"
  sha256 = "f747d9b23e1a252a8beafb4ed2bc2ddf78cff7f04a8e4de19f4ff88e9b51dc9d"
  archive = "cmake-3.30.5-linux-x86_64.tar.gz"
  paths = ["cmake-3.30.5-linux-x86_64/bin"]
```

**conf/platforms/x86_64-linux-ubuntu-22.04-gcc-11.5.0.toml**
```toml
[rootfs]
  url = "https://github.com/celer-pkg/test-conf/releases/download/resource/ubuntu-base-22.04.5-base-amd64.tar.xz"
  sha256 = "08442eca9ccf64fd307d8a92582902315a66dc075216812d454596b1208da3bb"
  path = "ubuntu-base-22.04.5-base-amd64"
  pkg_config_path = ["usr/lib/x86_64-linux-gnu/pkgconfig"]
  lib_dirs = ["lib/x86_64-linux-gnu", "usr/lib/x86_64-linux-gnu"]
```

**What SHA-256 does:**
- Provides data integrity verification
- Enables cache lookups by file identity (format: `{filename}-{sha256}.{ext}`)
- Detects if a cached file has been corrupted or modified

## Cache Directory Structure

Celer organizes cached downloads in a simple, flat structure:

```
/home/test/pkgcache/
    └── downloads/
        ├── cmake-3.30.5-linux-x86_64-f747d9b23...e9b51dc9d.tar.gz
        ├── gcc-ubuntu-11.5.0-x86_64-aarch64-linux-gnu-a99dee8e3ee2...56ebdad30c.tar.xz
        ├── ubuntu-base-22.04.5-base-arm64-47e7f499113.....297000486c6e76406232a.tar.xz
        └── ...
```

**Cached filename format**: `{basename}-{sha256}.{ext}`

## Verification How It Works

### Three-Layer Verification Mechanism

Celer uses three layers of verification to ensure data integrity:

1. Local cache lookup
2. Cache hit and size comparison with remote (not checked in offline mode)
3. Verify SHA-256 after download
