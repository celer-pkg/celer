# 🔄 更新命令（Update）

&emsp;&emsp;`update` 命令用于同步本地仓库与其远程副本，确保您具有最新的软件包配置和构建定义。它支持针对不同仓库类型的目标更新。

## 命令语法

```shell
celer update [选项] [package_name]
```

## ⚙️ 命令选项

| 选项          | 简写 | 说明                                   |
|---------------|------|----------------------------------------|
| --conf-repo   | -c   | 仅更新工作空间 conf 仓库            |
| --ports-repo  | -p   | 仅更新 ports 仓库                   |
| --force       | -f   | 与 --conf-repo 或 --ports-repo 配合强制更新 |
| --recurse     | -r   | 递归更新软件包的所有依赖项       |

## 💡 使用示例

### 1️⃣ 更新 conf 仓库

```shell
celer update --conf-repo
# 或使用简写
celer update -c
```

> 更新工作空间配置仓库，包括平台配置、项目配置等。

### 2️⃣ 更新 ports 仓库

```shell
celer update --ports-repo
# 或使用简写
celer update -p
```

> 更新第三方库配置托管仓库，获取最新的端口配置文件。

### 3️⃣ 更新特定端口的源码仓库

```shell
celer update ffmpeg@3.4.13
```

> 更新 FFmpeg 的源码仓库，拉取最新的源码变更。

### 4️⃣ 强制更新

```shell
celer update --conf-repo --force
# 或使用简写
celer update -c -f
```

> 强制更新 conf 仓库，即使存在本地修改也会被覆盖。

### 5️⃣ 递归更新（包含依赖）

```shell
celer update --recurse ffmpeg@3.4.13
# 或使用简写
celer update -r ffmpeg@3.4.13
```

> 更新 FFmpeg 及其所有依赖项的源码仓库。

### 6️⃣ 组合选项

```shell
celer update --force --recurse ffmpeg@3.4.13
# 或使用简写
celer update -f -r ffmpeg@3.4.13
```

> 强制更新 FFmpeg 及其所有依赖项的源码仓库。

---

## 📁 仓库类型说明

### conf 仓库
- **位置**：`conf/` 目录
- **内容**：平台配置（`platforms/`）、项目配置（`projects/`）、构建工具配置（`buildtools/`）
- **更新频率**：当需要新的平台或项目配置时

### ports 仓库
- **位置**：`ports/` 目录
- **内容**：第三方库的端口配置文件（`port.toml`）
- **更新频率**：定期更新以获取新的第三方库支持

### 源码仓库
- **位置**：`buildtrees/<库名>@<版本>/src/`
- **内容**：第三方库的源码
- **更新频率**：当需要最新的源码变更时

---

## ⚠️ 注意事项

1. **网络连接**：更新操作需要网络连接以访问远程仓库
2. **强制更新**：使用 `--force` 会覆盖本地修改，请谨慎使用
3. **递归更新**：`--recurse` 会更新所有依赖项，可能耗时较长
4. **Git 仓库**：conf 和 ports 仓库通常是 Git 仓库，确保有相应权限
5. **备份修改**：如果有自定义配置，建议在更新前备份

---

## 📚 相关文档

- [快速开始](./quick_start.md)
- [Install 命令](./cmd_install.md) - 安装第三方库
- [端口配置](./advance_port.md) - 了解端口配置文件
- [平台配置](./advance_platform.md) - 了解平台配置文件

---

**需要帮助？** [报告问题](https://github.com/celer-pkg/celer/issues) 或查看我们的 [文档](../../README.md)
