# Clean command

&emsp;&emsp;The clean command can remove build cache files for a package or project, helping to free up disk space or resolve build issues caused by outdated build cache.

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

**1. Clean build cache for all dependencies for a project**

```shell
celer clean project_xxx
```

**2. Clean build cache for multi packages**

```shell
celer clean ffmpeg@5.1.6 opencv@4.11.0
```

**3. Clean build cache for dev packages**

```shell
celer clean --dev/-d pkgconf@2.4.3
```

**4. Clean build cache alone with its dependencies**

```shell
celer clean --recurse/-r ffmpeg@5.1.6
```

**5. Clean all build cache under buildtrees directory**

```shell
celer clean --all
```

>**Note:**  
> **1.** For git repo library, the clean command will clean its local git repo.  
> **2.** For url download library, the clean command will unzip archive file to replace the src directory.