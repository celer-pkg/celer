# Caching Source Repositories

> **Avoid repeated clones and source downloads with Repo Cache**

## Why Repo Cache?

Artifact cache solves the question of whether an already built library can be reused, while **repo cache** solves whether the **source code itself** can be reused.

One of the core reasons repo cache exists is not just performance, but **unreliable source access**. In some countries or enterprise network environments, access to GitHub, GitLab, or other source hosting services may be restricted. You cannot assume that every developer, every CI machine, or every partner environment has stable external network access, and you especially cannot assume everyone has a working proxy configured.

When a project depends on many git repositories or source archives, repeated clone, download, and extraction steps waste time even if the project still needs to be rebuilt. When external access is restricted, those steps may fail entirely. Celer can package source trees into `pkgcache/repos` and restore them later, instead of hitting the remote repository or downloading the archive again.

**Typical scenarios:**
- **Restricted GitHub access** - Not every team member can reliably reach public source hosting services
- **You cannot require everyone to configure a proxy** - A shared cache lowers the environment barrier
- **Avoid repeated clone and download work** - Especially useful for large repositories or slow networks
- **Speed up CI and local reinstalls** - Faster when recreating source directories
- **Handle temporary upstream outages** - Restore from local cache when the remote source is temporarily unavailable
- **Combine with build artifact cache** - Reuse source first, then decide whether build outputs can also be reused

## Repo Cache vs Artifact Cache

| Capability | Repo Cache | Build Artifact Cache |
|------------|------------|----------------------|
| Cached content | Source code | Installed / built artifacts |
| Active stage | `Clone()` stage | `Install()` stage |
| Problem solved | Avoid repeated clone / download | Avoid repeated configure / build / install |
| Storage path | `pkgcache/repos` | `pkgcache/artifacts` |

In simple terms:
- A **repo cache** hit may still lead to a normal build
- An **artifact cache** hit usually means the build step is skipped and Celer goes through the simulated install flow instead

## How It Works

When source code needs to be prepared, Celer follows this flow:

1. Check whether the current source directory already exists and is non-empty
2. If the source directory is already usable, reuse it directly and skip repo cache lookup
3. If the source directory does not exist and global `cache_third_parties` is enabled and the library is defined in `ports/`, try restoring from `pkgcache/repos` first
4. If there is no cache hit, fall back to the normal git clone or archive download/extract flow
5. After the source is ready, if `pkgcache.writable=true` and the current run is not in offline mode, package that source into repo cache

## Quick Start

### Step 1: Configure `pkgcache`

Configure the cache directory in `celer.toml`:

```toml
[main]
	conf_repo = "https://github.com/celer-pkg/test-conf.git"
	platform = "x86_64-linux-ubuntu-22.04-gcc-11.5.0"
	project = "project_01"

[pkgcache]
	dir = "/home/test/pkgcache"
	writable = true
	cache_third_parties = true
```

