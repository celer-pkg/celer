# 如何初始化celer

celer依赖于一组配置，描述了工具链，根文件系统，cmake，工具和它依赖的第三方库。然后，celer将下载资源，拉取代码，编译，并将它们安装到指定目录。所基于的配置是一个叫做celer.toml的文件, 它存在于你的workspace的根目录下。

## 1. celer.toml的结构

celer.toml是celer的全局配置文件，它包含了平台，项目，缓存目录，工具链和其他配置。

```toml
[celer]
conf_repo_url = "https://gitee.com/phil-zhang/celer_conf.git"
conf_repo_ref = "master"
platform = ""
project = ""
job_num = 32
build_type = "release"

[cache_dir]
dir = "/remote/dirs/celer_cache"
readable = true
writable = true
```
>**Tips:**  
-conf_repo_url: 是celer的配置仓库地址，conf_repo_ref 是celer的配置仓库的分支；  
platform: 当前选则的平台名称，如果为空，意味着是用本地的编译环境，而不是celer的编译环境；  
project: 当前选择的项目名称，如果为空，意味着没有指定的项目，即默认值为"unname"；  
cache_dir: 是celer的缓存目录，它可以是本地目录，也可以是远程目录, 后面章节会详细介绍；
job_num: 是celer的编译线程数，默认为当前CPU的核数;
build_type: 默认为release。

## 2. 初始化celer

```
$ ./celer init --url=https://gitee.com/phil-zhang/celer_conf.git --branch=master
HEAD is now at 5a024af update config
Already on 'master'
Your branch is up to date with 'master'.
Already up to date.

[✔] ======== init celer successfully. ========
```

>Please note that `https://gitee.com/phil-zhang/celer_conf.git` is a test conf repo, you can use it to experience celer, and you can also create your own conf repo as a reference.

>**Tips:**
>
>  `https://gitee.com/phil-zhang/celer_conf.git` 是一个测试配置仓库，你可以使用它来体验celer，也可以创建自己的配置仓库作为参考。
