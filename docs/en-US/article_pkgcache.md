# PkgCache: Shared Cache (fs / MinIO)

PkgCache stores build artifacts, source repos, and downloaded files. Pick **one** backend:

- **fs**: a local directory, or an NFS / SMB mount
- **minio**: S3-compatible object storage

Configuring both fails: `pkgcache can not configure both 'minio' and 'fs'`.

- [Cache Build Artifacts](article_pkgcache_artifacts.md)
- [Cache Source Repositories](article_pkgcache_repos.md)
- [Cache Downloaded Files](article_pkgcache_downloads.md)
- [Cache Python Wheels](article_pkgcache_python_wheels.md)

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

## Preparing the MinIO Side (Recommended, Not Required)

The `access_key` configured above must already exist in MinIO. How you create it
is up to you — the MinIO Console, your own IaC (Terraform / Ansible), or the
script shipped here. **This section is a recommendation, not a requirement:**

- Recommended: restrict that credential with `celer-minio-policy.json` (read,
  upload and overwrite allowed; delete denied). The file can be imported through
  the Console or loaded with `mc admin policy create`; the script is not required.
- Recommended: enable object locking (i.e. versioning) on the `celer-cache` bucket.
- "celer never deletes objects" is a **code-level guarantee** (the client has no
  delete path at all) and does not depend on any policy; the policy only extends
  that rule to the credential itself.

If you would rather not do this by hand, the script in this repository does it in
one go:

```text
deploy/minio/
├── celer-minio-access.sh      # one-shot setup (run by an administrator, needs mc)
├── celer-minio-policy.json    # the append-only policy, single source of truth
└── README.md                  # usage, verification, admin deletion
```

```bash
cd deploy/minio
./celer-minio-access.sh --host=http://minio.example.com:9000 \
                        --admin-user=<root-user> --admin-password=<password>
```

The script does three things (idempotent, safe to re-run):

1. creates the bucket `celer-cache` (`--with-lock`, i.e. object locking = versioning)
2. writes `celer-minio-policy.json` as the policy `celer-append-only` (read, upload and overwrite allowed; delete denied)
3. creates the credential `celer-pkgcache` with that policy attached, then prints the matching `celer configure` command

Run the printed command once:

```bash
celer configure --pkgcache-minio-host=http://minio.example.com:9000 \
                --pkgcache-minio-access-key=celer-pkgcache \
                --pkgcache-minio-secret-key=<secret printed by the script>
```

Notes:

- The script needs MinIO **administrator** credentials, used only to create the bucket, policy and credential. They are never written to `celer.toml` and never echoed (`--dry-run` prints the `mc` commands without running them).
- The credential is a plain MinIO user (Console: *Identity → Users → celer-pkgcache*), not a service account.
- It can read, upload and overwrite, but **never delete**. Removing an object stays a manual administrator action in the Console or with `mc rm`.
- More details (`--rotate`, verification, admin deletion): [`deploy/minio/README.md`](../../deploy/minio/README.md).

## Options

`[pkgcache.options]` defaults to all `true` when you first set a backend. Set `fs` or `minio` before changing these.

```bash
celer configure --pkgcache-writable=true
celer configure --pkgcache-cache-downloads=true
celer configure --pkgcache-cache-artifacts=true
celer configure --pkgcache-cache-repos=true
celer configure --pkgcache-cache-python-wheels=true
```

| Field | Description |
|------|-------------|
| `pkgcache.fs.dir` | Cache root. Must already exist. |
| `pkgcache.minio.host` / `access_key` / `secret_key` | S3 endpoint and credentials |
| `pkgcache.options.writable` | `true` writable, `false` read-only |
| `pkgcache.options.downloads` / `artifacts` / `repos` / `python_wheels` | The four cache toggles |

## Layout

fs writes under `dir`. minio writes into bucket `celer-cache`. Same prefixes:

```text
artifacts-v0.2.7/   # artifacts, isolated by Celer version
repos/              # source repos
downloads/          # downloaded files
python-wheels/      # pip wheels for build_tools python libraries
```
