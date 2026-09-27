# Caching Python Wheels

> **Avoid repeated PyPI downloads for `build_tools` Python libraries**

Ports declare Python libraries as build tools, e.g. `build_tools = ["python3:mako", "python3:MarkupSafe@2.1.0"]`. Each is installed into the build venv via `pip`. The **Python Wheel Cache** stores the resolved wheel set (with transitive deps) into `pkgcache/python-wheels`, so later installs restore the wheels and run a fully offline `pip install --no-index --find-links` — no PyPI traffic on a hit.

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

## Quick Start

```toml
[pkgcache]
	writable = true              # required to store wheels

[pkgcache.fs]
	dir = "/home/test/pkgcache"
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