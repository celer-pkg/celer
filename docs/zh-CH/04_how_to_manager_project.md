# 项目配置管理

&emsp;&emsp;不同的项目，它们的差别往往体现在依赖了不同的库，有不一样的全局cmake变量、注入了不一样的环境变量、不一样的C/C++宏，甚至不一样的编译选项。celer推荐通过给每个项目定义不同的project配置文件来描述不同项目的特征，通过`./celer --configure project=xxx`来切换目标项目。

## 1. 创建新的项目配置

```
$ ./celer create --project=project_003

[✔] ======== project_003 is created, please proceed with its refinement. ========
```

>随后，你需要打开生成的文件跟你的目标项目进行配置。

## 2. project 配置文件详解：

比如：`conf/projects/project_001.toml`:

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

>**Tips**:  
**ports**: 此处可以定义当前项目依赖的三方库，需要注意的是无需指定库的内部依赖的库，比如：如果FFmpeg在自身内部已经定义了对x264和x265的依赖，那么在这里再定义x264和x265是冗余的；  
**vars**: 此处可以定义当前项目需要的全局cmake变量；  
**envs**: 此处可以定义当前项目需要的全局环境变量；  
**micros**: 此处可以定义当前项目需要的C/C++宏；  
**compile_options**: 此处可以定义当前项目需要的编译选项。


## 3. project 配置切换

&emsp;&emsp;`platform`和`project`是两两组合的关系，可自由搭配，比如：虽然目标环境是`aarch64-linux`, 你也可以选择在`x86_64-linux`平台下编译和开发调试，然后选择project还是当前的，platform为`aarch64-linux`去编译和打包，最后部署到板子里去验证。

执行如下命令进行选择project配置：

```
$ ./celer configure --project=project_001

[✔] ======== current project: project_001. ========
```

>**Tip:**  
&emsp;&emsp;当配置了具体的project后，可以一键部署当前平台项目的开发环境，即：执行`./celer deploy`，Celer会按此project里定义编译三方库、动态生成`toolchain_file.cmake`，并在生成的`toolchain_file.cmake`里定义全局的cmake变量、 C/C++宏、全局的环境变量、C/C++的编译选项。