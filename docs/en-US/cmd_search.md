# Search Command

The `search` command looks up available ports by exact or wildcard pattern.

## Command Syntax

```shell
celer search <pattern>
```

## Important Behavior

- Exactly one pattern argument is required.
- Search covers both global ports and current-project ports.
- Matching supports exact, prefix, suffix, and contains patterns.

## Supported Patterns

| Pattern   | Meaning                         |
|-----------|---------------------------------|
| `name@v`  | Exact match                     |
| `abc*`    | Prefix match                    |
| `*abc`    | Suffix match                    |
| `*abc*`   | Contains match                  |

## Common Examples

```shell
# Exact match
celer search ffmpeg@5.1.6

# Prefix match
celer search open*

# Suffix match
celer search *ssl

# Contains match
celer search *mp4*
```

## Notes

- Patterns outside the supported wildcard forms are ignored by matcher logic.
- Matching is string-based on `name@version`.
