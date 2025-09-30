# Celer概述 [🌍英文](../en-US/README.md)

&emsp;&emsp;Celer 是一款采用 Go 语言编写的轻量级 C/C++ 包管理工具。其命名灵感源自"成为 C/C++ 生态加速器"的愿景。该工具明确定位为 CMake 项目的非侵入式辅助工具，开发者仅需通过 toml 配置文件即可便捷地管理并编译第三方库，充分体现用户友好性。

# Celer诞生的背景

&emsp;&emsp;CMake 已成为 C/C++ 项目构建的主流选择，尤其在跨平台编译场景中表现突出。尽管 CMake 在管理构建流程（包括配置、编译和安装）方面表现出色，但其核心功能主要聚焦于依赖项定位（通过 find_package 实现），而非更高层次的包管理任务。在实际开发过程中，许多额外繁琐工作仍超出 CMake 的职责范围，例如： **克隆库源码和配置编译期间需要的工具**，**组织第三方库之间的依赖关系**， **配置交叉编译环境**等等。  
&emsp;&emsp;实际上，Celer 的核心功能在于按需动态生成 **toolchain_file.cmake** 文件。该文件会通过相对路径配置所有必需的构建工具，并指定库搜索路径以隔离系统库的干扰。这意味着在生成 **toolchain_file.cmake** 之前，所有准备工作都已由 Celer 自动完成——这也正是重新打造 Celer 而非直接采用其他 C/C++ 包管理工具的核心[**原因**](./why_reinvent_celer.md)之一。

# 主要功能：

| 特性 | 描述 |
| --- | --- |
| **自动下载所需要的工具并完成配置** | 根据当前选择的platform和目标编译的三方库自动下载编译工具并自动完成配置。 |
| **支持常见编译工具编译的三方库的托管** | 支持构建和托管基于 **cmake**、**makefiles**、**meson** 等的库。 |
| **支持生成CMake配置文件** | 对于非CMake作为构建工具的三方库，可以自动生成对应的**cmake config**文件。 |
| **支持精确的编译缓存管理** | Celer 支持通过本地网络上的共享文件夹进行精确的构建工件管理。这可以避免重复编译并提高开发效率。此外，Celer 还支持在没有源代码的情况下获取库的构建工件。这对于一些私有库来说非常有用。 |
| **支持多项目管理以及针对项目的三方库定制编译** | Celer 支持多项目管理，允许根据项目的需要定制第三方库，包括版本和编译选项。它还支持在每个项目的文件夹内管理内部库。 |
| **支持快速部署** | Celer 支持通过单个命令快速部署，Celer 会自动处理所有依赖项的编译和安装。 |
| **支持DEV 模式** | `toolchain_file.cmake` 可以通过 celer deploy 命令生成，可以直接用于项目开发，之后你可以选择任何 IDE 来开发你的项目。 |
| **支持CI/CD 模式** | 你可以将你的项目配置到 conf/project 中，以便与 CI/CD 集成。|

# 快速开始

这里提供了详细的文档来帮助您使用 Celer：

- [快速开始](./quick_start.md)
- [如何添加新平台](./cmd_create.md#1-创建一个新的平台)
- [如何添加新项目](./cmd_create.md#2-创建一个新的项目)
- [如何添加新端口](./cmd_create.md#3-创建一个新的端口)

高级功能：

- [生成 CMake 配置文件](./introduce_generate_cmake_config.md)
- [缓存构建产物](./introduce_cache_artifacts.md)

支持的命令列表：

| 命令                               | 描述                                  |
| ------------------------------------- | --------------------------------- |
| [about](./cmd_about.md)               | 显示Celer版本信息。 |
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

# 贡献

&emsp;&emsp;Celer 是一个开源项目，因此它的构建依赖于您的贡献。Celer 由两个部分组成：[celer](https://github.com/celer-pkg/celer.git) 和 [ports](https://github.com/celer-pkg/ports.git)，您可以贡献其中的任意一个。

# 许可证

&emsp;&emsp;Celer 本身的代码采用 MIT 许可证开源，而 ports 仓库中的库则根据其原始作者的条款进行开源。