Notes:
- `dir` must already exist
- Celer writes new source cache entries into `pkgcache/repos` only when `writable=true`
- With `writable=false`, Celer can still try to restore existing cache entries in read-only mode
- `cache_third_parties` (optional, default `false`): When enabled, automatically cache all third-party libraries defined in `ports/`. See [Third-Party Library Caching](#third-party-library-caching) below.

### Third-Party Library Caching (Global Policy)

The `cache_third_parties` setting provides an **automatic, project-level caching policy** for third-party libraries:

```toml
[pkgcache]
	dir = "/home/test/pkgcache"
	writable = true
	cache_third_parties = true
```

## Cache Keys for the Two Source Types

### 1. Git repositories

For git-based sources, the cache key is the **actual checked-out commit hash**.

Example path:

```text
pkgcache/repos/x264@stable/31e19f92f00c7003fa115047ce50978bc98c3a0d.tar.gz
```

This means:
- After the first online clone, Celer packages the source tree for that commit into repo cache
- If a later install resolves to the same commit hash, Celer can restore it directly from cache

> If `ref` is a floating branch or tag instead of a fixed commit, Celer may still write cache after the first clone, but future runs may not reliably hit that cache before touching the remote source. For stable cache hits, it is better to write the fixed commit hash into the `checksum` field.

### 2. Source archives

For archive-based sources, the cache key is the **archive `sha256` checksum**.

Example:

```toml
[package]
	url = "https://example.com/x264-20250101.tar.gz"
	ref = "20250101"
	checksum = "3147391d946bb4b6c68edd901f2add6ac1f31f8c"
	cache_repo = true
```

Example path:

```text
pkgcache/repos/x264@stable/3147391d946bb4b6c68edd901f2add6ac1f31f8c.tar.gz
```

These scenarios are similar to git repositories. The real goal is the same: keep source access stable even when the network is restricted.

## Runtime Behavior Details

### When does Celer try to read repo cache?

Celer tries repo cache before clone/download when all of these are true:

- `pkgcache.dir` is configured
- Global `cache_third_parties=true` is enabled
- The current library is defined in `ports/` directory
- The current source directory does not exist, or it exists but is empty
- The current package is not a virtual port (`url != "_"`)
- There is a usable `ref` or `checksum` to locate the cache entry

### When does Celer write repo cache?

Celer writes the prepared source tree into `pkgcache/repos` when all of these are true:

- `pkgcache.dir` is configured
- Global `cache_third_parties=true` is enabled
- `pkgcache.writable=true`
- The current library is defined in `ports/` directory
- The current run is not in offline mode
- Clone / download / extraction has completed successfully

### When will repo cache not hit?

Common cases include:

- `pkgcache` is not configured or `cache_third_parties=false`
- `pkgcache.dir` does not exist
- The current library is a project-specific override defined in `conf/projects/`
- The source directory already exists and is non-empty, so Celer reuses it directly
- No cache entry exists for the requested commit / checksum
- Offline mode is enabled

## Directory Layout

Repo cache entries are organized by `name@version` under `pkgcache/repos`:

```text
/home/test/pkgcache/
    └── repos
        ├── x264@stable
        │   ├── 31e19f92f00c7003fa115047ce50978bc98c3a0d.tar.gz
        │   └── 3147391d946bb4b6c68edd901f2add6ac1f31f8c.tar.gz
        ├── ffmpeg@6.1.1
        │   └── 1f2e3d4c....tar.gz
        └── opencv@4.10.0
            └── aabbccdd....tar.gz
```

Breakdown:
- The first level is the fixed `repos` folder
- The second level is the library name and version, for example `x264@stable`
- The third level is a `.tar.gz` file named by the cache key

## How It Works with Artifact Cache

In a typical install, Celer may work in this order:

1. Try restoring source code from **repo cache**
2. Try restoring built outputs from **artifact cache**
3. If artifact cache misses, continue with a normal build
4. After a successful build, write back repo cache and artifact cache when conditions allow

These two mechanisms do not conflict. They complement each other:
- Repo cache answers "where does the source come from?"
- Artifact cache answers "can the build result be reused directly?"

## Current Notes

- **Repo cache requires global enablement**: When `cache_third_parties=false`, repo cache will not be used
- **Repo cache does not contain final install outputs**: A repo cache hit does not mean the build can be skipped
- **An existing source directory has higher priority**: If `buildtrees/.../src` already exists and is non-empty, Celer reuses it instead of restoring repo cache
- **Lock source versions for reliable hits**: To get stable repo cache hits across workspaces, provide fixed commits or checksums in your third-party `port.toml`

## Recommended Setup

For projects using both repo cache and build artifact cache, configure in `celer.toml`:

```toml
[pkgcache]
	dir = "/path/to/shared/cache"  # Local or network-mounted directory
	writable = true
	cache_artifacts = true         # Enable build artifact caching
	cache_third_parties = true     # Enable third-party library source caching
	cache_downloads = true         # Enable download file caching
```

**Best practices:**
- In teams with restricted network access, place `pkgcache.dir` on a LAN-shared directory
- Provide accurate `checksum` values (git commit hash or sha256) for third-party libraries in `ports/`
- Ensure `cache_third_parties=true` is enabled in your workspace configuration
- Keep build artifact cache enabled for reusable build outputs

This reduces both:
- Source acquisition time
- Repeated build time
