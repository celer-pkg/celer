# 快速上手

## 1. 克隆Clone the repository

第一步是从GitHub克隆Celer仓库，这是Celer项目的所有源代码。  

要执行此操作，请运行以下命令：

```shell
git clone https://github.com/celer-pkg/celer.git
```

## 2. 编译 Celer

  - 安装Go SDK，参考https://go.dev/doc/install。
  - 进入Celer目录，执行`go build`。

  **Tips:**  
  在中国，你可能需要为Go设置代理，如下所示：

  ```shell
  export GOPROXY=https://goproxy.cn
  ```

>**Note:** 目前已经发布稳定版本，用户可以直接下载预构建的二进制文件，跳过前两步。

## 3. 配置 conf

不同的C++项目通常需要不同的构建环境和依赖项。Celer建议使用**conf**来定义每个项目的跨编译环境和第三方依赖项。

**conf**的结构如下：

```
conf
├── buildtools
│   ├── x86_64-linux.toml
│   └── x86_64-windows.toml
├── platforms
│   ├── aarch64-linux-gnu-gcc-9.2.toml
│   ├── x86_64-linux-ubuntu-20.04.toml
│   ├── x86_64-linux-ubuntu-22.04.toml
│   └── x86_64-windows-msvc-14.44.toml
└── projects
    ├── test_project_01 --------------- project_01's dependencies
    │   ├── boost --------------------- override public build options
    │   │   └── 1.87.0
    │   │       └── port.toml
    │   └── sqlite3 ------------------- override public build options
    │       └── 3.49.0
    │           └── port.toml
    ├── test_project_01.toml ---------- project_01
    ├── test_project_02 --------------- project_02's dependencies
    │   ├── ffmpeg -------------------- override public build options
    │   │   └── 5.1.6
    │   │       └── port.toml
    │   ├── lib_001 ------------------- second project's private library
    │   │   └── port.toml
    │   └── lib_002 ------------------- project_02's private library
    │       └── port.toml
    └── test_project_02.toml ---------- project_02
```

>关于如何创建新的**platform**、**project**和**port**，你可以参考：[**add platform**](./cmd_create.md#1-创建一个新的平台)、[**add project**](./cmd_create.md#2-创建一个新的项目)和[**add port**](./cmd_create.md#3-创建一个新的端口)。

以下是conf文件和它们的描述：

| 文件                   | 描述 |
| --------------------- | ---------- |
| buildtools/*.toml     | 定义一些三方库在编译期间需要的额外工具 |
| platforms/*.toml      | 定义平台，包括工具链和根文件系统。 |
| projects/*.toml       | 定义项目，包括依赖项、CMake变量、C++宏和构建选项。|
| projects/*/port.toml  | 用于覆盖项目特定的第三方库版本、自定义构建参数和定义项目的私有库。 |

>**Note:**  
&emsp;&emsp;虽然**conf**是Celer推荐的配置方式，但是Celer也可以在没有**conf**的情况下工作。在这种情况下，我们只能使用Celer来构建本地工具链的第三方库：
>
>- 在Windows中，Celer通过**vswhere**定位已安装的Visual Studio作为默认工具链。对于基于makefile的库，它会自动下载并配置MSYS2。
>
>- 在Linux中，Celer会自动使用本地安装的x86_64 gcc/g++工具链。

要配置conf，请运行以下命令：

```shell
celer init --url=https://github.com/celer-pkg/test-conf.git
```

**🚩如果你在中国，或许你需要给celer配置代理以便于访问github等资源: 🚩**

```shell
celer configure --proxy-host 127.0.0.1 --proxy-port 7890
```

Then the **celer.toml** file will be generated in the workspace directory:

```toml
[global]
  conf_repo = "https://github.com/celer-pkg/test-conf.git"
  platform = ""
  project = ""
  jobs = 16
  build_type = "release"
  offline = false
  verbose = false

[proxy]
  host = "127.0.0.1"
  port = 7890
```

>**Tips:**  
>  **https://github.com/celer-pkg/test-conf.git** 只是一个测试用的conf仓库，你可以使用它来体验Celer，也可以根据它创建你自己的conf仓库作为参考。

## 4. 配置平台或项目

**platform** 和 **project** 是两个组合，它们可以自由组合。例如，尽管目标环境是 **aarch64-linux**，但你可以选择在 **x86_64-linux** 平台上编译/开发/调试。

```shell
celer configure --platform=x86_64-linux-ubuntu-22.04
celer configure --project=test_project_02
```

经过配置后，**celer.toml** 文件会更新为以下内容：

```toml
[global]
  conf_repo = "https://github.com/celer-pkg/test-conf.git"
  platform = "aarch64-linux-gnu-gcc-9.2"
  project = "test_project_02"
  jobs = 16
  build_type = "release"
  offline = false
  verbose = false

[cache_dir]
  dir = "/home/phil/celer_cache"
```

以下是字段以及它们的描述：

| 字段 | 描述 |
| ----- | ----------- |
| conf_repo |  用于保存平台和项目配置的仓库URL |
| platform |  当前工作空间所选的平台，当为空时，Celer会使用检测本地工具链来编译您的库和项目。 |
| project |  当前工作空间所选的项目，当为空时，会创建一个名为“unname”的项目。 |
| jobs |  Celer编译时使用的最大CPU核心数，默认值为您CPU的核心数。 |
| build_type | 默认值为 **release**，您也可以将其设置为 **debug**。 |
| offline | 在offline模式下，celer将不再尝试更新仓库和下载资源。|
| verbose | 在verbose模式下，celer将生成更详细的编译日志。|
| cache_dir | Celer支持缓存构建工件，这可以避免重复编译。[你可以将其配置为本地目录或LAN中的共享文件夹](./advance_cache_artifacts.md)。 |

## 5. 部署 Celer

部署 Celer 是在所选平台的构建环境中构建项目所需的第三方库，要部署 Celer，请运行：

```shell
celer deploy
```

&emsp;&emsp;成功部署 Celer 后，会在工作空间目录下生成 **toolchain_file.cmake** 文件，该文件允许项目仅依赖于此文件，从而无需后续再使用 Celer。此外，您可以将工作空间打包为 **installed folder**、**downloaded folder** 和 **toolchain_file.cmake** 三个文件夹，这可以是构建环境的基础，供其他用户使用。  

## 6. 构建您的 CMake 项目

使用 Celer 生成的 **toolchain_file.cmake** 文件，构建您的 CMake 项目将变得非常简单，如下所示：

```shell
# 方式1: 
set(CMAKE_TOOLCHAIN_FILE "/xx/workspace/toolchain_file.cmake")  

# 方式2: 
cmake .. -DCMAKE_TOOLCHAIN_FILE="/xx/workspace/toolchain_file.cmake"
```
