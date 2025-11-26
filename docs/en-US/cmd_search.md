# 🔍 Search Command

&emsp;&emsp;The `search` command is used to search for available ports (third-party libraries) based on a specified name or pattern.

## Command Syntax

```shell
celer search [pattern]
```

## 💡 Usage Examples

### 1️⃣ Exact Search

```shell
celer search ffmpeg@5.1.6
```

Output:
```
[Search result]:
ffmpeg@5.1.6
```

> Search using the complete library name and version number for exact match.

### 2️⃣ Prefix Search

```shell
celer search open*
```

Output:
```
[Search result]:
opencv@4.11.0
openssl@1.1.1w
openssl@3.5.0
```

> Search for all ports starting with `open`.

### 3️⃣ Suffix Search

```shell
celer search *ssl
```

Output:
```
[Search result]:
openssl@1.1.1w
openssl@3.5.0
```

> Search for all ports ending with `ssl`.

### 4️⃣ Keyword Search

```shell
celer search *mp4*
```

> Search for all ports containing `mp4`.

---

## 🎯 Search Pattern Syntax

Celer supports the following wildcard patterns:

| Pattern   | Description                        | Example                           |
|-----------|------------------------------------|---------------------------------|
| `xxx*`    | Match ports starting with xxx      | `ffmpeg*` → ffmpeg@5.1.6, ffmpeg@6.0 |
| `*xxx`    | Match ports ending with xxx        | `*ssl` → openssl@3.5.0          |
| `*xxx*`   | Match ports containing xxx         | `*cv*` → opencv@4.11.0          |
| `xxx@y.y` | Exact match for specific version   | `ffmpeg@5.1.6`                  |

---

## 📝 Notes

1. **Case Sensitive**: Search patterns are case-sensitive
2. **Wildcard Position**: `*` can appear at the beginning, end, or both
3. **Exact Match**: Without wildcards, performs exact matching
4. **Version Number**: Can search for specific versions or all versions of a library

---

## 📚 Related Documentation

- [Quick Start](./quick_start.md)
- [Install Command](./cmd_install.md) - Install searched ports
- [Port Configuration](./advance_port.md) - Learn about port configuration

---

**Need Help?** [Report an Issue](https://github.com/celer-pkg/celer/issues) or check our [Documentation](../../README.md)
