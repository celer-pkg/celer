# Install Command

The `install` command installs one port (`name@version`) into the current workspace context.

## Command Syntax

```shell
celer install <name@version> [flags]
```

## Important Behavior

- Exactly one package argument is required.
- Package must be in `name@version` format.
- Celer checks circular dependencies and version conflicts before install.
- Celer searches ports in both global `ports/` and project-specific ports.
- `--jobs` and `--verbose` override runtime install behavior for this run.

## Command Options

| Option        | Short | Type    | Description                                                |
|---------------|-------|---------|------------------------------------------------------------|
| --dev         | -d    | boolean | Install as dev dependency                                  |
| --force       | -f    | boolean | Reinstall target (remove first if installed)              |
| --recursive   | -r    | boolean | With force-style reinstall, include dependencies           |
| --store-cache | -s    | boolean | Store build artifacts to package cache after installation  |
| --cache-token | -t    | string  | Cache token (typically used with `--store-cache`)          |
| --jobs        | -j    | integer | Parallel build jobs                                        |
| --verbose     | -v    | boolean | Enable verbose output                                      |

## Common Examples

```shell
# Standard install
celer install ffmpeg@5.1.6

# Install as dev dependency
celer install pkgconf@2.4.3 --dev

# Force reinstall with dependencies
celer install ffmpeg@5.1.6 --force --recursive

# Install with custom parallelism
celer install ffmpeg@5.1.6 --jobs=8

# Install and store artifact cache
celer install ffmpeg@5.1.6 --store-cache --cache-token=token_xxx
```

## Validation Rules

- Input cannot be empty.
- Input must split into exactly two parts by `@`.
- Name and version must both be non-empty.

## Notes

- On PowerShell, escaped backticks in completion input are automatically cleaned by the command.
- If a port does not exist in available port sources, install fails with a clear error.
