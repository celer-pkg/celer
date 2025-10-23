# 缓存构建产物

&emsp;&emsp;虽然三方库有源码就能编译出来，但是大量的c/c++的库编译往往需要耗费很长时间，这对项目开发效率有严重影响。幸运的是Celer支持对编译产物进行精确的缓存管理，能有效避免同样的库以同样的需求被重复编译。

## 1. 定义 **cache_dirs**

&emsp;&emsp;一旦在 **celer.toml** 中定义了 `cache_dir`，每次构建库时，Celer 都会尝试从 `cache_dir` 中查找匹配的缓存产物。如果未找到，则会从源代码构建。

```toml
[global]
conf_repo = "https://gitee.com/phil-zhang/celer_conf.git"
platform = "x86_64-linux-ubuntu-22.04-gcc-11.5.5"
project = "project_01"
jobs = 32

[cache_dir]
dir = "/home/test/celer_cache"
```

## 2. 存储编译产物到 `cache_dir`

&emsp;&emsp;如下，需要在`celer.toml`中配置`cache_token`, 当执行 `celer install xxx --store-cache`编译成功后，Celer会尝试对编译产物进行打包并按预定规则存入 `cache_dir`.

```toml
[global]
conf_repo = "https://gitee.com/phil-zhang/celer_conf.git"
platform = "x86_64-linux-ubuntu-22.04-gcc-11.5.5"
project = "project_01"
jobs = 32

[cache_dir]
dir = "/home/test/celer_cache"
token = "token_xxxx"
```

## 3. 在不clone项目源码的情况下获取编译产物

&emsp;&emsp;当在`port.toml`中设置了`commit`, Celer就会读取`commit`的值来计算当前编译环境下的缓存key, 然后带着此缓存key去`cache_dir`里搜索匹配的编译产物.

```toml
[package]
url = "https://gitlab.com/libeigen/eigen.git"
ref = "3.4.0"
commit = "3147391d946bb4b6c68edd901f2add6ac1f31f8c"

[[build_configs]]
build_system = "cmake"
options = ["-DEIGEN_TEST_NO_OPENGL=1", "-DBUILD_TESTING=OFF"]
```

## 4. 缓存目录结构

```shell
/home/test
└── celer_cache
    └── x86_64-linux-ubuntu-22.04-gcc-11.5
        └── project_01
            └── release
                ├── ffmpeg@3.4.13
                |    ├── d536728c4eb8725a607055d1222d526830f62af21d7ba922073aa16c59a09068.tar.gz
                |    ├── f466728c4eb8725a607055d1222d526830f62a8861d7ba922073aa16c59a0906.tar.gz
                |    └── meta
                |        ├── d536728c4eb8725a607055d1222d526830f62af21d7ba922073aa16c59a09068.meta
                |        └── f466728c4eb8725a607055d1222d526830f62a8861d7ba922073aa16c59a0906.meta
                |    
                ├── opencv@4.5.1
                |    ├── li9834324c4eb8725a607055d1222d526830f62af21d7ba9220732316c5339a8.tar.gz
                |    ├── 43243246728c4eb8725a607055d1222d526830f62a8861d7ba9220796h43sfdf.tar.gz
                |    └── meta
                |        ├── li9834324c4eb8725a607055d1222d526830f62af21d7ba9220732316c5339a8.meta
                |        └── 43243246728c4eb8725a607055d1222d526830f62a8861d7ba9220796h43sfdf.meta
                └── others
```

&emsp;&emsp;当构建库时，Celer 会尝试从 `cache_dir` 中查找匹配的缓存产物，缓存键是根据库的构建环境和参数计算得出的。如果未找到，则会从源代码构建。构建成功后，Celer 会尝试打包构建产物并将其存储在 `cache_dir` 中。

## 5. 缓存key的构成

缓存使用从以下因素派生的复合键：

**1. 构建环境**

- 工具链（url, path, name, version, system architecture, 等）。
- 系统根目录（url, path, pkg_config_path，等）。

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
