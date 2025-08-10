# 集成tab 命令补全

## 概览

Celer 集成tab 命令补全功能，将显著提高命令行生产力。

## 支持的shell

- Bash (Linux/macOS)
- PowerShell (Windows)
- Zsh (macOS/Linux)

## Basic Usage

```shell
celer integrate [--bash|--powershell|--zsh]
```

## 命令选项

| 选项	| 描述	|
| -------- | -------- |
| --bash	| 为Bash shell 启用tab 命令补全	|
| --powershell	| 为PowerShell 启用tab 命令补全	|
| --zsh	| 为Zsh shell 启用tab 命令补全	|
| --remove	| 移除所有Celer shell 命令补全	|

## 使用示例

### 特定shell 集成

要为特定shell 启用tab 命令补全，请使用对应的选项：

```shell
celer integrate --bash
celer integrate --powershell
celer integrate --zsh
```

### 移除tab 命令补全

要移除所有Celer shell 命令补全，请使用`--remove` 选项：

```shell
celer integrate --bash --remove
celer integrate --powershell --remove
celer integrate --zsh --remove
```

> **Note:**   
> 集成完成后，您可能需要重启shell 才能使更改生效。
