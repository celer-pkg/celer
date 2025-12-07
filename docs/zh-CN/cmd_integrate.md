# ⚡ 集成命令（Integrate）

&emsp;&emsp;`integrate` 命令为 Celer 提供智能 Tab 补全功能，显著提升命令行效率和使用体验。

## 命令语法

```shell
celer integrate [选项]
```

## 🖥️ 支持的 Shell

Celer 支持以下主流 Shell 环境的自动补全：

| Shell       | 操作系统       | 说明                     |
|-------------|---------------|--------------------------|
| Bash        | Linux/macOS   | 最常用的 Linux Shell      |
| Zsh         | Linux/macOS   | 强大的交互式 Shell        |
| PowerShell  | Windows       | Windows 原生 Shell       |

## ⚙️ 命令选项

| 选项       | 说明                     |
|------------|-------------------------|
| --remove   | 移除 Tab 补全功能        |

## 💡 使用示例

### 1️⃣ 启用 Tab 补全

```shell
celer integrate
```

> Celer 能自动识别当前 Shell 类型，并为其启用 Tab 补全功能。

### 2️⃣ 移除 Tab 补全

```shell
celer integrate --remove
```

> 移除 Tab 补全时同样能自动识别当前 Shell 类型。

---

## 🎯 功能特性

### 命令补全
输入 `celer` 后按 Tab 键，自动补全可用命令：
```shell
celer <Tab>
# 显示: install, configure, deploy, create, remove, clean, etc.
```

### 选项补全
输入命令后按 Tab 键，自动补全可用选项：
```shell
celer install --<Tab>
# 显示: --dev, --force, --jobs, --recursive, --store-cache, --cache-token
```

### 包名补全
输入包名时按 Tab 键，自动补全已知的端口：
```shell
celer install ffm<Tab>
# 自动补全为: celer install ffmpeg@
```

---

## ⚠️ 注意事项

1. **权限要求**：在某些系统上可能需要管理员权限
2. **重启终端**：配置生效可能需要重启终端或重新加载配置
3. **Shell 版本**：确保使用的 Shell 版本支持补全功能
4. **多个 Shell**：如果使用多个 Shell，需要分别在每个 Shell 中运行集成命令

---

## 📚 相关文档

- [快速开始](./quick_start.md)
- [命令总览](./commands.md)

---

**需要帮助？** [报告问题](https://github.com/celer-pkg/celer/issues) 或查看我们的 [文档](../../README.md)
