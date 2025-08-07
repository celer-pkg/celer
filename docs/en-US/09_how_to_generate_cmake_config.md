# 如何生成cmake配置文件

我们都知道，很多第三方库都没有使用cmake构建，安装后也不会生成cmake配置文件，这就导致我们不能使用cmake来找到它们。虽然我们可以使用`pkg-config`来找到它们，但是只能在linux下使用。现在，celer可以为它们生成cmake配置文件，这样就可以在任何平台下使用了。

## 1. 没有组件的库如何生成cmake配置文件

例如，x264，你应该在port的版本目录下创建一个cmake_config.toml文件。

```
└── x264
    └── stable  
        ├── cmake_config.toml
        └── port.toml
```

这个文件的内容如下，我们可以为不同的平台定义不同的文件名。

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

当编译和安装后，你可以看到生成的cmake配置文件如下：

```
lib
└── cmake
    └─── x264
        ├── x264Config.cmake
        ├── x264ConfigVersion.cmake
        ├── x264Targets.cmake
        └── x264Targets-release.cmake
```

最终，你可以在你的cmake项目中使用它，如下所示：

```cmake
find_package(x264 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE x264::x264)
```

> 注意，namespace是在cmake_config文件中定义的，如果没有定义，它将与库名相同, namespace也即使config文件名的前缀。

## 2. 有多组件的库如何生成cmake配置文件

例如，ffmpeg，你应该在port的版本目录下创建一个cmake_config.toml文件：

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

>不同的组件可能有不同的依赖项，所以我们需要在`dependencies`字段中定义它们。

当编译和安装后，你可以看到生成的cmake配置文件如下：

```
lib
└── cmake
    └─── FFmpeg
        ├── FFmpegConfig.cmake
        ├── FFmpegConfigVersion.cmake
        ├── FFmpegModules-release.cmake
        └── FfmpegModules.cmake
```

最终，你可以在你的cmake项目中使用它，如下所示：

```cmake
find_package(ffmpeg REQUIRED)
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

> 注意，namespace是在cmake_config文件中定义的，如果没有定义，它将与库名相同， namespace也即使config文件名的前缀。