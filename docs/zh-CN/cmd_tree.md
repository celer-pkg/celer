# Tree 命令

`tree` 命令用于打印包或项目的依赖树。

## 命令语法

```shell
celer tree <target> [flags]
```

## 重要行为

- 必须且只能提供一个目标参数。
- 目标包含 `@` 时按包处理。
- 否则按项目名处理。
- 输出前会检查循环依赖与版本冲突。
- 默认同时展示运行时依赖和开发依赖。

## 命令选项

| 选项       | 类型 | 说明                   |
|------------|------|------------------------|
| --hide-dev | 布尔 | 在树输出中隐藏开发依赖 |

## 常用示例

```shell
# 查看包依赖树
celer tree ffmpeg@5.1.6

# 隐藏开发依赖
celer tree ffmpeg@5.1.6 --hide-dev

# 查看项目依赖树
celer tree project_test_02
```

## 说明

- 输出末尾会给出依赖统计（`dependencies`、`dev_dependencies`）。
- 目标依赖较多时，树输出会较长。
