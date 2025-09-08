# Celer概述 [🌍英文](../en-US/README.md)

&emsp;&emsp;Celer 是一款采用 Go 语言编写的轻量级 C/C++ 包管理工具。其命名灵感源自"成为 C/C++ 生态加速器"的愿景。该工具明确定位为 CMake 项目的非侵入式辅助工具，开发者仅需通过 toml 配置文件即可便捷地管理并编译第三方库，充分体现用户友好性。

# Celer诞生的背景

&emsp;&emsp;CMake 已成为 C/C++ 项目构建的主流选择，尤其在跨平台编译场景中表现突出。尽管 CMake 在管理构建流程（包括配置、编译和安装）方面表现出色，但其核心功能主要聚焦于依赖项定位（通过 find_package 实现），而非更高层次的包管理任务。在实际开发过程中，许多额外繁琐工作仍超出 CMake 的职责范围，例如： **克隆库源码和配置编译期间需要的工具**，**组织第三方库之间的依赖关系**， **配置交叉编译环境**等等。  
&emsp;&emsp;实际上，Celer 的核心功能在于按需动态生成 **toolchain_file.cmake** 文件。该文件会通过相对路径配置所有必需的构建工具，并指定库搜索路径以隔离系统库的干扰。这意味着在生成 **toolchain_file.cmake** 之前，所有准备工作都已由 Celer 自动完成——这也正是我们重新打造 Celer 而非直接采用其他 C/C++ 包管理工具的核心[**原因**](./why_reinvent_celer.md)之一。

# 主要功能：

Celer目前主要提供以下几个核心功能：

1. **根据选择的编译环境自动下载所需要的工具并完成配置**  
根据当前选择的platform和目标编译的三方库，会自动下载 **toolchain**、**sysroot**、**CMake**、**ninja**、**msys2**、**strawberry-perl** 等，并自动完成配置；

2. **支持常见编译工具编译的三方库的托管**  
在每个三方库版本目录里的**port.toml**文件里可以通过指定**build_system**字段为**cmake**、**makefiles**、**ninja**、**meson**等，从而实现三方库的各种编译工具的编译；

3. **支持生成CMake配置文件**  
对于非CMake作为构建工具的三方库，可以自动生成对应的**cmake config**文件，方便在**CMake**项目中以**find_package**方式集成它们；

4. **支持精确的编译缓存共享**  
通过配置**cache_dir**，可进行在局域网内的共享文件夹进行存储和访问**编译安装后的输出产物**，通过精确地管理编译缓存，避免重复编译以提升开发效率；

5. **支持项目私有库管理**  
在实际项目中，不同项目往往需要使用三方库的不同版本，Celer支持在对应的project配置文件里指定特定版本。有些库不属于公开的三方库，只属于当前项目内部所有，Celer能通过在对应的 project目录里创建并管理它们。

# 快速开始

我们提供了详细的文档来帮助您使用 Celer：

- [快速开始](./quick_start.md)
- [如何添加新平台](./config_add_platform.md)
- [如何添加新项目](./config_add_project.md)
- [如何添加新端口](./config_add_port.md)

高级功能：

- [生成 CMake 配置文件](./config_generate_cmake_config.md)
- [缓存构建产物](./config_cache_management.md)

支持的命令列表：

| 命令                               | 描述                                  |
| ------------------------------------- | --------------------------------- |
| [about](./cmd_about.md)               | 显示 Celer 版本信息。 |
| [autoremove](./cmd_autoremove.md)     | 清理安装目录 - 移除项目不必要的文件。|
| [clean](./cmd_clean.md)               | 移除构建缓存和清理项目的仓库。|
| [configure](./quick_start.md#4-configure-platform-or-project) | 配置平台或项目。|
| [create](./cmd_create.md)             | 创建 [平台](./config_add_platform.md)、[项目](./config_add_project.md) 或 [端口](./config_add_port.md)。 |
| [deploy](./cmd_deploy.md)             | 部署项目。|
| [init](./quick_start.md#3-setup-conf) | 初始化配置仓库。|
| [install](./cmd_install.md)           | 安装一个库。|
| [integrate](./cmd_integrate.md)       | 集成以支持 tab 补全。|
| [remove](./cmd_remove.md)             | 移除已安装的库库。|
| [search](./cmd_search.md)             | 搜索库库。|
| [tree](./cmd_tree.md)                 | 显示端口或项目的依赖关系。| 
| [update](./cmd_update.md)             | 更新配置仓库、端口配置仓库或第三方仓库。|

# 贡献

&emsp;&emsp;Celer 是一个开源项目，因此它的构建依赖于您的贡献。Celer 由两个部分组成：[celer](https://github.com/celer-pkg/celer.git) 和 [ports](https://github.com/celer-pkg/ports.git)，您可以贡献其中的任意一个。

# 许可证

&emsp;&emsp;Celer 本身的代码采用 MIT 许可证开源，而 ports 仓库中的库则根据其原始作者的条款进行开源。
