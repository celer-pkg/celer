# Integrate Command

The `integrate` command installs or removes shell completion integration for `celer`.

## Command Syntax

```shell
celer integrate [flags]
```

## Important Behavior

- On Linux, Celer detects the current shell automatically (`bash` or `zsh`).
- On Windows, Celer always configures `PowerShell` completion.
- `--remove` switches behavior from register to unregister.
- Unsupported shell environments fail with a clear error.

## Command Options

| Option   | Type    | Description                  |
|----------|---------|------------------------------|
| --remove | boolean | Remove shell completion      |

## Common Examples

```shell
# Register completion (Linux: current shell, Windows: PowerShell)
celer integrate

# Unregister completion (Linux: current shell, Windows: PowerShell)
celer integrate --remove
```

## Notes

- On Linux, run in each shell environment separately if you use multiple shells.
- You may need to reopen the terminal to see completion changes.
