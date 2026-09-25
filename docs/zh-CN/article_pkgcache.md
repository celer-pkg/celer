# PkgCache：共享缓存（fs / MinIO）

PkgCache 缓存构建产物、源码仓库和下载文件。后端只能选一个：

- **fs**：本地目录，或 NFS / SMB 挂载
- **minio**：S3 兼容对象存储

同时配两个会报错：`pkgcache can not configure both 'minio' and 'fs'`。

- [缓存构建产物](article_pkgcache_artifacts.md)
- [缓存源码仓库](article_pkgcache_repos.md)
- [缓存下载文件](article_pkgcache_downloads.md)
- [缓存 Python Wheel](article_pkgcache_python_wheels.md)

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

## MinIO 服务端准备（推荐，非强制）

上面填的 `access_key` 需要先在 MinIO 侧存在。怎么创建由你决定——MinIO Console、你自己的 IaC（Terraform / Ansible），或者用下面附带脚本，**本节只是推荐做法**：

- 建议按 `celer-minio-policy.json` 限制该凭证的权限：读/传/覆盖允许，删除拒绝。策略文件可以直接用 Console 导入，或用 `mc admin policy create` 加载，不一定要跑脚本。
- 建议 bucket `celer-cache` 开启对象锁（即版本控制）。
- 「celer 不会删除对象」本身是**代码层保证**（客户端里已经没有删除路径），与是否上策略无关；策略只是让这条限制在凭证层面也成立。

不想手写这些的话，仓库里的脚本可以一次搞定：

```text
deploy/minio/
├── celer-minio-access.sh      # 一次性初始化脚本（管理员执行，需要 mc）
├── celer-minio-policy.json    # append-only 策略，唯一来源
└── README.md                  # 用法、验证与管理员删除说明
```

```bash
cd deploy/minio
./celer-minio-access.sh --host=http://minio.example.com:9000 \
                        --admin-user=<root 账号> --admin-password=<密码>
```

脚本会做三件事（幂等，可重复执行）：

1. 创建 bucket `celer-cache`（`--with-lock`，开启对象锁 = 版本控制）
2. 把 `celer-minio-policy.json` 写入为策略 `celer-append-only`（读/传/覆盖允许，删除一律拒绝）
3. 创建凭证 `celer-pkgcache` 并挂上该策略，最后打印对应的 `celer configure` 命令

把打印出来的命令执行一次即可：

```bash
celer configure --pkgcache-minio-host=http://minio.example.com:9000 \
                --pkgcache-minio-access-key=celer-pkgcache \
                --pkgcache-minio-secret-key=<脚本打印的 secret>
```

几点说明：

- 脚本需要 MinIO **管理员**凭证，只用于建桶/策略/凭证；不会写进 `celer.toml`，也不会回显（先跑 `--dry-run` 可以只看它要执行哪些 `mc` 命令）。
- 这个凭证是一个普通 MinIO 用户（Console 里在 *Identity → Users → celer-pkgcache*），不是 service account。
- 它能读、能传、能覆盖，**不能删**。删除对象只能由管理员在 Console 或 `mc rm` 手动执行。
- 更多细节（`--rotate` 轮换、验证步骤、管理员删除）见 [`deploy/minio/README.md`](../../deploy/minio/README.md)。

## 选项

第一次配后端时，`[pkgcache.options]` 默认全是 `true`。改选项前必须先配好 fs 或 minio。

```bash
celer configure --pkgcache-writable=true
celer configure --pkgcache-cache-downloads=true
celer configure --pkgcache-cache-artifacts=true
celer configure --pkgcache-cache-repos=true
celer configure --pkgcache-cache-python-wheels=true
```

| 字段 | 说明 |
|------|------|
| `pkgcache.fs.dir` | 缓存根目录，必须已存在 |
| `pkgcache.minio.host` / `access_key` / `secret_key` | S3 地址和凭证 |
| `pkgcache.options.writable` | `true` 可写，`false` 只读 |
| `pkgcache.options.downloads` / `artifacts` / `repos` / `python_wheels` | 四种缓存开关 |

## 布局

fs 写在 `dir` 下；minio 写在 bucket `celer-cache` 里。前缀一样：

```text
artifacts-v0.2.7/   # 构建产物（按 Celer 版本隔离）
repos/              # 源码仓库
downloads/          # 下载文件
python-wheels/      # build_tools python 库的 pip wheel
```
