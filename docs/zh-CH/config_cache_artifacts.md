# 缓存构建产物

&emsp;&emsp;所有的第三方库都可以在我们的项目中编译，但是有时候我们想分享它们。例如，我们想在我们的项目中使用 ffmpeg，但是我们不想每个人都编译它，因为这需要花费很多时间。幸运的是，Celer 的缓存管理可以帮助我们。

## 1. 定义 **cache_dirs**

&emsp;&emsp;一旦在 **celer.toml** 中定义了 `cache_dir`，每次构建库时，Celer 都会尝试从 `cache_dir` 中查找匹配的缓存产物。如果未找到，则会从源代码构建。构建成功后，Celer 会尝试打包构建产物并将其存储在 `cache_dir` 中。

```
[global]
conf_repo = "https://gitee.com/phil-zhang/celer_conf.git"
platform = "x86_64-linux-ubuntu-20.04.5"
project = "project_01"
job_num = 32

[cache_dir]
dir = "/home/test/celer_cache"
```

## 2. Cache directory structure

```
/home/test
└── celer_cache
    └── x86_64-linux-ubuntu-20.04
        └── project_01
            └── release
                ├── ffmpeg@3.4.13
                |    ├── d536728c4eb8725a607055d1222d526830f62af21d7ba922073aa16c59a09068.tar.gz
                |    ├── f466728c4eb8725a607055d1222d526830f62a8861d7ba922073aa16c59a0906.tar.gz
                |    └── hash
                |        ├── d536728c4eb8725a607055d1222d526830f62af21d7ba922073aa16c59a09068.meta
                |        └── f466728c4eb8725a607055d1222d526830f62a8861d7ba922073aa16c59a0906.meta
                |    
                ├── opencv@4.5.1
                |    ├── li9834324c4eb8725a607055d1222d526830f62af21d7ba9220732316c5339a8.tar.gz
                |    ├── 43243246728c4eb8725a607055d1222d526830f62a8861d7ba9220796h43sfdf.tar.gz
                |    └── hash
                |        ├── li9834324c4eb8725a607055d1222d526830f62af21d7ba9220732316c5339a8.meta
                |        └── 43243246728c4eb8725a607055d1222d526830f62a8861d7ba9220796h43sfdf.meta
                └── others
```

&emsp;&emsp;当构建库时，Celer 会尝试从 `cache_dir` 中查找匹配的缓存产物，缓存键是根据库的构建环境和参数计算得出的。如果未找到，则会从源代码构建。构建成功后，Celer 会尝试打包构建产物并将其存储在 `cache_dir` 中。

## 3. 缓存key的构成

缓存使用从以下因素派生的复合键：

**1. 构建环境**

- 工具链（编译器路径/版本、系统架构）。
- 系统根目录（名称、配置）。

**2. 构建参数**

- 库特定选项（例如，FFmpeg 的 --enable-cross-compile、--enable-shared、--with-x264）。
- 环境变量（CFLAGS/LDFLAGS）。
- 选择的构建类型（共享/静态）。

**3. 源码修改**

- 应用的补丁：补丁文件的内容会被纳入复合缓存键的计算中。

**4. 库依赖关系**

- 所有依赖项的递归哈希（x264、nasm 等）。
- 它们各自的构建配置、版本、补丁。

>如果任何一个以上因素发生变化，Celer 会将其视为不同的缓存键，然后从源代码构建，生成新的键并将新的构建产物存储在新的键下。
