# MinIO provisioning for celer pkgcache

celer uses the MinIO bucket `celer-cache` as an **append-only** cache: it lists,
downloads, uploads and overwrites objects, but it never deletes one. The client
exposes no delete operation at all (see
[`pkgcache/minio`](../../pkgcache/minio/minio_cache.go)), and the rule can be
enforced server-side by the credentials celer runs with.

Server-side enforcement is a deployment concern, not a celer feature, so the
pieces an administrator needs live here instead of inside the binary:

| File | Purpose |
|------|---------|
| `celer-minio-policy.json` | The IAM policy Celer's account should use. Single source of truth. |
| `celer-minio-access.sh` | Idempotent helper that applies the policy and creates the access key with `mc`. |

## Prerequisites

- `mc` on `PATH` ([minio client](https://min.io/docs/minio/linux/reference/minio-mc.html)):

  ```bash
  # linux
  mkdir -p ~/.local/bin
  curl -L https://dl.minio.org.cn/client/mc/release/linux-amd64/mc -o ~/.local/bin/mc
  chmod +x ~/.local/bin/mc
  mc --version

  # windows (Git Bash / MSYS2)
  mkdir -p ~/.local/bin
  curl -L https://dl.minio.org.cn/client/mc/release/windows-amd64/mc.exe -o ~/.local/bin/mc.exe
  mc.exe --version
  ```

## Usage

```bash
./celer-minio-access.sh --host http://minio.example.com:9000 \
  --admin-user <root-user> --admin-password <root-password>
```

Everything can also come from the environment, which makes the script usable
from a deployment pipeline (Ansible, Terraform `local-exec`, a container
entrypoint, …):

```bash
MINIO_HOST=http://minio.example.com:9000 \
  MINIO_ROOT_USER=... MINIO_ROOT_PASSWORD=... \
  ./celer-minio-access.sh
```

Flags: `--host`, `--admin-user`, `--admin-password`, `--runtime-access-key`
(default `celer-pkgcache`), `--rotate`, `--dry-run`, `--help`.

### What it creates

| Object | Role |
|--------|------|
| bucket `celer-cache` | created with object locking, which implies versioning |
| policy from `celer-minio-policy.json` | read + upload, delete denied |
| access key `celer-pkgcache` | a MinIO user (a user *is* an access key/secret pair), with the policy attached |

The credential is a plain MinIO user, so it lives under *Identity → Users → celer*.
The sidebar's *Access Keys* page only lists service accounts (the logged-in
identity's own, and a user's under its detail page), so it stays empty here. A
service account would have been the alternative, but its access key may not be
named after the user that owns it — and `celer` is the name we want.

The script:

1. registers the endpoint with a temporary `mc` configuration directory, so your
   own `~/.mc` aliases are never touched
2. creates `celer-cache` with object locking (`mb --with-lock`)
3. applies the policy from the JSON in this directory (`mc admin policy create`
   is an upsert, and MinIO refuses to remove a policy that is still attached)
4. creates the user (or replaces its secret with `--rotate`), then attaches the
   policy
5. prints the matching `celer configure --pkgcache-minio-*` command

Re-running it re-applies the policy and keeps an existing access key. Pass
`--rotate` to replace that key's secret (the new one is printed); a missing key is
always created. The admin password is never written to disk, and `--dry-run`
prints the commands with all secrets masked.

## What the policy allows

| Operation | Allowed | Reason |
|-----------|---------|--------|
| `ListBucket`, `GetBucketLocation`, `ListObjects` | yes | cache lookups |
| `GetObject` | yes | restore/download |
| `PutObject`, multipart upload | yes | store, and overwrite an existing entry |
| `DeleteObject` / `DeleteObjectVersion` | **no** | explicit `Deny` |
| `PutObjectRetention`, `PutObjectLegalHold`, `BypassGovernanceRetention` | **no** | keeps the object lock meaningful |
| `PutBucketPolicy`, `PutBucketVersioning`, `PutLifecycleConfiguration`, `DeleteBucket` | **no** | the credential cannot weaken the rule or expire objects |

`CreateBucket` is intentionally absent: `celer-minio-access.sh` creates the bucket, so the
runtime access key stays minimal. If the bucket is later removed, celer fails to
store an artifact and reports the missing bucket — ask an administrator to
re-create it with `mb --with-lock`.

## Verifying

```bash
# the access key and the policy attached to it
mc admin user info <alias> celer-pkgcache

# with the runtime key: upload works, delete is denied
mc alias set celer-runtime http://minio.example.com:9000 <access-key> <secret-key>
mc cp ./some.tar.gz celer-runtime/celer-cache/artifacts-v0.2.7/
mc rm celer-runtime/celer-cache/artifacts-v0.2.7/some.tar.gz   # expected: Access Denied
```

## Deleting objects (administrator only)

Nothing in celer deletes cache entries. When an object really must go, an
administrator removes it manually, e.g. in the MinIO console, or:

```bash
mc rm --recursive --force --dangerous <alias>/celer-cache/artifacts-v0.2.7/<platform>/<project>/<build-type>/<name@version>/
```

Because the bucket is versioned, overwriting an entry keeps the previous version
until an administrator prunes it with `mc rm --versions` or a lifecycle rule
applied by an administrator.

## Notes

- celer does **not** ship these operations as commands: no `pkgcache provision`,
  no admin credentials in `celer.toml`, no `mc` download. IAM stays with the
  platform team.
- Run this once per MinIO deployment; repeat it only to rotate keys or after
  editing `celer-minio-policy.json`.
