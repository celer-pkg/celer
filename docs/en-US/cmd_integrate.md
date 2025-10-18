# Integrate command

&emsp;&emsp;The integrate command enables intelligent tab completion for Celer in your shell environment, significantly improving command-line productivity.

## Current supported Shell

- Bash (Linux)
- Zsh (Linux)
- PowerShell (Windows)

## Command Syntax

```shell
celer integrate [--remove]
```

## Command Options

| Option	    | Description	                |
| ------------- | ----------------------------- |
| --remove      | remove tab completions	    |

## Usage Examples

**1. integration for current using shell**

```shell
celer integrate
```

>Celer can detect which kind of shell you're using and integrate tab completion for it.

**2. Remove tab completion**

```shell
celer integrate --remove
```

> **Note:**   
>Celer can also detect which kind of shell you'are using currently.