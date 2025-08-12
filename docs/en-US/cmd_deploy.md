# Deploy command

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

**1. Standard Deployment**

```shell
celer deploy
```

>Standard deployment is always used in command line.

**2. Dev mode deployment**

```shell
celer deploy --dev-mode/-d
```

>Dev mode deployment is always called in generated **toolchain_file.cmake**, and it will not override **toolchain_file.cmake**.

**3. Deploy with build Type**

```shell
celer deploy --build-type/-b debug
```

>The build type is read from **celer.toml** file, default is **release**. You can also specify build type in command line.

