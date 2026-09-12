# PkgCache：共享缓存（fs / MinIO）

PkgCache 缓存构建产物、源码仓库和下载文件。后端只能选一个：

- **fs**：本地目录，或 NFS / SMB 挂载
- **minio**：S3 兼容对象存储

同时配两个会报错：`pkgcache can not configure both 'minio' and 'fs'`。

- [缓存构建产物](article_pkgcache_artifacts.md)
- [缓存源码仓库](article_pkgcache_repos.md)
- [缓存下载文件](article_pkgcache_downloads.md)

## fs

`dir` 必须已存在。

```toml
[pkgcache.fs]
  dir = "/home/test/pkgcache"

[pkgcache.options]
  writable = true
  downloads = true
  artifacts = true
  repos = true
```

```bash
celer configure --pkgcache-fs-dir=/home/test/pkgcache
```

## minio

`host` 填 S3 API 端口（一般是 `:9000`），不要填控制台端口。bucket 固定为 `celer-cache`，没有就自动建。

```toml
[pkgcache.minio]
  host = "http://minio.example.com:9000"
  access_key = "xxx"
  secret_key = "yyy"
```

```bash
celer configure --pkgcache-minio-host=http://minio.example.com:9000 \
                --pkgcache-minio-access-key=xxx \
                --pkgcache-minio-secret-key=yyy
```

只改一项也可以，其余不动：

```bash
celer configure --pkgcache-minio-secret-key=new-key
```

## 选项

第一次配后端时，`[pkgcache.options]` 默认全是 `true`。改选项前必须先配好 fs 或 minio。

```bash
celer configure --pkgcache-writable=true
celer configure --pkgcache-cache-downloads=true
celer configure --pkgcache-cache-artifacts=true
celer configure --pkgcache-cache-repos=false
```

| 字段 | 说明 |
|------|------|
| `pkgcache.fs.dir` | 缓存根目录，必须已存在 |
| `pkgcache.minio.host` / `access_key` / `secret_key` | S3 地址和凭证 |
| `pkgcache.options.writable` | `true` 可写，`false` 只读 |
| `pkgcache.options.downloads` / `artifacts` / `repos` | 三种缓存开关 |

## 布局

fs 写在 `dir` 下；minio 写在 bucket `celer-cache` 里。前缀一样：

```text
artifacts-v0.2.7/   # 构建产物（按 Celer 版本隔离）
repos/              # 源码仓库
downloads/          # 下载文件
```
