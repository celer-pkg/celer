# 如何缓存并共享第三方库

所有的第三方库都可以在我们的项目中编译，但是有时候我们希望共享它们。例如，我们想在我们的项目中使用ffmpeg，但是我们不想每次都编译它，因为它需要花费很多时间。幸运的是，celer可以帮助我们共享它们。

## 1. 在`celer.toml`中定义`cache_dirs`

你可以在`celer.toml`中定义`cache_dirs`，你可以定义多个缓存目录，你可以定义它们为可读或可写。如果缓存目录是可写的，celer将尝试将安装的库复制到缓存目录。如果缓存目录是可读的，celer将首先尝试在缓存目录中找到安装的库。

```toml
conf_repo_url = "https://gitee.com/phil-zhang/celer_conf.git"
conf_repo_ref = "master"
platform_name = "x86_64-linux-ubuntu-20.04.5"
project_name = "project_01"
job_num = 32

[[cache_dirs]]
dir = "/home/test/celer_cache"
readable = true
writable = true
```

# 2. 从源码编译并安装

当第三方库从源代码编译并安装时，其安装文件将被打包并存储在缓存目录中，缓存目录将如下所示：

```
mnt
└── celer_cache
    └── x86_64-linux-ubuntu-20.04
        └── project_01
            └── Release
                ├── ffmpeg@3.4.13.tar.gz
                ├── opencv@4.5.1.tar.gz
                ├── sqlite3@3.49.0.tar.gz
                ├── x264@stable.tar.gz
                ├── x265@4.0.tar.gz
                └── zlib@1.3.1.tar.gz
```

当执行`celer -install xxx@yyy`时，celer将尝试从缓存目录中查找已安装的文件，直到找到为止，如果没有找到，它将从源代码构建和安装它。
它必须满足五个匹配元素才能确定它正在搜索的缓存目标：

**五个必须满足的元素是：**

1. platform name: for example `x86_64-linux-ubuntu-20.04.5`
2. project name: for example `project_01`
3. library name: for example `ffmpeg`
4. library version: for example `3.4.13`
5. build config: for example `Release`