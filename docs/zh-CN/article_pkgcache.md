# PkgCache 共享缓存与 NFS 权限管理

> **通过 NFS 共享缓存 + chattr +a 实现团队级安全缓存**

## 概述

Celer 的 PkgCache 系统提供三种缓存能力：**构建产物缓存**、**源码仓库缓存**、**下载文件缓存**。当团队通过 NFS 共享同一份缓存目录时，需要解决两个核心问题：

1. **多用户并发写入** — 不同开发者的构建结果都需要写入共享目录
2. **防误删** — 任何人都不能删除其他人依赖的缓存文件

Celer 通过 Linux `chattr +a`（append-only）属性 + 系统用户组 + `celer setup --nfs-server-dir` 命令来降低误删风险，并保证多用户可以共同写入共享缓存。

## 缓存目录结构

配置 `pkgcache.dir` 后，Celer 会在该目录下按功能划分子目录：

```text
/mnt/data/pkgcache/                       # pkgcache.dir
    ├── artifacts-v0.2.7/                  # 构建产物缓存（按版本隔离）
    │   └── x86_64-linux-ubuntu-22.04-gcc-11.5.0/
    │       └── project_01/
    │           └── release/
    │               └── ffmpeg@3.4.13/
    │                   ├── d536728...09068.tar.gz
    │                   └── metas/
    │                       └── d536728...09068.meta
    ├── repos/                             # 源码仓库缓存
    │   ├── x264@stable/
    │   │   └── 31e19f92...c3a0d.tar.gz
    │   └── ffmpeg@6.1.1/
    │       └── 1f2e3d4c....tar.gz
    └── downloads/                         # 下载文件缓存
        ├── cmake-3.30.5-linux-x86_64-f747d9b23...e9b51dc9d.tar.gz
        └── gcc-ubuntu-11.5.0-x86_64-aarch64-linux-gnu-a99dee8e3ee2...56ebdad30c.tar.xz
```

三种缓存的详细说明请参阅：

- [缓存构建产物](article_pkgcache_artifacts.md) — 避免重复编译
- [缓存源码仓库](article_pkgcache_repos.md) — 避免重复 clone / 下载源码
- [缓存下载文件](article_pkgcache_downloads.md) — 减少对外网的依赖

## 配置方法

在 `celer.toml` 中添加 `[pkgcache]` 部分：

```toml
[main]
  conf_repo = "https://github.com/celer-pkg/test-conf.git"
  platform = "x86_64-linux-ubuntu-22.04-gcc-11.5.0"
  project = "project_01"

[pkgcache]
  dir = "/home/test/pkgcache"   # 本地目录或网络挂载目录（NFS、SMB 等）
  writable = true               # 是否允许写入缓存
  cache_artifacts = true        # 是否启用构建产物缓存
  cache_downloads = true        # 是否启用下载文件缓存
```

**配置项说明：**

| 字段 | 说明 |
|------|------|
| `dir` | 缓存根目录，必须是一个已存在的目录 |
| `writable` | `true` 时允许写入缓存，`false` 时只读 |
| `cache_artifacts` | 是否启用构建产物缓存 |
| `cache_downloads` | 是否启用下载文件缓存 |

也可以通过命令行动态配置：

```bash
celer configure --pkgcache-dir=/home/test/pkgcache
celer configure --pkgcache-writable=true
```

### 解决方案：chattr +a（append-only）

Linux 的 `chattr +a` 属性可以让目录变为"仅追加"模式。Celer 只对缓存目录设置该属性，不对缓存文件本身设置 `+a`：

- **允许**：创建新文件、覆盖写入已有文件
- **禁止**：删除文件、重命名文件

这正是缓存目录需要的行为 — 开发者可以通过 Celer 往里写新的缓存，也可以原位覆盖已有缓存，但不能删除别人的缓存。

### 权限模型

Celer 的 NFS 缓存权限模型包含以下层次：

| 层次 | 机制 | 作用 |
|------|------|------|
| 所有权 | `chown -R celer:celer` | 所有文件属于 celer 系统用户 |
| 组权限 | `chmod 2775`（目录）、`chmod 664`（文件） | celer 组成员可以读写 |
| Setgid | `chmod 2775` 中的 `2` | 新建文件/目录自动继承 celer 组 |
| 追加保护 | `chattr +a` | 禁止删除目录中的文件 |
| 定时加固 | cron 每分钟执行 `chattr +a` | 确保新建目录也受保护 |


### 服务端配置

在 NFS 服务器上执行：

```bash
sudo celer setup --nfs-server-dir=/srv/celer-cache
```

> **注意**：必须使用 `sudo` 运行，且仅支持 Linux。

命令按以下步骤执行：

1. **检查依赖工具** — 确认已安装 `nfs-kernel-server`（apt）或 `nfs-utils`（yum）、`passwd`/`shadow-utils`
2. **移除旧的 append-only 属性** — `find <dir> -type d -exec chattr -a {} ;`（因为之前可能已设置 `+a`，会阻止后续的 `chown`/`chmod`）
3. **创建 celer 系统用户** — `useradd --system --no-create-home --shell /usr/sbin/nologin celer`（幂等，已存在则跳过）
4. **设置文件所有权** — `chown -R celer:celer <nfs-dir>`
5. **设置目录权限** — `find <dir> -type d -exec chmod 2775 {} ;`（组可写 + setgid 位，新文件自动继承 celer 组）
6. **设置文件权限** — `find <dir> -type f -exec chmod 664 {} ;`（组可覆盖写入）
7. **将当前用户加入 celer 组** — `usermod -aG celer $SUDO_USER`
8. **添加 NFS 导出** — 写入 `/etc/exports`，选项为 `*(rw,sync,no_subtree_check,no_root_squash)`，并执行 `exportfs -ra`
   - `no_root_squash` 允许 NFS 客户端以 root 身份访问共享目录，便于必要时在客户端侧进行管理操作；目录保护本身由服务端的 `chattr +a` 和 cron 负责
9. **对所有目录应用 chattr +a** — `find <dir> -type d -exec chattr +a {} ;`
10. **安装定时任务** — 写入 `/etc/cron.d/celer-chattr`，每分钟对所有目录执行 `chattr +a`，确保 NFS 客户端创建的新目录也受保护

### 客户端配置

在 NFS 客户端机器上执行：

```bash
sudo celer setup --nfs-client-dir=/home/phil/celer-cache@10.0.8.60:/mnt/data/celer-cache
```

参数格式：`<挂载点>@<服务器>:<导出路径>`

命令按以下步骤执行：

1. **解析参数**: 按 `@` 分割为挂载点和服务端导出路径
2. **检查挂载点目录是否存在**: 明确存在的目录，celer不会帮你创建
3. **安装 NFS 客户端包**: `nfs-common`（apt）或 `nfs-utils`（yum）
4. **卸载已有挂载**: 幂等操作，忽略错误
5. **写入 fstab**: 先删除旧条目，再追加新条目：`<server>:<export> <mount> nfs rw,_netdev,noatime,rsize=1048576,wsize=1048576 0 0`
6. **挂载 NFS 共享**: `mount <挂载点>`
7. **创建 celer 组**: 在客户端也创建 celer 系统用户
8. **将当前用户加入 celer 组**: 确保有写入权限

### 配置完成后

组成员身份需要重新登录后生效，也可以立即执行：

```bash
newgrp celer
```