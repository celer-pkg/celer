# Autoremove Command

The `autoremove` command removes libraries that are not required by the current project dependencies.

## Command Syntax

```shell
celer autoremove [flags]
```

## Important Behavior

- The command compares currently required packages vs installed/cached packages.
- Keep the libraries specified in the current project toml configuration and their sub-dependencies.
- Keep build-time library dependencies and their sub-dependencies.
- Unused runtime and dev packages are removed.
- It works against the current workspace context (`platform`, `project`, `build_type` in `celer.toml`).

## Command Options

| Option        | Short | Type    | Description                                           |
|---------------|-------|---------|-------------------------------------------------------|
| --purge       | -p    | boolean | Also remove cached package files/directories          |
| --build-cache | -c    | boolean | Also remove build cache of removed packages           |

## Common Examples

```shell
# Remove unused installed packages
celer autoremove

# Also remove cached package files
celer autoremove --purge

# Also remove build cache
celer autoremove --build-cache

# Remove installed packages, cache files, and build cache
celer autoremove --purge --build-cache
```

## Detection Scope

- Installed traces: `installed/celer/trace/*@<platform>@<project>@<build_type>.trace`
- Cached package folders: `packages/*@<platform>@<project>@<build_type>`

Because cached package folders are part of detection, running `celer autoremove --purge`
later can still remove stale package files even if an earlier run already removed trace data.