# Integrate command

&emsp;&emsp;The celer integrate command enables intelligent tab completion for Celer in your shell environment, significantly improving command-line productivity.

## Supported Shells

- Bash (Linux/macOS)
- PowerShell (Windows)
- Zsh (macOS/Linux)

## Command Syntax

```shell
celer integrate [--bash|--powershell]
```

## Command Options

| Option	    | Description	                                                    |
| ------------- | ----------------------------------------------------------------- |
| --bash	    | Enable tab completion for Bash shell	                            |
| --powershell	| Enable tab completion for PowerShell	                            |
| --remove	    | Combing with --bash and --powershell to remove tab completions	|

## Usage Examples

**1. Specific shell integration**

To enable tab completion for a specific shell, use the corresponding option:

```shell
celer integrate --bash
celer integrate --powershell
```

**2. Remove tab completion**

To remove all Celer shell completions, use the --remove option:

```shell
celer integrate --bash --remove
celer integrate --powershell --remove
```

> **Note:**   
>After integrate completion, you may need to restart your shell for the changes to take effect.