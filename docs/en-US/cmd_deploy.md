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
- `--snapshot=<path>` triggers snapshot export only after deployment succeeds.
- `--snapshot` accepts both relative and absolute paths.
- `--snapshot` must be a non-empty path.
- All port refs are resolved to concrete commits in one pass before any cloning begins, ensuring consistent code versions (see below).

## Command Options

| Option   | Short | Type    | Description                                       |
|----------|-------|---------|---------------------------------------------------|
| --force  | -f    | boolean | Force deployment, ignoring already installed libs |
| --snapshot | -     | string  | Export workspace snapshot after successful deploy |

## Common Examples

```shell
# Normal deployment
celer deploy

# Force deployment
celer deploy --force

# Deploy and export snapshot
celer deploy --snapshot=snapshots/2026-02-21

# Force deploy and export snapshot
celer deploy --force --snapshot=snapshots/rebuild
```

## Notes

- Make sure platform and project are configured before running deploy.
- Export is skipped if deployment fails.
- When deployment succeeds, you can use `toolchain_file.cmake` in CMake with `-DCMAKE_TOOLCHAIN_FILE=...`.

## Pre-Resolution of Refs

Before cloning, `deploy` resolves all ports' refs (branch/tag names) to commit hashes in a single pass, then clones uniformly. Results are saved as `snapshot.md` under `<workspace>/installed/celer/deployments/`.

This avoids the risk of remote pushes causing inconsistent commits for the same branch when resolving one-by-one during cloning, ensuring the entire deployment is based on a consistent code snapshot.

- **Fresh clone**: `git clone --branch <ref>` + `git reset --hard <commit>` — branch name preserved.
- **Existing repo**: `git reset --hard <commit>` directly.
