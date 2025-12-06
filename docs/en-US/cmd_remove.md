# 🗑️ Remove Command

&emsp;&emsp;The `remove` command is used to uninstall specified third-party libraries from the system. It provides flexible removal options, including dependency cleanup, build cache deletion, and dev-mode library handling.

## Command Syntax

```shell
celer remove [name@version] [options]
```

## ⚙️ Command Options

| Option            | Short | Description                                     |
|-------------------|-------|-------------------------------------------------|
| --dev             | -d    | Remove library installed in dev mode            |
| --purge           | -p    | Remove library along with its package files     |
| --recursive       | -r    | Remove library along with its dependencies      |
| --build-cache     | -c    | Remove library along with build cache           |

## 💡 Usage Examples

### 1️⃣ Basic Removal

```shell
celer remove ffmpeg@5.1.6
```

> Only remove the specified version of FFmpeg, keeping its dependencies and build cache.

### 2️⃣ Recursive Removal (Including Dependencies)

```shell
celer remove ffmpeg@5.1.6 --recursive
# Or use shorthand
celer remove ffmpeg@5.1.6 -r
```

> Remove the specified library and all its dependencies. **Note**: Dependencies used by other libraries will not be removed.

### 3️⃣ Purge Removal (Including Package)

```shell
celer remove ffmpeg@5.1.6 --purge
# Or use shorthand
celer remove ffmpeg@5.1.6 -p
```

> Remove the installed library and delete corresponding package files in the `packages/` directory.

### 4️⃣ Remove Dev Mode Library

```shell
celer remove nasm@2.16.03 --dev
# Or use shorthand
celer remove nasm@2.16.03 -d
```

> Remove build tools or dependencies installed in dev mode (`--dev`).

### 5️⃣ Remove and Clean Build Cache

```shell
celer remove ffmpeg@5.1.6 --build-cache
# Or use shorthand
celer remove ffmpeg@5.1.6 -c
```

> Remove the library and delete build cache in the `buildtrees/` directory.

### 6️⃣ Combined Options

```shell
celer remove ffmpeg@5.1.6 --recursive --purge --build-cache
# Or use shorthand
celer remove ffmpeg@5.1.6 -r -p -c
```

> Complete removal: delete library, dependencies, package files, and build cache.

---

## 📁 Removal Operations

### Basic Removal
- Delete library files in `installed/<platform>/` directory
- Delete `.trace` files in `installed/celer/info/` directory
- Delete `.hash` files in `installed/celer/hash/` directory

### Recursive Removal (--recursive)
- Based on basic removal, recursively delete all dependencies
- Only delete dependencies not used by other libraries

### Purge Removal (--purge)
- Based on basic removal, delete package files in `packages/` directory
- Package files are typically used for binary cache distribution

### Clean Build Cache (--build-cache)
- Based on basic removal, delete build cache in `buildtrees/` directory
- Includes source code, build intermediate files, etc.

---

## ⚠️ Notes

1. **Dependency Check**: When using `--recursive`, the system checks dependencies and won't delete libraries depended on by other libraries
2. **Irreversible**: Removal operations are irreversible, deleted files cannot be recovered
3. **Version Specification**: Must specify complete library name and version number, e.g., `ffmpeg@5.1.6`
4. **Dev Mode Libraries**: Libraries installed in dev mode (such as build tools) are stored in `installed/<platform>-dev/` directory
5. **Disk Space**: Using `--build-cache` and `--purge` can free up significant disk space

---

## 📚 Related Documentation

- [Quick Start](./quick_start.md)
- [Install Command](./cmd_install.md) - Install third-party libraries
- [Clean Command](./cmd_clean.md) - Clean unused resources
- [Autoremove Command](./cmd_autoremove.md) - Auto-remove orphaned dependencies

---

**Need Help?** [Report an Issue](https://github.com/celer-pkg/celer/issues) or check our [Documentation](../../README.md)