# Integrate 命令

&emsp;&emsp;Celer 集成命令为 Celer 提供智能 Tab 补全功能，显著提升命令行效率。

## 支持的 Shell

- Bash (Linux)
- Zsh (Linux)
- PowerShell (Windows)

## 命令语法

```shell
celer integrate [--remove]
```

## 命令选项

| Option	    | Description	                |
| ------------- | ------------------------------|
| --remove      | remove tab completions	    |

## 用法示例

**1. 特定 Shell 集成**

```shell
celer integrate
```

>Celer能自动识别当前是那种shell终端，并为其支持Tab补全的功能。

**2. 移除 Tab 补全**

```shell
celer integrate --remove
```

> 在移除Tab补全时候同样能自动识别当前是那种shell终端。
