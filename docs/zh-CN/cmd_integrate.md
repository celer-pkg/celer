# Integrate 命令

`integrate` 命令用于为 `celer` 注册或移除 shell 自动补全集成。

## 命令语法

```shell
celer integrate [flags]
```

## 重要行为

- 在 Linux 上会自动识别当前 shell（`bash` 或 `zsh`）。
- 在 Windows 上固定配置 `PowerShell` 补全。
- `--remove` 会将行为从“注册补全”切换为“移除补全”。
- 不支持的 shell 环境会直接报错。

## 命令选项

| 选项     | 类型 | 说明           |
|----------|------|----------------|
| --remove | 布尔 | 移除 shell 补全 |

## 常用示例

```shell
# 注册补全（Linux: 当前 shell，Windows: PowerShell）
celer integrate

# 移除补全（Linux: 当前 shell，Windows: PowerShell）
celer integrate --remove
```

## 说明

- 在 Linux 上如果你同时使用多个 shell，需要分别执行一次。
- 某些环境下可能需要重开终端后补全才生效。
