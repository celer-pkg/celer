# Update

## Overview

&emsp;&emsp;The celer update command synchronizes local repositories with their remote counterparts, ensuring you have the latest package configurations and build definitions. It supports targeted updates for different repository types.

## Command Syntax

```shell
celer update [flags]
```

## Command Options

| Option	        | Short flag | Description                                          |
| ----------------- | ---------- | -----------------------------------------------------|
| --conf-repo	    | -c	     | Update only the workspace conf repository.  |
| --ports-repo      | -p         | Update only the ports repository.                    |
| --force	        | -f	     | Force update the repository.                         |
| --recurse         | -r         | Recursively update all dependencies of a package.    |


## Usage Examples

**1. Update the workspace conf repository:**

```shell
celer update --conf-repo
```

**2. Update the ports repository:**

```shell
celer update --ports-repo
```

**3. Update the source of ports repository:**

```shell
celer update ffmpeg@3.4.13
```

**4. Update with combination of --force and --recurse:**

```shell
celer update --force --recurse ffmpeg@3.4.13
```

> **Note:**  
> The --force and --recurse flags can be used together to forcefully update a package and its recursive dependencies.
