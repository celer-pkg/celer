# Integrate Tab completion

## Overview

The celer integrate command enables intelligent tab completion for Celer in your shell environment, significantly improving command-line productivity.

## Supported Shells

- Bash (Linux/macOS)
- PowerShell (Windows)
- Zsh (macOS/Linux)

## Command Syntax

```shell
celer integrate [--bash|--powershell|--zsh]
```

## Command Options

| Option	| Description	|
| -------- | -------- |
| --bash	| Enable tab completion for Bash shell	|
| --powershell	| Enable tab completion for PowerShell	|
| --zsh	| Enable tab completion for Zsh shell	|
| --remove	| Remove all Celer shell completions	|

## Usage Examples

### Specific shell integration

To enable tab completion for a specific shell, use the corresponding option:

```shell
celer integrate --bash
celer integrate --powershell
celer integrate --zsh
```

### Remove tab completion

To remove all Celer shell completions, use the --remove option:

```shell
celer integrate --bash --remove
celer integrate --powershell --remove
celer integrate --zsh --remove
```

> **Note:**   
>After integrate completion, you may need to restart your shell for the changes to take effect.