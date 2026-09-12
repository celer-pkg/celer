# PkgCache: Shared Cache (fs / MinIO)

PkgCache stores build artifacts, source repos, and downloaded files. Pick **one** backend:

- **fs**: a local directory, or an NFS / SMB mount
- **minio**: S3-compatible object storage

Configuring both fails: `pkgcache can not configure both 'minio' and 'fs'`.

- [Cache Build Artifacts](article_pkgcache_artifacts.md)
- [Cache Source Repositories](article_pkgcache_repos.md)
- [Cache Downloaded Files](article_pkgcache_downloads.md)

## fs

`dir` must already exist.

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

Set `host` to the S3 API port (usually `:9000`), not the console port. The bucket is always `celer-cache`; Celer creates it if missing.

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

Change one field and leave the rest:

```bash
celer configure --pkgcache-minio-secret-key=new-key
```

## Options

`[pkgcache.options]` defaults to all `true` when you first set a backend. Set `fs` or `minio` before changing these.

```bash
celer configure --pkgcache-writable=true
celer configure --pkgcache-cache-downloads=true
celer configure --pkgcache-cache-artifacts=true
celer configure --pkgcache-cache-repos=false
```

| Field | Description |
|------|-------------|
| `pkgcache.fs.dir` | Cache root. Must already exist. |
| `pkgcache.minio.host` / `access_key` / `secret_key` | S3 endpoint and credentials |
| `pkgcache.options.writable` | `true` writable, `false` read-only |
| `pkgcache.options.downloads` / `artifacts` / `repos` | The three cache toggles |

## Layout

fs writes under `dir`. minio writes into bucket `celer-cache`. Same prefixes:

```text
artifacts-v0.2.7/   # artifacts, isolated by Celer version
repos/              # source repos
downloads/          # downloaded files
```
