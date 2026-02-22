# Update 命令

`update` 命令用于更新 conf/ports 仓库，或更新指定端口的源码仓库。

## 命令语法

```shell
celer update [flags] [name@version...]
```

## 重要行为

- `--conf-repo` 和 `--ports-repo` 互斥。
- 如果不指定这两个仓库 flag，则必须提供至少一个端口参数。
- 端口源码更新要求 `buildtrees/<name@version>/src` 已存在。
- 端口源码更新仅支持 git 端口（`url` 以 `.git` 结尾）。
- `--recursive` 会递归更新端口依赖。

## 命令选项

| 选项         | 简写 | 类型 | 说明                         |
|--------------|------|------|------------------------------|
| --conf-repo  | -c   | 布尔 | 更新 `conf/` 仓库            |
| --ports-repo | -p   | 布尔 | 更新 `ports/` 仓库           |
| --force      | -f   | 布尔 | 强制更新（覆盖本地修改）      |
| --recursive  | -r   | 布尔 | 递归更新依赖（端口更新场景）   |

## 常用示例

```shell
# 更新 conf 仓库
celer update --conf-repo

# 更新 ports 仓库
celer update --ports-repo

# 更新单个端口源码仓库
celer update ffmpeg@3.4.13

# 递归更新端口及其依赖
celer update --recursive ffmpeg@3.4.13

# 强制更新 conf 仓库
celer update --conf-repo --force
```

## 说明

- 运行环境需要可用的 git。
- 在 PowerShell 输入中，命令会自动清理包名中的反引号转义字符。
