# Autoremove command

&emsp;&emsp;The autoremove command can remove installed packages but no required by the current project. This command is useful for reducing disk usage, clean up unused dependencies and maintain a tidy development environment.  

## Command Syntax

```shell
celer autoremove [flags]
```

## Command Options

| Option	        | Short flag | Description                                              	|
| ----------------- | ---------- | ------------------------------------------------------------ |
| --purge           | -p         | autoremove packages along with its package file.             |
| --remove‑cache	| -c	     | autoremove packages along with build cache.  	            |

## Usage Examples

**1. Autoremove not required libraries.**

```shell
celer autoremove
```

**2. Autoremote along with their packages.**

```shell
celer autoremove --purge/-p
```

**3. Autoremove along with their packages and build cache.**

```shell
celer autoremove --purge/-p --remove-cache/-c  
```