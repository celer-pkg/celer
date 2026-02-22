# Remove 命令

`remove` 命令用于从当前工作区卸载一个或多个包。

## 命令语法

```shell
celer remove <name@version> [more_packages...] [flags]
```

## 重要行为

- 至少需要一个包参数。
- 包名必须符合 `name@version` 格式。
- 支持在一条命令中移除多个包。
- `--dev` 会针对开发依赖侧安装的包进行移除。

## 命令选项

| 选项          | 简写 | 类型 | 说明                       |
|---------------|------|------|----------------------------|
| --build-cache | -c   | 布尔 | 移除包时同时清理构建缓存     |
| --recursive   | -r   | 布尔 | 递归移除依赖               |
| --purge       | -p   | 布尔 | 彻底清理 package 文件       |
| --dev         | -d   | 布尔 | 从开发依赖侧移除           |

## 常用示例

```shell
# 移除单个包
celer remove ffmpeg@5.1.6

# 递归移除
celer remove ffmpeg@5.1.6 --recursive

# 移除并清理 package 文件
celer remove ffmpeg@5.1.6 --purge

# 移除开发依赖侧包
celer remove nasm@2.16.03 --dev

# 全量清理式移除
celer remove ffmpeg@5.1.6 --recursive --purge --build-cache
```

## 参数校验规则

- 空包名会被拒绝。
- 参数前后空白会在校验前被自动去除。
- 不符合 `^[a-zA-Z0-9_-]+@[a-zA-Z0-9._-]+$` 的输入会被拒绝。
