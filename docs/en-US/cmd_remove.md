# How to remove installed libraries

## Overview

The celer remove command uninstalls a specified package from your system. It provides flexible removal options including dependency cleanup, build cache deletion, and development-mode package handling.

## Command Syntax

```shell
celer remove [package_name] [flags]  
```

## Flags

| Flag | Shorthand | Description |
| ---- | --------- | ----------- |
| --build-type | -b | Uninstall a package with the specified build type (default: "release"). |
| --dev | -d | Uninstall a development-mode package (used for dev dependencies). |
| --purge | -f | Aggressive removal: Delete the package along with all its associated files (e.g., configs, data). |
| --recurse | -r | Recursive removal: Uninstall the package and its dependencies (if no other packages require them). |
| --remove-cache | -c | Clean build cache: Remove cached build artifacts for the package. |

## Usage Examples

1. Basic removal

```shell
celer remove ffmpeg@5.1.6
```

2. Remove with dependencies

```shell
celer remove ffmpeg@5.1.6 --recurse/-r
```

3. Purge package completely

```shell
celer remove ffmpeg@5.1.6 --purge/-p
```

4. Remove Dev-Mode package

```shell
celer remove ffmpeg@5.1.6 --dev/-d
```

5. Remove package and clean build cache

```shell
celer remove ffmpeg@5.1.6 --remove-cache/-c
```

>Notes
>1. Use -r with caution—it may break other packages if shared dependencies are removed.
>2. -p is irreversible; ensure you don’t need the package’s files (e.g., configs).
>3. Cache removal (-c) frees disk space but may slow down future rebuilds.
