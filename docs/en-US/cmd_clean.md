# Clean command

&emsp;&emsp;The clean command allows to remove build cache files for a package or project, helping to free up disk space or resolve build issues caused by outdated cached data.

## Command Syntax

```shell
celer clean [flags]
```

## Command Options

| Option	        | Short flag | Description                                          |
| ----------------- | ---------- | -----------------------------------------------------|
| --all	            | -a	     | clean all packages.	                                |
| --dev             | -d         | clean package/project for dev mode.                  |
| --recurse	        | -r	     | clean package/project along with its dependencies.   |

## Usage Examples

**1. Clean build caches for all dependencies of specified project**

```shell
celer clean project_xxx
```

**2. Clean build caches for multi packages**

```shell
celer clean ffmpeg@5.1.6 opencv@4.11.0
```

**3. Clean build caches for dev packages**

```shell
celer clean --dev/-d pkgconf@2.4.3
```

**4. Combine recurse and cache**

```shell
celer clean --recurse/-r ffmpeg@5.1.6
```

**5. Clean all build caches under buildtrees directory**

```shell
celer clean --all
```
