# Autoremove 命令

`autoremove` 命令用于移除“不属于当前项目依赖”的库。

## 命令语法

```shell
celer autoremove [flags]
```

## 重要行为

- 命令会对比“当前项目需要的包集合”与“已安装/已缓存包集合”。
- 保留当前project toml配置的runtime库文件以及它们的子依赖。
- 保留buildtime的库文件依赖，以及从buildtime依赖自身子依赖。
- 不在保留集合中的 runtime/buildtime 包会被移除。
- 作用范围基于当前工作空间上下文（`celer.toml` 中的 `platform`、`project`、`build_type`）。

## 命令选项

| 选项          | 简写 | 类型 | 说明                                   |
|---------------|------|------|----------------------------------------|
| --purge       | -p   | 布尔 | 同时删除缓存的 package 文件/目录        |
| --build-cache | -c   | 布尔 | 同时删除被移除包对应的构建缓存           |

## 常用示例

```shell
# 仅移除未使用的已安装包
celer autoremove

# 同时删除 package 缓存
celer autoremove --purge

# 同时删除构建缓存
celer autoremove --build-cache

# 同时删除已安装包、package 缓存和构建缓存
celer autoremove --purge --build-cache
```

## 检测范围

- 已安装追踪：`installed/celer/trace/*@<platform>@<project>@<build_type>.trace`
- 缓存包目录：`packages/*@<platform>@<project>@<build_type>`

由于缓存包目录也在检测范围内，即使上一次运行已删除 trace，
后续再执行 `celer autoremove --purge` 仍可清理残留的 package 文件。
