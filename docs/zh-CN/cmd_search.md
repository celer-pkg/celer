# 🔍 搜索命令（Search）

&emsp;&emsp;`search` 命令用于根据指定的名称或模式搜索可用的端口（第三方库）。

## 命令语法

```shell
celer search [pattern]
```

## 💡 使用示例

### 1️⃣ 精确搜索

```shell
celer search ffmpeg@5.1.6
```

输出结果：
```
search results that match pattern 'ffmpeg@5.1.6':
------------------------------------------
ffmpeg@5.1.6
------------------------------------------
total: 1 port(s)
```

> 使用完整的库名和版本号进行精确搜索。

### 2️⃣ 按前缀搜索

```shell
celer search open*
```

输出结果：
```
search results that match pattern 'open*':
------------------------------------------
opencv@4.11.0
opencv_contrib@4.11.0
openssl@1.1.1w
openssl@3.5.0
------------------------------------------
total: 4 port(s)
```

> 搜索所有以 `open` 开头的端口。

### 3️⃣ 按后缀搜索

```shell
celer search *ssl
```

输出结果：
```
search results that match pattern '*ssl':
------------------------------------------
openssl@1.1.1w
openssl@3.5.0
------------------------------------------
total: 2 port(s)
```

> 搜索所有以 `ssl` 结尾的端口。

### 4️⃣ 按关键词搜索

```shell
celer search *mp4*
```

> 搜索所有包含 `mp4` 的端口。

---

## 🎯 搜索模式语法

Celer 支持以下通配符模式：

| 模式      | 说明                      | 示例                  |
|-----------|---------------------------|-----------------------|
| `xxx*`    | 匹配以 xxx 开头的端口     | `ffmpeg*` → ffmpeg@5.1.6, ffmpeg@6.0 |
| `*xxx`    | 匹配以 xxx 结尾的端口     | `*ssl` → openssl@3.5.0 |
| `*xxx*`   | 匹配包含 xxx 的端口       | `*cv*` → opencv@4.11.0 |
| `xxx@y.y` | 精确匹配特定版本          | `ffmpeg@5.1.6` |

---

## 📝 注意事项

1. **大小写敏感**：搜索模式区分大小写
2. **通配符位置**：`*` 可以出现在开头、结尾或两端
3. **精确匹配**：不使用通配符时执行精确匹配
4. **版本号**：可以搜索特定版本或搜索所有版本的库

---

## 📚 相关文档

- [快速开始](./quick_start.md)
- [Install 命令](./cmd_install.md) - 安装搜索到的端口
- [端口配置](./article_port.md) - 了解端口配置

---

**需要帮助？** [报告问题](https://github.com/celer-pkg/celer/issues) 或查看我们的 [文档](../../README.md)
