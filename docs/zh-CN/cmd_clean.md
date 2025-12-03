
# 🧹 clean 命令

> 一键清理项目或包的构建缓存，释放空间，解决编译异常。

## ✨ 功能简介

`clean` 命令用于移除指定包或项目的构建缓存文件，常用于：
- 释放磁盘空间
- 解决因缓存导致的编译问题
- 保持开发环境干净

## 📝 命令语法

```shell
celer clean [flags] [package/project...]
```

## ⚙️ 命令选项

| 选项         | 简写 | 说明                                 |
| ------------ | ---- | ---------------------------------- |
| --all        | -a   | 清理所有包和项目的构建缓存              |
| --dev        | -d   | 清理开发模式下的包/项目缓存             |
| --recurse    | -r   | 递归清理包/项目及其依赖的构建缓存        |

## 💡 使用示例

**1. 清理指定项目的所有依赖项的构建缓存**
```shell
celer clean project_xxx
```

**2. 清理多个包的构建缓存**
```shell
celer clean ffmpeg@5.1.6 opencv@4.11.0
```

**3. 清理开发模式下的包构建缓存**
```shell
celer clean --dev pkgconf@2.4.3
# 或
celer clean -d pkgconf@2.4.3
```

**4. 递归清理包及其依赖项的构建缓存**
```shell
celer clean --recurse ffmpeg@5.1.6
# 或
celer clean -r ffmpeg@5.1.6
```

**5. 清理所有构建缓存**
```shell
celer clean --all
# 或
celer clean -a
```

## 📖 场景说明

- 项目或包升级后，清理旧缓存避免冲突
- CI/CD 环境定期清理，保证构建一致性
- 本地开发磁盘空间不足时快速释放

---

> **注意：**
> 1. 对于 git clone 的库，命令会清除源码目录。
> 2. 对于 URL 下载的库，在下载解压到于源码目录后Celer会自动帮创建本地的git仓库，便于后面快速clean。
