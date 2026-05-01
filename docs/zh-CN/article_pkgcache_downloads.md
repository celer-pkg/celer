# 缓存下载的构建工具

> **通过智能下载缓存减少对外网的依赖**

## 为什么需要下载缓存？

下载构建工具和依赖源可能会很**慢且不可靠**，特别是在网络连接不佳的地区。典型项目包含多个工具链和大型依赖时，可能需要花费**数小时**在下载上。Celer 的智能下载缓存可以帮助你：

- **加速全球构建** - 跳过多次构建中的重复下载
- **提高可靠性** - 当远程服务器不可用或缓慢时，可从缓存恢复文件
- **节省带宽** - 跨团队共享缓存下载，避免重复下载
- **支持离线构建** - 在网络有限或间歇性连接的环境中高效工作

## 工作原理

Celer 使用**基于 SHA-256 的验证**来缓存下载文件并确保数据完整性：

1. **检查本地缓存** - 在本地 downloads 目录中查找文件
2. **验证远程大小** - 与远程服务器比较文件大小，检测不完整或过期的文件
3. **从缓存恢复** - 如果缓存文件与远程大小匹配，从 `pkgcache/downloads` 恢复
4. **需要时下载** - 如果文件缺失或过期，从远程服务器下载
5. **验证并缓存** - 下载后验证 SHA-256 哈希值，并存储供将来使用

## 快速开始

### 第一步：配置缓存目录

在 `celer.toml` 中添加 `[pkgcache]` 部分以启用下载缓存：

```toml
[global]
	conf_repo = "https://github.com/celer-pkg/test-conf.git"
	platform = "x86_64-linux-ubuntu-22.04-gcc-11.5.0"
	project = "project_01"
	jobs = 32

[pkgcache]
	dir = "/home/test/pkgcache"  # 本地或网络挂载目录, 可以为FTP，SMB，或者NFS等
	writable = true              # 必须为 true 才能自动缓存下载
```

**重要**：需要设置 `writable = true` 才能自动缓存下载。

### 第二步：为构建工具添加 SHA-256 校验值

为每个构建工具或依赖源在构建工具配置中提供 SHA-256 校验值：

**buildtools/static/x86_64-linux.toml**
```toml
[[build_tools]]
  name = "cmake"
  version = "3.30.5"
  default = true
  url = "https://github.com/Kitware/CMake/releases/download/v3.30.5/cmake-3.30.5-linux-x86_64.tar.gz"
  sha256 = "f747d9b23e1a252a8beafb4ed2bc2ddf78cff7f04a8e4de19f4ff88e9b51dc9d"
  archive = "cmake-3.30.5-linux-x86_64.tar.gz"
  paths = ["cmake-3.30.5-linux-x86_64/bin"]
```

**conf/platforms/x86_64-linux-ubuntu-22.04-gcc-11.5.0.toml**
```toml
[rootfs]
  url = "https://github.com/celer-pkg/test-conf/releases/download/resource/ubuntu-base-22.04.5-base-amd64.tar.xz"
  sha256 = "08442eca9ccf64fd307d8a92582902315a66dc075216812d454596b1208da3bb"
  path = "ubuntu-base-22.04.5-base-amd64"
  pkg_config_path = ["usr/lib/x86_64-linux-gnu/pkgconfig"]
  lib_dirs = ["lib/x86_64-linux-gnu", "usr/lib/x86_64-linux-gnu"]
```

**SHA-256 的作用**：
- 提供数据完整性验证
- 通过文件标识启用缓存查询（格式：`{filename}-{sha256}.{ext}`）
- 检测缓存文件是否被破坏或修改

## 缓存目录结构

Celer 使用简单、扁平的结构组织缓存的下载文件：

```
/home/test/pkgcache/
    └── downloads/
        ├── cmake-3.30.5-linux-x86_64-f747d9b23...e9b51dc9d.tar.gz
        ├── gcc-ubuntu-11.5.0-x86_64-aarch64-linux-gnu-a99dee8e3ee2...56ebdad30c.tar.xz
        ├── ubuntu-base-22.04.5-base-arm64-47e7f499113.....297000486c6e76406232a.tar.xz
        └── ...
```

**缓存后的文件名格式**：`{basename}-{sha256}.{ext}`

## 验证工作原理

### 三层验证机制

Celer 使用三层验证来确保数据完整性：

1. 本地缓存查询
2. 缓存命中并检查和远端的大小比较（离线模式下不检查）
3. 下载后验证sha-256