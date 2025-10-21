# 如何配置生成cmake配置文件

&emsp;&emsp;我们都知道，许多第三方库都不使用cmake来构建，安装后也不会生成cmake配置文件。这使得使用cmake来找到它们变得困难。尽管我们可以使用**pkg-config**来找到它们，但是它只能在Linux上使用。现在，Celer可以为它们生成cmake配置文件，因此它们可以在任何平台上使用。

**1. 如何为单库文件的库生成cmake配置文件**

例如，x264，你应该在端口的版本目录中创建一个cmake_config.toml文件。

```
└── x264
    └── stable  
        ├── cmake_config.toml
        └── port.toml
```

cmake_config.toml文件的内容如下，我们可以为不同的平台定义不同的文件名。

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

当编译并安装后，你可以在lib目录下看到生成的cmake配置文件，如下所示：

```
lib
└── cmake
    └─── x264
        ├── x264Config.cmake
        ├── x264ConfigVersion.cmake
        ├── x264Targets.cmake
        └── x264Targets-release.cmake
```

随后, 你可以在cmake项目中使用它，如下所示：

```cmake
find_package(x264 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE x264::x264)
```

> **Note:**  
> 如果namespace没有在cmake_config里定义，library名将成为namespace的默认值，namespace同时也是cmake config文件的前缀。

**2. 如何为有组件的库生成cmake配置文件**

例如，ffmpeg，你应该在端口的版本目录中创建一个cmake_config.toml文件。

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
> 不同的组件可能有不同的依赖关系，因此我们需要在`dependencies`字段中定义它们。

当编译并安装后，你可以在lib目录下看到生成的cmake配置文件，如下所示：

```
lib
└── cmake
    └─── FFmpeg
        ├── FFmpegConfig.cmake
        ├── FFmpegConfigVersion.cmake
        ├── FFmpegModules-release.cmake
        └── FFmpegModules.cmake
```
随后, 你可以在cmake项目中使用它，如下所示：

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

**3. 如何生成目标为interface类型的cmake配置文件**

例如，prebuilt-ffmpeg，你应该在端口的版本目录中创建一个cmake_config.toml文件。

```
└── prebuilt-ffmpeg
    └── 5.1.6
        ├── cmake_config.toml
        └── port.toml
```

```toml
[package]
ref = "5.1.6"

[[build_configs]]
url = "https://github.com/celer-pkg/test-conf/releases/download/resource/prebuilt-ffmpeg@5.1.6@x86_64-linux.tar.gz"
pattern = "x86_64-linux*"
build_system = "prebuilt"
library_type = "interface"
```
> 为了生成interface类型的cmake配置文件，你需要将`library_type`设置为**interface**。

```toml
namespace = "FFmpeg"

[linux_interface]
libraries = [
    "libavutil.so.57",
    "libavcodec.so.59",
    "libavdevice.so.59",
    "libavfilter.so.8",
    "libavformat.so.59",
    "libpostproc.so.56",
    "libswresample.so.4",
    "libswscale.so.6",
]
```

> 因为是生成interface类型的cmake配置文件，且有很多库文件，只需要将所有需要链接的库都列在**libraries**下即可。

当编译并安装后，你可以在lib目录下看到生成的cmake配置文件，如下所示：

```
lib
└── cmake
    └── FFmpeg
        ├── FFmpegConfig.cmake
        └── FFmpegConfigVersion.cmake
```

随后, 你可以在cmake项目中使用它，如下所示：

```cmake
find_package(FFmpeg REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE FFmpeg::prebuilt-ffmpeg)
```

> **Note:**  
> 1. 如果namespace没有在cmake_config里定义，library名字将成为默认的namespace，namespace同时也是cmake config文件的前缀。  
> 2. 当cmake config文件有错误时，安装的库文件将被删除。
