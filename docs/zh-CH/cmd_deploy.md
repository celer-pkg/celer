# Deploy 命令

&emsp;&emsp;**Deploy**命令执行所有必需的第三方库的完整构建和部署周期，根据选择的平台和项目配置。它同时生成一个**toolchain_file.cmake**文件，用于与基于CMake的项目无缝集成。

## 命令语法

```shell
celer deploy [flags]
```

## 命令选项

| Option	        | Short flag | Description                                          |
| ----------------- | ---------- | -----------------------------------------------------|
| --build-type      | -b         | Specify build type (release/debug). Default: release |
| --dev-mode        | -d         | Deploy in dev mode. Default: false                   |

## 命令示例

**1. 标准部署**

```shell
celer deploy
```

>标准部署始终在命令行中使用。

**2. Dev 模式部署**

```shell
celer deploy --dev-mode/-d
```

>Dev 模式部署始终在生成的**toolchain_file.cmake**中调用，并且不会覆盖**toolchain_file.cmake**。

**3. 部署时指定构建类型**

```shell
celer deploy --build-type/-b debug
```

>构建类型从**celer.toml**文件中读取，默认值为**release**。您也可以在命令行中指定构建类型。

