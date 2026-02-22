# Version 命令

`version` 命令用于打印当前 Celer 版本和 CMake 工具链快速用法提示。

## 命令语法

```shell
celer version
```

## 重要行为

- 无需参数和 flag。
- 输出包含 Celer 版本字符串。
- 输出包含 `toolchain_file.cmake` 在 CMake 中的使用示例。

## 常用示例

```shell
celer version
```

## 说明

- 输出中的工具链路径来自当前工作区路径解析结果。
