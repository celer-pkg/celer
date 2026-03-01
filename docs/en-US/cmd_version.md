# Version Command

The `version` command prints current Celer version and quick CMake toolchain usage hints.

## Command Syntax

```shell
celer version
```

## Important Behavior

- No flags or arguments are required.
- Output includes Celer version string.
- Output includes workspace `toolchain_file.cmake` usage examples for CMake.

## Example

```shell
celer version
```

## Notes

- The toolchain path shown is resolved from current workspace location.
