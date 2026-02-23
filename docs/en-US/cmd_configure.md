# Configure Command

&emsp;&emsp;The `configure` command updates global settings for the current workspace.

## Command Syntax

```shell
celer configure [flags]
```

## Important Behavior

- In one command, you can configure only one setting group.
- Mixing flags from different groups fails.
- Multiple flags are allowed only inside the same related group (for example package cache, proxy, or ccache).

## Command Options
| Option                    | Type    | Description                            |
|---------------------------|---------|----------------------------------------|
| --platform                | string  | Set target platform                    |
| --project                 | string  | Set current project                    |
| --build-type              | string  | Set build type                         |
| --downloads               | string  | Set downloads directory                |
| --jobs                    | integer | Set parallel build jobs                |
| --offline                 | boolean | Enable/disable offline mode            |
| --verbose                 | boolean | Enable/disable verbose logging         |
| --proxy-host              | string  | Set proxy host                         |
| --proxy-port              | integer | Set proxy port                         |
| --package-cache-dir       | string  | Set package cache directory            |
| --package-cache-writable  | boolean | Set whether package cache is writable  |
| --ccache-enabled          | boolean | Enable/disable ccache                  |
| --ccache-dir              | string  | Set ccache working directory           |
| --ccache-maxsize          | string  | Set ccache max size                    |
| --ccache-remote-storage   | string  | Set ccache remote storage URL          |
| --ccache-remote-only      | boolean | Enable/disable remote-only cache mode  |

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

# Package cache group (can combine in one command)
celer configure --package-cache-dir=/home/xxx/cache --package-cache-writable=true

# Proxy group (can combine in one command)
celer configure --proxy-host=127.0.0.1 --proxy-port=7890

# ccache group (can combine in one command)
celer configure --ccache-enabled=true --ccache-maxsize=5G --ccache-remote-only=true
```

## Validation Rules

- `--platform`: must match a TOML file name under `conf/platforms`.
- `--project`: must match a TOML file name under `conf/projects`.
- `--build-type`: supports `Release`, `Debug`, `RelWithDebInfo`, `MinSizeRel` (stored in lowercase).
- `--downloads`: directory must already exist.
- `--jobs`: must be greater than `0`.
- `--package-cache-dir`: cannot be empty, and directory must already exist.
- `--package-cache-writable`: boolean; package cache dir must be configured first (or in the same command).
- `--proxy-host`: cannot be empty.
- `--proxy-port`: must be greater than `0`.
- `--ccache-dir`: directory must already exist.
- `--ccache-maxsize`: must end with `M` or `G` (for example `512M`, `5G`).
- `--ccache-remote-storage`: empty value is allowed (clear setting), otherwise must be a valid URL with scheme and host, such as `http://server:8080/ccache`.
- `--ccache-remote-only`: boolean (`true` or `false`).
