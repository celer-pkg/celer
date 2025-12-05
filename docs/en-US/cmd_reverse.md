# Reverse Command

The `celer reverse` command provides reverse dependency lookup functionality, allowing you to find which packages depend on a specified library.

## Usage

```bash
celer reverse [package_name@version] [flags]
```

## Description

The reverse command searches through all installed packages to find which ones depend on the specified package. This is useful for:

- Understanding package usage in your project
- Impact analysis before removing a package
- Dependency tree analysis from a bottom-up perspective
- Finding all consumers of a specific library

## Examples

### Basic Usage

Find all packages that depend on Eigen:
```bash
celer reverse eigen@3.4.0
```

### Include Development Dependencies

Find all packages that depend on NASM (including dev dependencies):
```bash
celer reverse nasm@2.16.03 --dev
```

## Flags

- `-d, --dev`: Include development dependencies in reverse lookup
- `-h, --help`: Show help for the reverse command

## Output Format

The command displays results in the following format:

```
[Reverse Dependencies]:
  package1@version1
  package2@version2
  package3@version3
```

If no reverse dependencies are found:
```
[Reverse Dependencies]:
no reverse dependencies found.
```

## Use Cases

1. **Impact Analysis**: Before removing a package, check what depends on it
2. **Refactoring**: Understanding which packages use a specific library
3. **Security Assessment**: Finding all packages affected by a vulnerable dependency
4. **Architecture Analysis**: Understanding dependency relationships in your project

## Related Commands

- [`celer tree`](./cmd_tree.md) - Show forward dependencies of a package
- [`celer search`](./cmd_search.md) - Search for available packages
- [`celer remove`](./cmd_remove.md) - Remove packages (useful after impact analysis)