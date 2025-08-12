# Autoremove command

&emsp;&emsp;The celer autoremove command removes installed packages that are no required by the current project. This helps clean up unused dependencies and maintain a tidy development environment.

## Command Syntax

```shell
celer autoremove [flags]  
```

## Command Options

| Option	        | Short flag | Description                                              	|
| ----------------- | ---------- | ------------------------------------------------------------ |
| --purge           | -p         | Remove packages along with their associated package files.   |
| --remove‑cache	| -c	     | Remove packages along with their build cache.	            |

## Usage Examples

**1. Standard autoremove not required libraries**

```shell
celer autoremove  
```

**2. Autoremote not required libraries, along with their packages**

```shell
celer autoremove --purge/-p
```

**3. Combine purge and cache removal**

```shell
celer autoremove --purge/-p --remove-cache/-c  
```

>This command is useful for optimizing disk space and keeping the project environment clean by removing unnecessary dependencies.