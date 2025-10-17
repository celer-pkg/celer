# Integrate command

&emsp;&emsp;The integrate command enables intelligent tab completion for Celer in your shell environment, significantly improving command-line productivity.

## Current supported Shells

- Bash (Linux)
- PowerShell (Windows)

## Command Syntax

```shell
celer integrate [--bash|--powershell]
```

## Command Options

| Option	    | Description	                                                            |
| ------------- | ------------------------------------------------------------------------- |
| --bash	    | Enable tab completion for Bash shell	                                    |
| --powershell	| Enable tab completion for PowerShell	                                    |
| --unregister  | Combing with --bash and --powershell to unregister tab completions	    |

## Usage Examples

**1. Specific shell integration**

To enable tab completion for a specific shell, use the corresponding option:

```shell
celer integrate --bash
celer integrate --powershell
```

**2. Unregister tab completion**

To unregister all Celer shell completions, use the --unregister option:

```shell
celer integrate --bash --unregister
celer integrate --powershell --unregister
```

> **Note:**   
>After integrate completion, you may need to restart your shell for the changes to take effect.