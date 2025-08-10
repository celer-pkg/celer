# How to automatically generate cmake configuration files

&emsp;&emsp;We all know that many third-party libraries do not use cmake to build, and after installation, they will not generate cmake configuration files. This makes it not easy to use cmake to find them. Although we can use `pkg-config` to find them, it can only be used on Linux. Now, Celer can generate cmake configuration files for them, so they can be used on any platform.

## 1. How to generate cmake configuration files for libraries without components

For example, x264, you should create a cmake_config.toml file in the version directory of the port.

```
└── x264
    └── stable  
        ├── cmake_config.toml
        └── port.toml
```

The content of this file is as follows, we can define different filenames for different platforms.

```toml
namespace = "x264"

[linux_static]
filename = "libx264.a"

[linux_shared]
filename = "libx264.so.164"
soname = "libx264.so"

[windows_static]
filename = "x264.lib"

[windows_shared]
filename = "libx264-164.dll"
impname = "libx264.lib"

```

After compiling and installing, you can see the generated cmake configuration files as follows:

```
lib
└── cmake
    └─── x264
        ├── x264Config.cmake
        ├── x264ConfigVersion.cmake
        ├── x264Targets.cmake
        └── x264Targets-release.cmake
```

Finally, you can use it in your cmake project as follows:

```cmake
find_package(x264 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE x264::x264)
```

> **Note:**  
> &emsp;&emsp;Note that the namespace is defined in the cmake_config file. If it is not defined, it will be the same as the library name. The namespace is also the prefix of the config file name.

## 2. How to generate cmake configuration files for libraries with components

For example, ffmpeg, you should create a cmake_config.toml file in the version directory of the port.

```
└── ffmpeg
    └── 3.4.13
        ├── cmake_config.toml
        └── port.toml
```

```toml
namespace = "FFmpeg"

[linux_shared]
[[linux_shared.components]]
component = "avutil"
soname = "libavutil.so.55"
filename = "libavutil.so.55.78.100"
dependencies = []

[[linux_shared.components]]
component = "avcodec"
soname = "libavcodec.so.57"
filename = "libavcodec.so.57.107.100"
dependencies = ["avutil"]

[[linux_shared.components]]
component = "avdevice"
soname = "libavdevice.so.57"
filename = "libavdevice.so.57.10.100"
dependencies = ["avformat", "avutil"]

[[linux_shared.components]]
component = "avfilter"
soname = "libavfilter.so.6"
filename = "libavfilter.so.6.107.100"
dependencies = ["swscale", "swresample"]

[[linux_shared.components]]
component = "avformat"
soname = "libavformat.so.57"
filename = "libavformat.so.57.83.100"
dependencies = ["avcodec", "avutil"]

[[linux_shared.components]]
component = "postproc"
soname = "libpostproc.so.54"
filename = "libpostproc.so.54.7.100"
dependencies = ["avcodec", "swscale", "avutil"]

[[linux_shared.components]]
component = "swresample"
soname = "libswresample.so.2"
filename = "libswresample.so.2.9.100"
dependencies = ["avcodec", "swscale", "avutil", "avformat"]

[[linux_shared.components]]
component = "swscale"
soname = "libswscale.so.4"
filename = "libswscale.so.4.8.100"
dependencies = ["avcodec", "avutil", "avformat"]

[windows_shared]
[[windows_shared.components]]
component = "avutil"
impname = "avutil.lib"
filename = "avutil-55.dll"
dependencies = []

[[windows_shared.components]]
component = "avcodec"
impname = "avcodec.lib"
filename = "avcodec-57.dll"
dependencies = ["avutil"]

[[windows_shared.components]]
component = "avdevice"
impname = "avdevice.lib"
filename = "avdevice-57.dll"
dependencies = ["avformat", "avutil"]

[[windows_shared.components]]
component = "avfilter"
impname = "avfilter.lib"
filename = "avfilter-6.dll"
dependencies = ["swscale", "swresample"]

[[windows_shared.components]]
component = "avformat"
impname = "avformat.lib"
filename = "avformat-57.dll"
dependencies = ["avcodec", "avutil"]

[[windows_shared.components]]
component = "postproc"
impname = "postproc.lib"
filename = "postproc-54.dll"
dependencies = ["avcodec", "swscale", "avutil"]

[[windows_shared.components]]
component = "swresample"
impname = "swresample.lib"
filename = "swresample-2.dll"
dependencies = ["avcodec", "swscale", "avutil", "avformat"]

[[windows_shared.components]]
component = "swscale"
impname = "swscale.lib"
filename = "swscale-4.dll"
dependencies = ["avcodec", "avutil", "avformat"]
```

> **Note:**  
> &emsp;&emsp;Note that different components may have different dependencies, so we need to define them in the `dependencies` field.

After compiling and installing, you can see the generated cmake configuration files as follows:

```
lib
└── cmake
    └─── FFmpeg
        ├── FFmpegConfig.cmake
        ├── FFmpegConfigVersion.cmake
        ├── FFmpegModules-release.cmake
        └── FFmpegModules.cmake
```

Finally, you can use it in your cmake project as follows:

```cmake
find_package(FFmpeg REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
    FFmpeg::avutil
    FFmpeg::avcodec
    FFmpeg::avdevice
    FFmpeg::avfilter
    FFmpeg::avformat
    FFmpeg::postproc
    FFmpeg::swresample
    FFmpeg::swscale
)
```

> **Note:**  
> 1. If namespace is not specified, it will be the same as the library name. And the namespace is also the prefix of the config file name.  
>
> 2. The installed libraries files would be removed when there is any wrong in the cmake config file.
