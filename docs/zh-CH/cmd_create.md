# Create command

&emsp;&emsp;create命令用于创建新的平台、项目或端口。

## 命令语法

```shell
celer create [flags]
```

## 命令选项

| Option	        | Description              |
| ----------------- | -------------------------|
| --platform	    | create a new platform.   |
| --project 	    | create a new project.	   |
| --port	        | create a new port.	   |

## 使用实例

### 1. 创建一个新的平台

```shell
celer create --platform x86_64-linux-xxxx
```

>推荐的平台名称格式为 `[arch]-[os]-xxxx`;  
>生成的文件位于 **conf/platforms** 目录下；  
>然后需要根据目标环境打开生成的文件进行配置。

关于平台的详细信息，请参考 [平台介绍](./advance_platform.md)。

### 2. 创建一个新的项目

```shell
celer create --project xxxx
```

>生成的文件位于 **conf/projects** 目录下；  
>然后需要根据目标项目打开生成的文件进行配置。

关于项目的详细信息，请参考 [项目介绍](./advance_project.md)。

### 3. 创建一个新的端口

```shell
celer create --port xxxx
```

>当创建端口后，需要根据目标库打开生成的文件进行配置。生成的文件位于 **workspace/ports/glog/0.6.0/port.toml** 目录下。

关于端口的详细信息，请参考 [端口介绍](./advance_port.md)。
