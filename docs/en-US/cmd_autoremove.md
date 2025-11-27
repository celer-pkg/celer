
# 🧹 autoremove Command

> Automatically clean up unused libraries in your project and keep your development environment tidy and efficient.

## ✨ Overview

The `autoremove` command removes installed dependencies that are no longer required by the current project. It helps you:

- Clean up unused dependencies and free up disk space
- Ensure your project only contains necessary libraries
- Optionally delete package files and build cache

## 📝 Command Syntax

```shell
celer autoremove [flags]
```

## ⚙️ Command Options

| Option         | Short | Description                                 |
| -------------- | ----- | ------------------------------------------- |
| --purge        | -p    | Remove libraries and their package files     |
| --remove-cache | -c    | Remove libraries and their build cache       |

## 💡 Usage Examples

**1. Remove unused libraries**

```shell
celer autoremove
```

**2. Remove unused libraries and their package files**

```shell
celer autoremove --purge
# or
celer autoremove -p
```

**3. Remove unused libraries, package files, and build cache**

```shell
celer autoremove --purge --remove-cache
# or
celer autoremove -p -c
```

## 📖 Use Cases

- Quickly clean up unused libraries after dependency changes
- Keep CI/CD environments clean
- Save disk space and avoid redundant files

---

For more help, see the [Command Reference](./cmds.md) or [Report Issues](https://github.com/celer-pkg/celer/issues).