# Clean

## Overview

&emsp;&emsp;The celer deploy command performs a complete build and deployment cycle for all required third-party libraries based on the selected platform and project configuration. It simultaneously generates a **toolchain_file.cmake** for seamless integration with CMake-based projects.

## Command Syntax

```shell
celer deploy [flags]
```

## Command Options

| Option	        | Short flag | Description                                          |
| ----------------- | ---------- | -----------------------------------------------------|
| --build-type      | -b         | Specify build type (release/debug). Default: release |
| --dev-mode        | -d         | Deploy in dev mode. Default: false                   |

## Usage Examples

**1. Standard Deployment:**

```shell
celer deploy
```

>Standard deployment is always used in command line.

**2. Development Mode Deployment:**

```shell
celer deploy --dev-mode
```

>Development mode deployment is always called in generated **toolchain_file.cmake**.

**3. Deploy with build Type:**

```shell
celer deploy --build-type debug
```

>Build type can be specified in **celer.toml** file.

