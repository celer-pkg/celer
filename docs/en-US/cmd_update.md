# Update Command

The `update` command updates conf/ports repositories or updates source repositories for specified ports.

## Command Syntax

```shell
celer update [flags] [name@version...]
```

## Important Behavior

- `--conf-repo` and `--ports-repo` are mutually exclusive.
- If neither repo flag is set, you must provide at least one port.
- Port source update requires existing `buildtrees/<name@version>/src`.
- Port source update works only for git-based ports (`url` ending with `.git`).
- `--recursive` updates dependencies recursively for port updates.

## Command Options

| Option       | Short | Type    | Description                             |
|--------------|-------|---------|-----------------------------------------|
| --conf-repo  | -c    | boolean | Update `conf/` repository               |
| --ports-repo | -p    | boolean | Update `ports/` repository              |
| --force      | -f    | boolean | Force update (overwrite local changes)  |
| --recursive  | -r    | boolean | Recursive dependency update (ports)     |

## Common Examples

```shell
# Update conf repo
celer update --conf-repo

# Update ports repo
celer update --ports-repo

# Update one port source repo
celer update ffmpeg@3.4.13

# Update port and dependency repos recursively
celer update --recursive ffmpeg@3.4.13

# Force update conf repo
celer update --conf-repo --force
```

## Notes

- Git must be available in environment.
- For PowerShell completion input, escaped backticks in package names are cleaned automatically.
