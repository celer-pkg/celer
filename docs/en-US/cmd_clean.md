# Clean Command

The `clean` command removes build cache and cleans source repositories for packages or projects.

## Command Syntax

```shell
celer clean [flags] [target...]
```

## Important Behavior

- A target containing `@` is treated as a package (for example `ffmpeg@5.1.6`).
- A target without `@` is treated as a project name from `conf/projects`.
- Without `--all`, at least one target is required.
- `--all` works without targets and cleans all entries under `buildtrees`.
- For project targets, Celer cleans both normal and dev build cache for each project port.
- `--dev` mainly affects package targets (clean host-dev build cache).
- `--recursive` cleans dependencies and dev-dependencies recursively.

## Command Options

| Option      | Short | Type    | Description                                            |
|-------------|-------|---------|--------------------------------------------------------|
| --all       | -a    | boolean | Clean all package build entries under `buildtrees`     |
| --dev       | -d    | boolean | Clean package target in dev/host-dev mode              |
| --recursive | -r    | boolean | Clean target and dependencies recursively              |

## Common Examples

```shell
# Clean one package
celer clean x264@stable

# Clean one project
celer clean project_test_02

# Clean package in dev mode
celer clean --dev automake@1.18

# Recursively clean package and its dependencies
celer clean --recursive ffmpeg@5.1.6

# Clean multiple targets
celer clean x264@stable ffmpeg@5.1.6

# Clean all build entries
celer clean --all
```

## Notes

- `clean` removes build directories and related logs.
- `clean` also runs source cleanup for matched ports.
- For `--all`, non-`src` subdirectories in each `buildtrees/<nameVersion>/` entry are removed first.
