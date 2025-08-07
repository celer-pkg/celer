# 如何初始化celer

&emsp;&emsp;如上一篇介绍，Celer依赖于一组**conf**配置，用于描述工具链，根文件系统以及不同的project的配置。然后，还需要一个叫**celer.toml**的文件用于指定当前workspace指定的platform和project, 它存在于你的workspace的根目录下。

## 1. celer.toml的结构

&emsp;&emsp;celer.toml是Celer的全局配置文件，在执行configure、create、init等命令时候会自动生成，它的内容如下：

```toml
[settings]
  conf_repo = ""
  ports_repo = "https://github.com/celer-pkg/ports.git"
  platform = ""
  project = ""
  job_num = 16
  build_type = "release"

[cache_dir]
  dir = ""
  readable = true
  writable = false
```

>**Tips:**  
**conf_repo**: Celer的conf配置文件的git仓库地址；  
**ports_repo**: 管理可供Celer编译的三方库git仓库地址, 默认为github地址，也可以是其他的git仓库地址；  
**platform**: 当前worksapce选定的platform，如果为空，意味着是用本地的编译环境进行编译，即不再是交叉编译；  
**project**: 当前workspace选定的项目，如果为空，意味着没有指定的项目，即默认值为"unname"；  
**job_num**: Celer在此指定全局的编译线程数，默认为当前CPU的核数，可以适当调整;
**build_type**: 默认为release，也可以是debug;  
**cache_dir**: Celer支持编译结果缓存，避免重复编译，这里可以配置为本地目录，也可以是局域网共享文件夹, 后面章节会详细介绍；

## 2. 初始化celer

```
$ ./celer init --url=https://github.com/celer-pkg/test-conf.git
HEAD is now at 5a024af update config
Already on 'master'
Your branch is up to date with 'master'.
Already up to date.

[✔] ======== initialize successfully. ========
```

>Please note that `https://gitee.com/phil-zhang/celer_conf.git` is a test conf repo, you can use it to experience celer, and you can also create your own conf repo as a reference.

>**Tips:**
>
>  `https://gitee.com/phil-zhang/celer_conf.git` 是一个测试配置仓库，你可以使用它来体验celer，也可以创建自己的配置仓库作为参考。

完整的celer.toml文件参考如下：

```toml
[celer]
  conf_repo = "https://github.com/celer-pkg/test-conf.git"
  ports_repo = "https://github.com/celer-pkg/ports.git"
  platform = "aarch64-linux-gnu-gcc-9.2"
  project = "test_project_02"
  job_num = 16
  build_type = "release"

[cache_dir]
  dir = "/home/phil/celer_cache"
  readable = true
  writable = false
```