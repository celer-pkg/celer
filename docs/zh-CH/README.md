# Celer概述 [🌍英文](../en-US/README.md)

&emsp;&emsp;Celer 是一款采用 Go 语言编写的轻量级 C/C++ 包管理工具。其命名灵感源自"成为 C/C++ 生态加速器"的愿景。该工具明确定位为 CMake 项目的非侵入式辅助工具，开发者仅需通过 toml 配置文件即可便捷地管理并编译第三方库，充分体现用户友好性。

# Celer诞生的背景

&emsp;&emsp;CMake已成为 C/C++ 项目构建的主流选择，但其核心在于依赖查找，而非依赖管理。像拉取源码、处理库间依赖、配置交叉编译环境这类繁琐工作，仍然需要额外解决。正因如此，我开发了 Celer。它能自动生成 **toolchain_file.cmake**，预先配置好所有构建工具和库路径，并将其与系统库隔离。  
&emsp;&emsp;如图所示，Celer 的设计使其不强依赖于项目开发流程。它仅根据所选平台生成 **toolchain_file.cmake**，此文件便成为了连接项目与构建环境的桥梁。

![workflow](../assets/workflow.svg)

# 主要功能：

| 特性 | 描述 |
| --- | --- |
| **可配置化的交叉编译平台** | 支持C/C++开发的芯片平台很多，而且依赖特定的toolchain和对应的配置，Celer提供了非常友好的配置入口。 |
| **可定制化的项目配置** | 支持管理不同的项目的全局配置，如：依赖库的清单、环境变量、宏定义、甚至全局的CMake变量等，甚至在每个项目维度定制化三方库的编译过程，同时支持托管各个项目自身内的库。|
| **支持常见构建工具的库的托管** | 如：**cmake**、**makefiles**、**meson**、**b2** 等。 |
| **支持生成CMake配置文件** | 对于非CMake作为构建工具的三方库，可以自动生成对应的**cmake config**文件。 |
| **支持精确的编译缓存管理** | Celer 支持通过本地网络上的共享文件夹进行精确的编译产物管理，可以避免重复编译并提高开发效率。此外，Celer 还支持在没有源代码的情况下获取库的编译产物，这对于一些私有库来说非常有用。 |
| **支持DEV 模式** | `toolchain_file.cmake` 可以通过 celer deploy 命令生成，可以直接用于项目开发，之后你可以选择任何 IDE 来开发你的项目。 |
| **支持CI/CD 模式** | 你可以将你的项目配置到 conf/project 中，以便与 CI/CD 集成。|

# 快速开始

这里提供了详细的文档来帮助您使用 Celer：

- [快速开始](./quick_start.md)
- [如何添加新平台](./cmd_create.md#1-创建一个新的平台)
- [如何添加新项目](./cmd_create.md#2-创建一个新的项目)
- [如何添加新端口](./cmd_create.md#3-创建一个新的端口)

高级功能：

- [生成 CMake 配置文件](./advance_generate_cmake_config.md)
- [缓存构建产物](./advance_cache_artifacts.md)

为什么创造Celer: 
- [Celer能解决别的工具不能解决的问题](./why_celer.md)

支持的命令列表：

| 命令                                   | 描述                                  |
| ------------------------------------- | --------------------------------- |
| [autoremove](./cmd_autoremove.md)     | 清理安装目录 - 移除项目不必要的文件。|
| [clean](./cmd_clean.md)               | 移除构建缓存和清理项目的仓库。|
| [configure](./cmd_configfure.md)      | 修改workspace的全局配置。|
| [create](./cmd_create.md)             | 创建平台、项目或端口。 |
| [deploy](./cmd_deploy.md)             | 部署项目。|
| [init](./quick_start.md#3-setup-conf) | 初始化配置仓库。|
| [install](./cmd_install.md)           | 安装一个库。|
| [integrate](./cmd_integrate.md)       | 集成以支持tab补全。|
| [remove](./cmd_remove.md)             | 移除已安装的库库。|
| [search](./cmd_search.md)             | 搜索库库。|
| [tree](./cmd_tree.md)                 | 显示三方库或项目的依赖关系。| 
| [update](./cmd_update.md)             | 更新配置仓库、端口配置仓库或第三方仓库。|
| [version](./cmd_version.md)           | 显示Celer版本信息。 |

# 贡献

&emsp;&emsp;Celer 是一个开源项目，因此它的构建依赖于您的贡献。Celer 由两个部分组成：[celer](https://github.com/celer-pkg/celer.git) 和 [ports](https://github.com/celer-pkg/ports.git)，您可以贡献其中的任意一个。

# 许可证

&emsp;&emsp;Celer 本身的代码采用 MIT 许可证开源，而 ports 仓库中的库则根据其原始作者的条款进行开源。
