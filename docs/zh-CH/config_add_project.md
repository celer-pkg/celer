# 添加一个新的项目配置

&emsp;&emsp;每个项目都有其独特的配置特征，包括项目依赖项、全局 CMake 变量、环境变量、C/C++ 宏定义以及编译选项等。Celer 建议为每个项目单独创建配置文件，用以描述这些项目特征。

要创建一个新的项目配置，运行以下命令：

```shell
celer create --project=project_003
```

> 生成的文件位于 **conf/projects** 目录中。  
> 然后，您需要打开生成的文件并根据您的目标项目进行配置。

## 1. 项目配置文件介绍

让我们来看一个示例项目配置文件，`project_003.toml`：

```toml
ports = [
    "x264@stable",
    "sqlite3@3.49.0",
    "ffmpeg@3.4.13",
    "zlib@1.3.1",
    "opencv@4.5.1"
]

vars = [
    "CMAKE_VAR1=value1",
    "CMAKE_VAR2=value2"
]

envs = [
    "ENV_VAR1=/home/ubuntu/ccache"
]

micros = [
    "MICRO_VAR1=111",
    "MICRO_VAR2"
]

compile_options = [
    "-Wall",
    "-O2"
]
```

以下是字段及其描述：

| 字段 | 描述 |
| --- | --- |
| ports | 定义当前项目依赖的第三方库。 |
| vars | 定义当前项目所需的全局 CMake 变量。 |
| envs | 定义当前项目所需的全局环境变量。 |
| micros | 定义当前项目所需的 C/C++ 宏定义。 |
| compile_options | 定义当前项目所需的编译选项。 |
