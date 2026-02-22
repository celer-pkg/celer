# Deploy Command

The `deploy` command builds and installs all ports defined by the current project.

## Command Syntax

```shell
celer deploy [flags]
```

## Important Behavior

- Before deployment, Celer checks circular dependencies and version conflicts across project ports.
- Deployment uses the current workspace context (`platform`, `project`, `build_type`).
- `--force` passes force mode to project deployment (reinstall behavior).
- `--export=<path>` triggers snapshot export only after deployment succeeds.

## Command Options

| Option   | Short | Type    | Description                                       |
|----------|-------|---------|---------------------------------------------------|
| --force  | -f    | boolean | Force deployment, ignoring already installed libs |
| --export | -     | string  | Export workspace snapshot after successful deploy |

## Common Examples

```shell
# Normal deployment
celer deploy

# Force deployment
celer deploy --force

# Deploy and export snapshot
celer deploy --export=snapshots/2026-02-21

# Force deploy and export snapshot
celer deploy --force --export=snapshots/rebuild
```

## Notes

- Make sure platform and project are configured before running deploy.
- Export is skipped if deployment fails.
- When deployment succeeds, you can use `toolchain_file.cmake` in CMake with `-DCMAKE_TOOLCHAIN_FILE=...`.
