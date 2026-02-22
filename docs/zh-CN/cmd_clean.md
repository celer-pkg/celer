# Clean 命令

`clean` 命令用于清理包或项目的构建缓存，并清理对应源码仓库状态。

## 命令语法

```shell
celer clean [flags] [target...]
```

## 重要行为

- 包含 `@` 的目标会按包处理（例如 `ffmpeg@5.1.6`）。
- 不含 `@` 的目标会按项目名处理（来自 `conf/projects`）。
- 未指定 `--all` 时，必须提供至少一个目标。
- `--all` 可不带目标，直接清理 `buildtrees` 下所有构建条目。
- 对项目目标，Celer 会同时清理该项目端口的普通缓存和 dev 缓存。
- `--dev` 主要影响包目标（清理 host-dev 构建缓存）。
- `--recursive` 会递归清理依赖和开发依赖。

## 命令选项

| 选项        | 简写 | 类型 | 说明                                   |
|-------------|------|------|----------------------------------------|
| --all       | -a   | 布尔 | 清理 `buildtrees` 下所有包构建条目      |
| --dev       | -d   | 布尔 | 按 dev/host-dev 模式清理包目标          |
| --recursive | -r   | 布尔 | 递归清理目标及其依赖                    |

## 常用示例

```shell
# 清理单个包
celer clean x264@stable

# 清理单个项目
celer clean project_test_02

# 以 dev 模式清理包
celer clean --dev automake@1.18

# 递归清理包及其依赖
celer clean --recursive ffmpeg@5.1.6

# 一次清理多个目标
celer clean x264@stable ffmpeg@5.1.6

# 清理所有构建条目
celer clean --all
```

## 说明

- `clean` 会删除构建目录和相关日志。
- `clean` 会对匹配端口执行源码清理。
- 使用 `--all` 时，会先删除每个 `buildtrees/<nameVersion>/` 下非 `src` 子目录。
