# 🗑️ 移除命令（Remove）

&emsp;&emsp;`remove` 命令用于从系统中卸载指定的第三方库。它提供了灵活的移除选项，包括依赖项清理、构建缓存删除和开发模式库处理。

## 命令语法

```shell
celer remove [name@version] [选项]
```

## ⚙️ 命令选项

| 选项              | 简写 | 说明                                 |
|-------------------|------|------------------------------------|
| --dev             | -d   | 移除开发模式安装的库                   |
| --purge           | -p   | 移除库的同时删除其 package 文件        |
| --recursive       | -r   | 移除库的同时删除其依赖项               |
| --build-cache     | -c   | 移除库的同时删除构建缓存               |

## 💡 使用示例

### 1️⃣ 基本移除

```shell
celer remove ffmpeg@5.1.6
```

> 仅移除指定版本的 FFmpeg，保留其依赖项和构建缓存。

### 2️⃣ 递归移除（包含依赖项）

```shell
celer remove ffmpeg@5.1.6 --recursive
# 或使用简写
celer remove ffmpeg@5.1.6 -r
```

> 移除指定的库及其所有依赖项。**注意**：如果依赖项被其他库使用，则不会被移除。

### 3️⃣ 彻底清除（包含 package）

```shell
celer remove ffmpeg@5.1.6 --purge
# 或使用简写
celer remove ffmpeg@5.1.6 -p
```

> 移除安装的库，同时删除 `packages/` 目录下对应的打包文件。

### 4️⃣ 移除开发模式库

```shell
celer remove nasm@2.16.03 --dev
# 或使用简写
celer remove nasm@2.16.03 -d
```

> 移除以开发模式（`--dev`）安装的构建工具或依赖项。

### 5️⃣ 移除并清理构建缓存

```shell
celer remove ffmpeg@5.1.6 --build-cache
# 或使用简写
celer remove ffmpeg@5.1.6 -c
```

> 移除库的同时删除 `buildtrees/` 目录下的构建缓存。

### 6️⃣ 组合选项

```shell
celer remove ffmpeg@5.1.6 --recursive --purge --build-cache
# 或使用简写
celer remove ffmpeg@5.1.6 -r -p -c
```

> 彻底移除：删除库、依赖项、package 文件和构建缓存。

---

## 📁 移除操作说明

### 基本移除
- 删除 `installed/<platform>/` 目录下的库文件
- 删除 `installed/celer/info/` 目录下的 `.trace` 文件
- 删除 `installed/celer/hash/` 目录下的 `.hash` 文件

### 递归移除（--recursive）
- 在基本移除的基础上，递归删除所有依赖项
- 仅删除不被其他库依赖的依赖项

### 彻底清除（--purge）
- 在基本移除的基础上，删除 `packages/` 目录下的打包文件
- 打包文件通常用于二进制缓存分发

### 清理构建缓存（--build-cache）
- 在基本移除的基础上，删除 `buildtrees/` 目录下的构建缓存
- 包括源码、构建中间文件等

---

## ⚠️ 注意事项

1. **依赖关系检查**：使用 `--recursive` 时，系统会检查依赖关系，不会删除被其他库依赖的库
2. **无法撤销**：移除操作不可撤销，删除的文件无法恢复
3. **版本指定**：必须指定完整的库名和版本号，例如 `ffmpeg@5.1.6`
4. **开发模式库**：开发模式安装的库（如构建工具）存储在 `installed/<platform>-dev/` 目录
5. **磁盘空间**：使用 `--build-cache` 和 `--purge` 可以释放大量磁盘空间

---

## 📚 相关文档

- [快速开始](./quick_start.md)
- [Install 命令](./cmd_install.md) - 安装第三方库
- [Clean 命令](./cmd_clean.md) - 清理未使用的资源
- [Autoremove 命令](./cmd_autoremove.md) - 自动移除孤立依赖

---

**需要帮助？** [报告问题](https://github.com/celer-pkg/celer/issues) 或查看我们的 [文档](../../README.md)