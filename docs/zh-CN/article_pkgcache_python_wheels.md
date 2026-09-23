# 缓存 Python Wheel

> **避免 `build_tools` 中 Python 库的重复 PyPI 下载**

端口会把 Python 库声明为构建工具，如 `build_tools = ["python3:mako", "python3:MarkupSafe@2.1.0"]`，每个通过 `pip` 装进构建 venv。**Python Wheel 缓存**把解析出的 wheel 集（含传递依赖）存进 `pkgcache/python-wheels`，后续安装直接 restore 出 wheelhouse，跑完全离线的 `pip install --no-index --find-links`——命中时不打 PyPI。

## 工作原理

每个 spec 走三层：

1. **L1** — 持久 wheelhouse（`downloads/wheelhouse-<pyminor>/`）：先用 `pip install --no-index --find-links` 离线装所有 spec，全部解析即完成。
2. **L2** — pkgcache 恢复：未满足的 spec 调 `Restore(cacheKey, wheelhouse)` 把缓存 wheel 拷进 wheelhouse。
3. **L3** — 在线 `pip download <spec>` 到 tmp 目录，合并进 wheelhouse，存入 pkgcache。

最终安装恒为 `pip install --no-index --find-links=<wheelhouse> <spec>`——无论 wheel 来自哪层都离线。

> 只有 `build_tools` 的 Python 库走此路径。Conda-forge env 创建和 Python 解释器本身不在范围。

## 目录结构

```text
python-wheels/<平台范围>/<Name>/<Version>/<wheel 文件>
```

- `<平台范围>` = `py<minor>-<GOOS>-<GOARCH>`，如 `py310-linux-amd64`。按 Python 小版本和 OS/架构隔离，不跨平台串台。
- `<Name>` / `<Version>` 来自 spec。

```text
pkgcache/python-wheels/py310-linux-amd64/
    ├── MarkupSafe/
    │   ├── 2.1.0/MarkupSafe-2.1.0-cp310-cp310-manylinux_2_17_x86_64.whl
    │   └── 2.1.5/MarkupSafe-2.1.5-cp310-cp310-manylinux_2_17_x86_64.whl
    └── Mako/1.2.0/
        ├── Mako-1.2.0-py3-none-any.whl
        └── ...                            # 传递依赖
```

多版本以兄弟目录共存。每个 wheel 用 SHA-256 校验：fs 放 `<wheel>.sha256` sidecar；minio 存对象元数据 `x-amz-meta-sha256`。

## 快速开始

```toml
[pkgcache.fs]
	dir = "/home/test/pkgcache"

[pkgcache.options]
	writable = true              # 写入 wheel 必须
```

```toml
# ports/m/mesa/24.0.0/port.toml
[package]
    build_tools = ["python3:mako", "python3:packaging", "python3:MarkupSafe@2.1.0"]
```

```bash
celer install mesa@24.0.0
```

- **首次**：`pip download` 打 PyPI，wheel 存进 `python-wheels/...`。
- **删 venv 重装**：无 PyPI 请求，离线 restore 后安装。
- **换工作区共享同一 `pkgcache`**：同样不打 PyPI，跨机器复用。

## 行为

- **写入**：已配 pkgcache、`writable = true`、在线、且该 spec 走了 L3。
- **恢复**：fs 离线可用（本地目录）；minio 离线返回未命中（连不上 bucket）。
- **已安装的包跳过缓存**（按精确 name 和 version 匹配）。
- **无独立开关**：跟随 `pkgcache.options.writable`。
