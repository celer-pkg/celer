# Install Command

The `install` command installs one or more ports (`name@version`) into the current workspace context.

## Command Syntax

```shell
celer install <name@version> [<name@version> ...] [flags]
```

## Important Behavior

- At least one package argument is required, and multiple packages are supported.
- Every package argument must be in `name@version` format.
- Multi-package install runs in argument order and stops on the first failed package.
- Celer checks circular dependencies and version conflicts before install.
- Celer searches ports in both global `ports/` and project-specific ports.
- After successful source builds, Celer best-effort stores package cache by default.
  Cache is written only when `package_cache.writable=true`; if cache is not configured,
  cache is readonly, or source was already locally modified before build,
  cache storing is skipped without failing install.
- `--jobs` and `--verbose` override install runtime behavior for this command run (all packages in it).

## Command Options

| Option        | Short | Type    | Description                                                |
|---------------|-------|---------|------------------------------------------------------------|
| --dev         | -d    | boolean | Install as dev dependency                                  |
| --force       | -f    | boolean | Reinstall target (remove first if installed)              |
| --recursive   | -r    | boolean | With force-style reinstall, include dependencies           |
| --jobs        | -j    | integer | Parallel build jobs                                        |
| --verbose     | -v    | boolean | Enable verbose output                                      |

## Common Examples

```shell
# Standard install
celer install ffmpeg@5.1.6

# Install multiple packages in one command
celer install ffmpeg@5.1.6 pkgconf@2.4.3

# Install as dev dependency
celer install pkgconf@2.4.3 --dev

# Force reinstall with dependencies
celer install ffmpeg@5.1.6 --force --recursive

# Install with custom parallelism
celer install ffmpeg@5.1.6 --jobs=8

# Default best-effort cache storing
celer install ffmpeg@5.1.6
```

## Validation Rules

- Package list cannot be empty.
- Each package must split into exactly two parts by `@`.
- Name and version must both be non-empty for each package.

## Notes

- On PowerShell, escaped backticks in completion input are automatically cleaned by the command.
- If a port does not exist in available port sources, install fails with a clear error.
