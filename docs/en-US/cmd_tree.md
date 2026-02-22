# Tree Command

The `tree` command prints dependency trees for a package or a project.

## Command Syntax

```shell
celer tree <target> [flags]
```

## Important Behavior

- Exactly one target is required.
- If target contains `@`, it is treated as a package.
- Otherwise, target is treated as a project name.
- Command validates circular dependencies and version conflicts before printing.
- By default, both runtime and dev dependencies are shown.

## Command Options

| Option     | Type    | Description                            |
|------------|---------|----------------------------------------|
| --hide-dev | boolean | Hide dev dependencies in tree output   |

## Common Examples

```shell
# Package dependency tree
celer tree ffmpeg@5.1.6

# Package tree without dev dependencies
celer tree ffmpeg@5.1.6 --hide-dev

# Project dependency tree
celer tree project_test_02
```

## Notes

- Output includes dependency counts (`dependencies`, `dev_dependencies`).
- Large targets can produce long tree output.
