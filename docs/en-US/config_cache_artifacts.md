# Cache artifacts

&emsp;&emsp;All third-party libraries can be compiled in our project, but sometimes we want to share them. For example, we want to use ffmpeg in our project, but we don't want to compile it by everyone, because it takes a lot of time. Fortunately, Celer's cache managerment can help us.

## 1. Define `cache_dirs`

&emsp;&emsp;Once define `cache_dir` in `celer.toml`, everytime when build a library Celer will try to find matched cache artifact from `cache_dir`. If not found then will build from source. After building successfull, Celer will try to pack build artifact and store it in `cache_dir`.

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
                |        ├── d536728c4eb8725a607055d1222d526830f62af21d7ba922073aa16c59a09068.txt
                |        └── f466728c4eb8725a607055d1222d526830f62a8861d7ba922073aa16c59a0906.txt
                |    
                ├── opencv@4.5.1
                |    ├── li9834324c4eb8725a607055d1222d526830f62af21d7ba9220732316c5339a8.tar.gz
                |    ├── 43243246728c4eb8725a607055d1222d526830f62a8861d7ba9220796h43sfdf.tar.gz
                |    └── hash
                |        ├── li9834324c4eb8725a607055d1222d526830f62af21d7ba9220732316c5339a8.txt
                |        └── 43243246728c4eb8725a607055d1222d526830f62a8861d7ba9220796h43sfdf.txt
                └── others
```

>When build a library, Celer will try to find matched cache artifact from `cache_dir` with a cache key. If not found then will build from source. After building successfull, Celer will try to pack build artifact and store it in `cache_dir`.

## 3. Cache key

The cache uses a composite key derived from:

1. Build Environment

    - Toolchain (compiler path/version, system architecture).
    - Sysroot (name, configure).

2. Build Parameters

    - Library-specific options (e.g., FFmpeg's --enable-cross-compile, --enable-shared, --with-x264).
    - Environment variables (CFLAGS/LDFLAGS).
    - Selected build type (shared/static).

3. Source Modifications

    - Applied patches: The patch file contents are factored into the composite cache key computation.

4. Dependency Graph
    - Recursive hashes of all dependencies (x264, nasm, etc.)
    - Their respective build configurations, versions, patches.

>Any one of the above factors change, Celer will consider it as a different cache key, then build from source, generating new key and store new build aritifact with the new key.