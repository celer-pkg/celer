
# 🧹 autoremove 命令

> 自动清理项目中未被依赖的库，保持开发环境整洁高效。

## ✨ 功能简介

`autoremove` 命令用于移除当前项目已安装但不再需要的依赖库。它可以帮助你：

- 清理未使用的依赖，释放磁盘空间
- 保证项目环境只包含真正需要的库
- 支持同时删除 package 文件和构建缓存

## 📝 命令语法

```shell
celer autoremove [flags]
```

## ⚙️ 命令选项

| 选项           | 简写  | 说明                                   |
| -------------- | ----- | -------------------------------------- |
| --purge        | -p    | 移除依赖库及其 package 文件             |
| --remove-cache | -c    | 移除依赖库及其构建缓存                  |

## 💡 使用示例

**1. 移除未被依赖的库**

```shell
celer autoremove
```

**2. 移除未被依赖的库及其 package 文件**

```shell
celer autoremove --purge
# 或
celer autoremove -p
```

**3. 移除未被依赖的库、package 文件和构建缓存**

```shell
celer autoremove --purge --remove-cache
# 或
celer autoremove -p -c
```

## 📖 场景说明

- 项目依赖变更后，快速清理无用库
- 保持 CI/CD 环境干净
- 节省磁盘空间，避免冗余文件堆积

---

如需更多帮助，请查阅 [命令参考文档](./cmds.md) 或 [报告问题](https://github.com/celer-pkg/celer/issues)。