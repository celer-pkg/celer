# Integrate 命令

&emsp;&emsp;Celer 集成命令为 Celer 提供智能 Tab 补全功能，显著提升命令行效率。

## 支持的 Shell

- Bash (Linux)
- PowerShell (Windows)

## 命令语法

```shell
celer integrate [--bash|--powershell]
```

## 命令选项

| Option	    | Description	                                                            |
| ------------- | ------------------------------------------------------------------------- |
| --bash	    | Enable tab completion for Bash shell	                                    |
| --powershell	| Enable tab completion for PowerShell	                                    |
| --unregister  | Combing with --bash and --powershell to unregister tab completions	    |

## 用法示例

**1. 特定 Shell 集成**

要为特定 Shell 启用 Tab 补全，请使用对应的选项：

```shell
celer integrate --bash
celer integrate --powershell
```

**2. 移除 Tab 补全**

要移除所有 Celer shell 补全，请使用 --unregister 选项：

```shell
celer integrate --bash --unregister
celer integrate --powershell --unregister
```

> **Note:**   
> 集成完成后，您可能需要重启 shell 才能使更改生效。
