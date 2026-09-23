# Caching Python Wheels

> **Avoid repeated PyPI downloads for `build_tools` Python libraries**

Ports declare Python libraries as build tools, e.g. `build_tools = ["python3:mako", "python3:MarkupSafe@2.1.0"]`. Each is installed into the build venv via `pip`. The **Python Wheel Cache** stores the resolved wheel set (with transitive deps) into `pkgcache/python-wheels`, so later installs restore the wheels and run a fully offline `pip install --no-index --find-links` — no PyPI traffic on a hit.

## How It Works

Each spec is installed through three layers:

1. **L1** — persistent wheelhouse (`downloads/wheelhouse-<pyminor>/`): try `pip install --no-index --find-links` for all specs. If pip resolves everything, done.
2. **L2** — pkgcache restore: for unsatisfied specs, `Restore(cacheKey, wheelhouse)` copies cached wheels into the wheelhouse.
3. **L3** — online `pip download <spec>` into a tmp dir, merge into the wheelhouse, store into pkgcache.

Final install is always `pip install --no-index --find-links=<wheelhouse> <spec>` — offline regardless of which layer produced the wheels.

> Only `build_tools` Python libraries go through this path. Conda-forge env creation and the Python interpreter are not covered.

## Directory Layout

```text
python-wheels/<platform-scope>/<Name>/<Version>/<wheel-files>
```

- `<platform-scope>` = `py<minor>-<GOOS>-<GOARCH>`, e.g. `py310-linux-amd64`. Isolates by Python minor version and OS/arch — no cross-platform contamination.
- `<Name>` / `<Version>` come from the spec.

```text
pkgcache/python-wheels/py310-linux-amd64/
    ├── MarkupSafe/
    │   ├── 2.1.0/MarkupSafe-2.1.0-cp310-cp310-manylinux_2_17_x86_64.whl
    │   └── 2.1.5/MarkupSafe-2.1.5-cp310-cp310-manylinux_2_17_x86_64.whl
    └── Mako/1.2.0/
        ├── Mako-1.2.0-py3-none-any.whl
        └── ...                            # transitive deps
```

Multiple versions coexist as sibling directories. Each wheel is verified by SHA-256: fs stores a `<wheel>.sha256` sidecar; minio stores it in object metadata (`x-amz-meta-sha256`).

## Quick Start

```toml
[pkgcache.fs]
	dir = "/home/test/pkgcache"

[pkgcache.options]
	writable = true              # required to store wheels
	python_wheels = true         # toggle the wheel cache (default true)
```

```toml
# ports/m/mesa/24.0.0/port.toml
[package]
    build_tools = ["python3:mako", "python3:packaging", "python3:MarkupSafe@2.1.0"]
```

```bash
celer install mesa@24.0.0
```

- **First build**: `pip download` hits PyPI, wheels stored under `python-wheels/...`.
- **Delete venv and reinstall**: no PyPI request — wheels restored and installed offline.
- **Fresh workspace sharing the same `pkgcache`**: same, proving cross-machine reuse.

## Behavior

- **Store** when: pkgcache configured, `writable = true`, `python_wheels = true`, online, and the spec went through L3.
- **Restore**: fs works offline (local dir); minio returns a miss offline (cannot reach the bucket).
- **Already-installed packages skip the cache** (matched by exact name and version).
- **Toggle**: `pkgcache.options.python_wheels` (default `true` on first backend config). Disable with `celer configure --pkgcache-cache-python-wheels=false`.
