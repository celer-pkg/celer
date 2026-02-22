# Integrate Command

The `integrate` command installs or removes shell completion integration for `celer`.

## Command Syntax

```shell
celer integrate [flags]
```

## Important Behavior

- Celer detects the current shell automatically.
- Supported shells: `bash`, `zsh`, and `PowerShell`.
- `--remove` switches behavior from register to unregister.
- Unsupported shell environments fail with a clear error.

## Command Options

| Option   | Type    | Description                  |
|----------|---------|------------------------------|
| --remove | boolean | Remove shell completion      |

## Common Examples

```shell
# Register completion for current shell
celer integrate

# Unregister completion for current shell
celer integrate --remove
```

## Notes

- Run in each shell environment separately if you use multiple shells.
- You may need to reopen the terminal to see completion changes.
