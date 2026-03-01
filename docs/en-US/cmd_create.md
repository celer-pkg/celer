# Create Command

The `create` command create a new draft platform, project, or port configuration.

## Command Syntax

```shell
celer create [flags]
```

## Important Behavior

- You must provide exactly one of `--platform`, `--project`, or `--port`.
- These three flags are mutually exclusive.
- `--port` must use `name@version` format.

## Command Options

| Option     | Type   | Description                      |
|------------|--------|----------------------------------|
| --platform | string | Create a platform configuration  |
| --project  | string | Create a project configuration   |
| --port     | string | Create a port configuration      |

## Common Examples

```shell
# Create a platform
celer create --platform=x86_64-linux-custom

# Create a project
celer create --project=my_project

# Create a port
celer create --port=opencv@4.11.0
```

## Validation Rules

- `--platform`: cannot be empty and cannot contain spaces.
- `--project`: cannot be empty.
- `--port`: must be `name@version` and both name/version must be non-empty.
