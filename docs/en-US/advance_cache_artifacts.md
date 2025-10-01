# Advancement of cache artifacts

&emsp;&emsp;Although third-party libraries can be compiled from source code, the compilation of numerous C/C++ libraries often takes a long time, which can severely impact project development efficiency. Fortunately, Celer supports precise cache management of build artifacts, effectively preventing the same libraries from being repeatedly compiled with identical requirements.  
&emsp;&emsp;Furthermore, Celer supports retrievuing build artifacts of libraries without cloning their source code, this can be very useable for private libraries.

## 1. Retrieve build artifacts by defining `cache_dirs`

&emsp;&emsp;Once configured `cache_dir` in `celer.toml`, everytime when build a library Celer will try to find matched cache artifact from `cache_dir`.

```toml
[global]
conf_repo = "https://gitee.com/phil-zhang/celer_conf.git"
platform = "x86_64-linux-ubuntu-20.04.5"
project = "project_01"
jobs = 32

[cache_dir]
dir = "/home/test/celer_cache"
```

## 2. Save build artifacts to `cache_dir`

&emsp;&emsp;`cache_token` should be configured in `celer.toml` as below, when install port with `--store-cache`, Celer will try to pack build artifact and store it in `cache_dir`.

```toml
[global]
conf_repo = "https://gitee.com/phil-zhang/celer_conf.git"
platform = "x86_64-linux-ubuntu-20.04.5"
project = "project_01"
jobs = 32

[cache_dir]
dir = "/home/test/celer_cache"
token = "token_xxxx"
```

## 3. Retrieve build artifacts without cloning source code

&emsp;&emsp;When the commit is provided in the target library's `port.toml`, Celer calculates the cache key based on the commit value, and then searches for the matching build artifact in the `cache_dir` with this key.

```toml
[package]
url = "https://gitlab.com/libeigen/eigen.git"
ref = "3.4.0"
commit = "3147391d946bb4b6c68edd901f2add6ac1f31f8c"

[[build_configs]]
build_system = "cmake"
options = ["-DEIGEN_TEST_NO_OPENGL=1", "-DBUILD_TESTING=OFF"]
```

## 4. Cache directory structure

```
/home/test
└── celer_cache
    └── x86_64-linux-ubuntu-20.04
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

## 5. Cache key

The cache uses a composite key derived from:

**1. Build Environment**

- Toolchain (url, path, name, version, system architecture, etc).
- Sysroot (url, path, pkg_config_path, etc).

**2. Build Parameters**

- Library-specific options (e.g., FFmpeg's --enable-cross-compile, --enable-shared, --with-x264).
- Environment variables (CFLAGS/LDFLAGS).
- Selected build type (shared/static).

**3. Source Modifications**

- Applied patches: The patch file is included in the cache key computation.

**4. Dependency Graph**

- Recursive hashes of all dependencies (x264, nasm, etc.)
- Their respective build configurations, versions, patches.

>Any one of the above factors change, Celer will consider it as a different cache key, then build from source, generating new key and store new build aritifact with the new key.