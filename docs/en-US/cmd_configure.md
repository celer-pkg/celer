# Configure Command

&emsp;&emsp;The `configure` command updates global settings for the current workspace.

## Command Syntax

```shell
celer configure [flags]
```

## Important Behavior

- In one command, you can configure only one setting group.
- Mixing flags from different groups fails.
- Multiple flags are allowed only inside the same related group (for example pkgcache, proxy, or ccache).

## Command Options
| Option                     | Type    | Description                                          |
|----------------------------|---------|------------------------------------------------------|
| --platform                 | string  | Set target platform                                  |
| --project                  | string  | Set current project                                  |
| --build-type               | string  | Set build type                                       |
| --downloads                | string  | Set downloads directory                              |
| --jobs                     | integer | Set parallel build jobs                              |
| --offline                  | boolean | Enable/disable offline mode                          |
| --verbose                  | boolean | Enable/disable verbose logging                       |
| --proxy-host               | string  | Set proxy host                                       |
| --proxy-port               | integer | Set proxy port                                       |
| --pkgcache-fs-dir             | string  | Set pkgcache fs backend directory                      |
| --pkgcache-minio-host         | string  | Set pkgcache minio backend host (fs and minio are mutually exclusive) |
| --pkgcache-minio-access-key   | string  | Set pkgcache minio access key                          |
| --pkgcache-minio-secret-key   | string  | Set pkgcache minio secret key                          |
| --pkgcache-writable           | boolean | Set whether the package cache is writable              |
| --pkgcache-cache-downloads    | boolean | Cache downloaded sources into the package cache        |
| --pkgcache-cache-artifacts    | boolean | Cache built artifacts into the package cache           |
| --pkgcache-cache-repos        | boolean | Cache source repos into the package cache              |
| --ccache-enabled           | boolean | Enable/disable ccache                                |
| --ccache-dir               | string  | Set ccache working directory                         |
| --ccache-maxsize           | string  | Set ccache max size                                  |
| --ccache-remote-storage    | string  | Set ccache remote storage URL                        |
| --ccache-remote-only       | boolean | Enable/disable remote-only cache mode                |
| --port                     | string  | Target port to update, in `name@version` form        |
| --port-url                 | string  | New source URL for the port (requires `--port`)      |
| --port-ref                 | string  | New ref for the port — branch/tag/commit (requires `--port`) |

## Common Examples

```shell
# Platform / project
celer configure --platform=x86_64-linux-ubuntu-22.04-gcc-11.5.0
celer configure --project=project_test_02

# Build settings
celer configure --build-type=Release
celer configure --downloads=/home/xxx/Downloads
celer configure --jobs=8

# Runtime switches
celer configure --offline=true
celer configure --verbose=false

# PkgCache group (can combine in one command)
# fs and minio are mutually exclusive backends; the first backend
# configuration enables all shared options by default.
celer configure --pkgcache-fs-dir=/home/xxx/cache --pkgcache-writable=true
celer configure --pkgcache-minio-host=http://minio.example.com:9000 \
                --pkgcache-minio-access-key=xxx \
                --pkgcache-minio-secret-key=yyy
celer configure --pkgcache-minio-secret-key=new-key   # rotate key alone, others unchanged
celer configure --pkgcache-cache-artifacts=true
celer configure --pkgcache-cache-downloads=true
celer configure --pkgcache-cache-repos=false

# Proxy group (can combine in one command)
celer configure --proxy-host=127.0.0.1 --proxy-port=7890

# ccache group (can combine in one command)
celer configure --ccache-enabled=true --ccache-maxsize=5G --ccache-remote-only=true
celer configure --ccache-remote-storage=http://server:8080/ccache

# Port group (update a port's url/ref — must combine with --port)
celer configure --port=eigen@3.4.0 --port-ref=3.4.1
celer configure --port=eigen@3.4.0 --port-url=https://example.com/eigen.git --port-ref=main
```

## Validation Rules

- `--platform`: must match a TOML file name under `conf/platforms`.
- `--project`: must match a TOML file name under `conf/projects`.
- `--build-type`: supports `Release`, `Debug`, `RelWithDebInfo`, `MinSizeRel` (stored in lowercase).
- `--downloads`: directory must already exist.
- `--jobs`: must be greater than `0`.
- `--pkgcache-fs-dir`: cannot be empty, and directory must already exist.
- `--pkgcache-minio-host` / `--pkgcache-minio-access-key` / `--pkgcache-minio-secret-key`: host must be reachable; empty values keep the current setting, so a key can be rotated alone.
- fs and minio backends are mutually exclusive; configuring one while the other is set fails.
- `--pkgcache-writable` / `--pkgcache-cache-downloads` / `--pkgcache-cache-artifacts` / `--pkgcache-cache-repos`: boolean options shared by all backends; a backend (fs or minio) must be configured first (or in the same command). All options default to `true` on first backend configuration.
- `--proxy-host`: cannot be empty.
- `--proxy-port`: must be greater than `0`.
- `--ccache-dir`: directory must already exist.
- `--ccache-maxsize`: must end with `M` or `G` (for example `512M`, `5G`).
- `--ccache-remote-storage`: empty value is allowed (clear setting), otherwise must be a valid URL with scheme and host, such as `http://server:8080/ccache`.
- `--ccache-remote-only`: boolean (`true` or `false`).
- `--port`: must be in `name@version` form and refer to an existing port; must be combined with `--port-url` or `--port-ref`.
- `--port-url` / `--port-ref`: must be used together with `--port`; using either flag alone fails.
