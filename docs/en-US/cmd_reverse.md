# Reverse Command

The `reverse` command finds which ports depend on a target package.

## Command Syntax

```shell
celer reverse <name@version> [flags]
```

## Important Behavior

- Exactly one package argument is required.
- Package must use `name@version` format.
- Reverse lookup scans available ports, then checks dependency lists.
- By default, only runtime dependencies are considered.
- `--dev` includes dev dependencies in lookup.

## Command Options

| Option | Short | Type    | Description                              |
|--------|-------|---------|------------------------------------------|
| --dev  | -d    | boolean | Include development dependencies         |

## Common Examples

```shell
# Runtime reverse dependencies
celer reverse eigen@3.4.0

# Include dev dependencies
celer reverse nasm@2.16.03 --dev
```

## Validation Rules

- Empty package is rejected.
- Missing `@` is rejected.
- Name must match `^[a-zA-Z0-9_-]+@[a-zA-Z0-9._-]+$`.
