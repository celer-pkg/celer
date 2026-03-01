# Search 命令

`search` 命令用于按精确值或通配符模式查找可用端口。

## 命令语法

```shell
celer search <pattern>
```

## 重要行为

- 必须且只能提供一个模式参数。
- 搜索范围包含全局 ports 和当前项目私有端口。
- 支持精确匹配、前缀匹配、后缀匹配、包含匹配。

## 支持的模式

| 模式      | 含义         |
|-----------|--------------|
| `name@v`  | 精确匹配     |
| `abc*`    | 前缀匹配     |
| `*abc`    | 后缀匹配     |
| `*abc*`   | 包含匹配     |

## 常用示例

```shell
# 精确匹配
celer search ffmpeg@5.1.6

# 前缀匹配
celer search open*

# 后缀匹配
celer search *ssl

# 包含匹配
celer search *mp4*
```

## 说明

- 超出支持形式的通配符表达式会被匹配器忽略。
- 匹配对象是 `name@version` 字符串。
