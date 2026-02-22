# Install 命令

`install` 命令用于在当前工作空间上下文中安装一个或多个包（`name@version`）。

## 命令语法

```shell
celer install <name@version> [<name@version> ...] [flags]
```

## 重要行为

- 至少提供一个包参数，可一次提供多个。
- 每个包参数都必须为 `name@version` 格式。
- 多包安装按输入顺序执行，任一包失败后命令立即停止。
- 安装前会检查循环依赖和版本冲突。
- 会同时在全局 `ports/` 和项目私有端口目录中查找端口。
- `--jobs` 与 `--verbose` 会覆盖本次命令的运行行为（对本次所有包生效）。

## 命令选项

| 选项          | 简写 | 类型   | 说明                                 |
|---------------|------|--------|--------------------------------------|
| --dev         | -d   | 布尔   | 作为开发依赖安装                      |
| --force       | -f   | 布尔   | 强制重装（如已安装则先移除）          |
| --recursive   | -r   | 布尔   | 结合重装语义，递归处理依赖            |
| --store-cache | -s   | 布尔   | 安装后将构建产物写入缓存              |
| --cache-token | -t   | 字符串 | 缓存令牌（通常与 `--store-cache` 配合）|
| --jobs        | -j   | 整数   | 并行构建任务数                        |
| --verbose     | -v   | 布尔   | 输出详细日志                          |

## 常用示例

```shell
# 标准安装
celer install ffmpeg@5.1.6

# 一次安装多个包
celer install ffmpeg@5.1.6 pkgconf@2.4.3

# 安装为开发依赖
celer install pkgconf@2.4.3 --dev

# 强制重装并递归处理依赖
celer install ffmpeg@5.1.6 --force --recursive

# 指定并行数
celer install ffmpeg@5.1.6 --jobs=8

# 安装并写入构建缓存
celer install ffmpeg@5.1.6 --store-cache --cache-token=token_xxx
```

## 参数校验规则

- 包列表不能为空。
- 每个包都必须能按 `@` 拆分为且仅为两段。
- 每个包的名称和版本都不能为空。

## 说明

- 在 PowerShell 中，命令会自动清理补全产生的反引号转义字符。
- 端口在可用源中不存在时会直接报错并退出。
