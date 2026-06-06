# PkgCache Shared Cache and NFS Permission Management

> **Use NFS shared cache + chattr +a for team-level cache protection**

## Overview

Celer's PkgCache system provides three cache capabilities: **artifact cache**, **repo cache**, and **download cache**. When a team shares one cache directory through NFS, two core problems need to be handled:

1. **Multi-user concurrent writes** — build results from different developers need to be written into the shared directory
2. **Accidental deletion prevention** — no user should delete cache files that other users depend on

Celer uses Linux `chattr +a` (append-only), a system user/group, and `celer setup --nfs-server-dir` to reduce accidental deletion risk while allowing multiple users to write to the shared cache.

## Cache Directory Layout

After `pkgcache.dir` is configured, Celer organizes cache data into functional subdirectories:

```text
/mnt/data/pkgcache/                       # pkgcache.dir
    ├── artifacts-v0.2.7/                  # Artifact cache, isolated by Celer version
    │   └── x86_64-linux-ubuntu-22.04-gcc-11.5.0/
    │       └── project_01/
    │           └── release/
    │               └── ffmpeg@3.4.13/
    │                   ├── d536728...09068.tar.gz
    │                   └── metas/
    │                       └── d536728...09068.meta
    ├── repos/                             # Source repo cache
    │   ├── x264@stable/
    │   │   └── 31e19f92...c3a0d.tar.gz
    │   └── ffmpeg@6.1.1/
    │       └── 1f2e3d4c....tar.gz
    └── downloads/                         # Download file cache
        ├── cmake-3.30.5-linux-x86_64-f747d9b23...e9b51dc9d.tar.gz
        └── gcc-ubuntu-11.5.0-x86_64-aarch64-linux-gnu-a99dee8e3ee2...56ebdad30c.tar.xz
```

For details about each cache type, see:

- [Cache Build Artifacts](article_pkgcache_artifacts.md) — avoid repeated builds
- [Cache Source Repositories](article_pkgcache_repos.md) — avoid repeated clone / source downloads
- [Cache Downloaded Files](article_pkgcache_downloads.md) — reduce dependency on external networks

## Configuration

Add a `[pkgcache]` section to `celer.toml`:

```toml
[main]
  conf_repo = "https://github.com/celer-pkg/test-conf.git"
  platform = "x86_64-linux-ubuntu-22.04-gcc-11.5.0"
  project = "project_01"

[pkgcache]
  dir = "/home/test/pkgcache"   # Local directory or network-mounted directory, such as NFS or SMB
  writable = true               # Whether Celer can write to the cache
  cache_artifacts = true        # Whether artifact cache is enabled
  cache_downloads = true        # Whether download cache is enabled
```

**Configuration fields:**

| Field | Description |
|------|-------------|
| `dir` | Cache root directory. It must already exist. |
| `writable` | `true` allows Celer to write cache entries; `false` makes the cache read-only. |
| `cache_artifacts` | Whether artifact cache is enabled. |
| `cache_downloads` | Whether download cache is enabled. |

You can also configure pkgcache from the command line:

```bash
celer configure --pkgcache-dir=/home/test/pkgcache
celer configure --pkgcache-writable=true
```

### Solution: chattr +a (append-only)

Linux `chattr +a` can make a directory append-only. Celer applies this attribute only to cache directories, not to cache files themselves:

- **Allowed**: create new files, overwrite existing files in place
- **Blocked**: delete files, rename files

This matches the behavior required by cache directories — developers can write new cache entries through Celer and overwrite existing entries in place, but they cannot delete cache entries owned by other users.

### Permission Model

Celer's NFS cache permission model has these layers:

| Layer | Mechanism | Purpose |
|------|-----------|---------|
| Ownership | `chown -R celer:celer` | All files are owned by the `celer` system user. |
| Group permissions | `chmod 2775` for directories, `chmod 664` for files | Members of the `celer` group can read and write. |
| Setgid | The `2` in `chmod 2775` | New files/directories inherit the `celer` group. |
| Append-only protection | `chattr +a` | Prevents deletion of directory entries. |
| Periodic hardening | cron runs `chattr +a` every minute | Ensures newly created directories are also protected. |

### Server Setup

Run this on the NFS server:

```bash
sudo celer setup --nfs-server-dir=/srv/celer-cache
```

> **Note**: this command must be run with `sudo`, and it is supported only on Linux.

The command performs these steps:

1. **Check required tools** — verifies that `nfs-kernel-server` (apt) or `nfs-utils` (yum), and `passwd` / `shadow-utils` are installed
2. **Remove existing append-only attributes** — `find <dir> -type d -exec chattr -a {} ;` because previous `+a` attributes may block `chown` / `chmod`
3. **Create the `celer` system user** — `useradd --system --no-create-home --shell /usr/sbin/nologin celer` (idempotent; skipped if it already exists)
4. **Set file ownership** — `chown -R celer:celer <nfs-dir>`
5. **Set directory permissions** — `find <dir> -type d -exec chmod 2775 {} ;` (group writable + setgid, so new files inherit the `celer` group)
6. **Set file permissions** — `find <dir> -type f -exec chmod 664 {} ;` (group can overwrite files in place)
7. **Add the invoking user to the `celer` group** — `usermod -aG celer $SUDO_USER`
8. **Add the NFS export** — writes to `/etc/exports` with `*(rw,sync,no_subtree_check,no_root_squash)`, then runs `exportfs -ra`
   - `no_root_squash` allows root users on NFS clients to access the shared directory as root when client-side administration is needed; directory protection itself is handled by server-side `chattr +a` and cron
9. **Apply `chattr +a` to all directories** — `find <dir> -type d -exec chattr +a {} ;`
10. **Install the cron job** — writes `/etc/cron.d/celer-chattr`, which applies `chattr +a` to all directories every minute, ensuring that directories created by NFS clients are also protected

### Client Setup

Run this on each NFS client machine:

```bash
sudo celer setup --nfs-client-dir=/home/phil/celer-cache@10.0.8.60:/mnt/data/celer-cache
```

Argument format: `<mount_point>@<server>:<export_path>`

The command performs these steps:

1. **Parse arguments**: split by `@` into the mount point and server export path
2. **Check that the mount point exists**: Celer does not create the mount directory for you
3. **Install NFS client packages**: `nfs-common` (apt) or `nfs-utils` (yum)
4. **Unmount an existing mount**: idempotent operation; errors are ignored
5. **Write fstab**: remove the old entry first, then append: `<server>:<export> <mount> nfs rw,_netdev,noatime,rsize=1048576,wsize=1048576 0 0`
6. **Mount the NFS share**: `mount <mount_point>`
7. **Create the `celer` group**: create the `celer` system user on the client as well
8. **Add the invoking user to the `celer` group**: ensures the user has write permission

### After Setup

Group membership takes effect after logging in again. To apply it immediately, you can run:

```bash
newgrp celer
```