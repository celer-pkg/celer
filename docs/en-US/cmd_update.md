# 🔄 Update Command

&emsp;&emsp;The `update` command is used to synchronize local repositories with their remote counterparts, ensuring you have the latest package configurations and build definitions. It supports targeted updates for different repository types.

## Command Syntax

```shell
celer update [options] [package_name]
```

## ⚙️ Command Options

| Option        | Short | Description                                        |
|---------------|-------|----------------------------------------------------|
| --conf-repo   | -c    | Update only the workspace conf repository          |
| --ports-repo  | -p    | Update only the ports repository                   |
| --force       | -f    | Combine with --conf-repo or --ports-repo to force update |
| --recursive   | -r    | Recursively update all dependencies of a package   |

## 💡 Usage Examples

### 1️⃣ Update conf Repository

```shell
celer update --conf-repo
# Or use shorthand
celer update -c
```

> Update workspace configuration repository, including platform configs, project configs, etc.

### 2️⃣ Update ports Repository

```shell
celer update --ports-repo
# Or use shorthand
celer update -p
```

> Update third-party library configuration hosting repository to get the latest port configuration files.

### 3️⃣ Update Specific Port Source Repository

```shell
celer update ffmpeg@3.4.13
```

> Update FFmpeg's source code repository to pull the latest source changes.

### 4️⃣ Force Update

```shell
celer update --conf-repo --force
# Or use shorthand
celer update -c -f
```

> Force update the conf repository, overwriting local modifications.

### 5️⃣ Recursive Update (Including Dependencies)

```shell
celer update --recursive ffmpeg@3.4.13
# Or use shorthand
celer update -r ffmpeg@3.4.13
```

> Update FFmpeg and all its dependencies' source repositories.

### 6️⃣ Combined Options

```shell
celer update --force --recursive ffmpeg@3.4.13
# Or use shorthand
celer update -f -r ffmpeg@3.4.13
```

> Force update FFmpeg and all its dependencies' source repositories.

---

## 📁 Repository Types

### conf Repository
- **Location**: `conf/` directory
- **Content**: Platform configs (`platforms/`), project configs (`projects/`), build tools configs (`buildtools/`)
- **Update Frequency**: When new platform or project configurations are needed

### ports Repository
- **Location**: `ports/` directory
- **Content**: Port configuration files for third-party libraries (`port.toml`)
- **Update Frequency**: Regular updates to get new third-party library support

### Source Repository
- **Location**: `buildtrees/<library_name>@<version>/src/`
- **Content**: Source code of third-party libraries
- **Update Frequency**: When latest source changes are needed

---

## ⚠️ Notes

1. **Network Connection**: Update operations require network access to remote repositories
2. **Force Update**: Using `--force` will overwrite local modifications, use with caution
3. **Recursive Update**: `--recursive` updates all dependencies, which may take a long time
4. **Git Repositories**: conf and ports repositories are typically Git repositories, ensure proper permissions
5. **Backup Modifications**: If you have custom configurations, recommend backing up before updating

---

## 📚 Related Documentation

- [Quick Start](./quick_start.md)
- [Install Command](./cmd_install.md) - Install third-party libraries
- [Port Configuration](./article_port.md) - Learn about port configuration files
- [Platform Configuration](./article_platform.md) - Learn about platform configuration files

---

**Need Help?** [Report an Issue](https://github.com/celer-pkg/celer/issues) or check our [Documentation](../../README.md)
