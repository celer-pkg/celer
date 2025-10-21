# Remove command

&emsp;&emsp;The remove command enable uninstall packages from installed directory. It provides flexible removal options including dependency cleanup, build cache deletion and dev-mode package handling.

## Command Syntax

```shell
celer remove [name@version] [flags]  
```

## Flags

| Flag              | Shorthand | Description                                               |
| ----------------- | --------- | --------------------------------------------------------- |
| --dev             | -d        | uninstall package for dev mode.                           |
| --purge           | -f        | uninstall package along with its package files.           |
| --recurse         | -r        | uninstall package along with its depedencies.             |
| --build-cache     | -c        | uninstall package along with build cache.                 |

## Usage Examples

### 1. Basic removal

```shell
celer remove ffmpeg@5.1.6
```

### 2. Remove package along with dependencies

```shell
celer remove ffmpeg@5.1.6 --recurse/-r
```

### 3. Remove package along with its package

```shell
celer remove ffmpeg@5.1.6 --purge/-p
```

### 4. Remove dev runtime mode package

```shell
celer remove ffmpeg@5.1.6 --dev/-d
```

### 5. Remove package and remove build cache

```shell
celer remove ffmpeg@5.1.6 --build-cache/-c
```