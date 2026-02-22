# Remove Command

The `remove` command uninstalls one or more packages from the workspace.

## Command Syntax

```shell
celer remove <name@version> [more_packages...] [flags]
```

## Important Behavior

- At least one package argument is required.
- Package names must match `name@version` format.
- Multiple packages can be removed in one command.
- `--dev` targets dev-installed packages.

## Command Options

| Option        | Short | Type    | Description                                   |
|---------------|-------|---------|-----------------------------------------------|
| --build-cache | -c    | boolean | Remove build cache with package removal       |
| --recursive   | -r    | boolean | Remove package dependencies recursively        |
| --purge       | -p    | boolean | Purge package files completely                |
| --dev         | -d    | boolean | Remove from development dependency side       |

## Common Examples

```shell
# Remove one package
celer remove ffmpeg@5.1.6

# Remove package recursively
celer remove ffmpeg@5.1.6 --recursive

# Remove package and purge package files
celer remove ffmpeg@5.1.6 --purge

# Remove dev package
celer remove nasm@2.16.03 --dev

# Full cleanup remove
celer remove ffmpeg@5.1.6 --recursive --purge --build-cache
```

## Validation Rules

- Empty package names are rejected.
- Leading/trailing spaces in package arguments are trimmed before validation.
- Names not matching `^[a-zA-Z0-9_-]+@[a-zA-Z0-9._-]+$` are rejected.
