# 🌳 树形命令（Tree）

&emsp;&emsp;`tree` 命令用于可视化显示软件包或项目的依赖关系树，默认显示运行时依赖项和开发依赖项。

## 命令语法

```shell
celer tree [package_name|project_name] [选项]
```

## ⚙️ 命令选项

| 选项         | 说明                      |
|--------------|--------------------------|
| --hide-dev   | 隐藏开发依赖项             |

## 💡 使用示例

### 1️⃣ 显示完整的依赖树

```shell
celer tree ffmpeg@5.1.6
```

> 显示 FFmpeg 的所有运行时依赖项和开发依赖项。

### 2️⃣ 隐藏开发依赖项

```shell
celer tree ffmpeg@5.1.6 --hide-dev
```

> 仅显示运行时依赖项，隐藏构建时所需的开发工具。

### 3️⃣ 显示项目依赖树

```shell
celer tree project_xxx
```

> 在项目目录下执行，显示当前项目的所有依赖项。

---

## 📊 示例输出

```
display dependencies in tree view:
--------------------------------------------
libffi@3.4.8
├── macros@1.20.2 -- [dev]
│   └── automake@1.18 -- [dev]
│       └── autoconf@2.72 -- [dev]
│           └── m4@1.4.19 -- [dev]
└── libtool@2.5.4 -- [dev]
    └── m4@1.4.19 -- [dev]
---------------------------------------------
summary: dependencies: 0  dev_dependencies: 5
```

### 输出说明

- **常规项**：运行时依赖项，是库运行时必需的依赖
- **[dev] 前缀**：开发依赖项，仅在构建时需要，不会包含在最终部署中

---

## 🎯 使用场景

### 场景 1：分析依赖关系
在安装新库之前，查看其依赖项，了解会安装哪些额外的库。

```shell
celer tree opencv@4.11.0
```

### 场景 2：排查依赖问题
当遇到依赖冲突或编译错误时，查看完整的依赖树来定位问题。

```shell
celer tree ffmpeg@5.1.6
```

### 场景 3：检查项目配置
验证项目中配置的依赖项是否正确。

```shell
celer tree
```
---

## 📝 注意事项

1. **循环依赖**：如果存在循环依赖，命令会提示错误
2. **大型项目**：对于具有大量依赖项的项目，输出可能很长
3. **版本信息**：树形结构显示的版本号是实际安装的版本
4. **开发依赖**：使用 `--hide-dev` 可以简化输出，仅关注运行时依赖

---

## 📚 相关文档

- [快速开始](./quick_start.md)
- [Install 命令](./cmd_install.md) - 安装第三方库
- [Reverse 命令](./cmd_reverse.md) - 分析反向依赖关系
- [项目配置](./article_project.md) - 配置项目依赖

---

**需要帮助？** [报告问题](https://github.com/celer-pkg/celer/issues) 或查看我们的 [文档](../../README.md)