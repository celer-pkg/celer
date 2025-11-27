
# 🧹 clean Command

> One-click clean build cache for projects or packages, free up space, and fix build issues.

## ✨ Overview

The `clean` command removes build cache files for specified packages or projects. Common use cases:
- Free up disk space
- Resolve build issues caused by stale cache
- Keep your development environment tidy

## 📝 Command Syntax

```shell
celer clean [flags] [package/project...]
```

## ⚙️ Command Options

| Option         | Short | Description                                 |
| -------------- | ----- | ------------------------------------------- |
| --all          | -a    | Clean build cache for all packages/projects  |
| --dev          | -d    | Clean cache for dev mode packages/projects   |
| --recurse      | -r    | Recursively clean cache for package/project and its dependencies |

## 💡 Usage Examples

**1. Clean build cache for all dependencies of a project**
```shell
celer clean project_xxx
```

**2. Clean build cache for multiple packages**
```shell
celer clean ffmpeg@5.1.6 opencv@4.11.0
```

**3. Clean cache for dev mode packages**
```shell
celer clean --dev pkgconf@2.4.3
# or
celer clean -d pkgconf@2.4.3
```

**4. Recursively clean cache for a package and its dependencies**
```shell
celer clean --recurse ffmpeg@5.1.6
# or
celer clean -r ffmpeg@5.1.6
```

**5. Clean all build cache**
```shell
celer clean --all
# or
celer clean -a
```

## 📖 Use Cases

- Clean old cache after upgrading packages or projects
- Keep CI/CD environments consistent and clean
- Quickly free up disk space during local development

---

> **Note:**
> 1. For git clone libraries, this command will clean the source directory.
> 2. For URL download libraries, this command will re-extract the archive to replace the source directory.