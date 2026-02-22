# Integrate 命令

`integrate` 命令用于为 `celer` 注册或移除 shell 自动补全集成。

## 命令语法

```shell
celer integrate [flags]
```

## 重要行为

- 会自动识别当前 shell 环境。
- 支持 `bash`、`zsh`、`PowerShell`。
- `--remove` 会将行为从“注册补全”切换为“移除补全”。
- 不支持的 shell 环境会直接报错。

## 命令选项

| 选项     | 类型 | 说明           |
|----------|------|----------------|
| --remove | 布尔 | 移除 shell 补全 |

## 常用示例

```shell
# 为当前 shell 注册补全
celer integrate

# 为当前 shell 移除补全
celer integrate --remove
```

## 说明

- 如果你同时使用多个 shell，需要分别执行一次。
- 某些环境下可能需要重开终端后补全才生效。
