# Create 命令

`create` 命令用于创建平台、项目或端口的模板配置。

## 命令语法

```shell
celer create [flags]
```

## 重要行为

- 必须且只能提供 `--platform`、`--project`、`--port` 其中一个。
- 这三个 flag 互斥，不能同时使用。
- `--port` 必须使用 `name@version` 格式。

## 命令选项

| 选项       | 类型   | 说明             |
|------------|--------|------------------|
| --platform | 字符串 | 创建平台配置     |
| --project  | 字符串 | 创建项目配置     |
| --port     | 字符串 | 创建端口配置     |

## 常用示例

```shell
# 创建平台
celer create --platform=x86_64-linux-custom

# 创建项目
celer create --project=my_project

# 创建端口
celer create --port=opencv@4.11.0
```

## 参数校验规则

- `--platform`：不能为空，且不能包含空格。
- `--project`：不能为空。
- `--port`：必须是 `name@version` 且名称与版本都不能为空。
