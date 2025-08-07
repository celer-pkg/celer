# Celer概述 [🌍英文](../en-US/README.md)

&emsp;&emsp;Celer 是一个用 Go 语言开发的非常轻量级的C/C++包管理器。取名 "Celer" 是期望其作为**C/C++**项目开发的加速器。  
Celer 的目标是作为 CMake 的补充，而不是取代它。它让开发者仅通过 toml 文件来管理和编译第三方库，提供一个更加友好的用户体验。

# Celer诞生的背景

&emsp;&emsp;CMake 已经成为编译 C/C++ 项目的主流选择，尤其是在进行交叉编译时。然而，CMake 仅在查找库（通过 find_package）方面发挥作用。在实际的项目开发中，往往还有一些 CMake 不负责的繁琐任务，包括如下：

1. **下载代码和搭建编译环境**: CMake 不会帮你下载库代码，也不会帮你搭建编译环境，甚至不会帮你准备各种编译需要的工具，比如：autoconf, automake, pkg-config, nasm, windows-perl, cmake等等，这些工作往往需要开发自己手动准备和搭建；
2. **管理三方库的编译依赖**: 网上信息非常分散，没有地方进行统一的存档，而且在编译它们期间，而且需要大量的手动工作以组织它们的依赖关系；
3. **配置交叉编译环境**: CMake虽然支持过指定`CMAKE_TOOLCHAIN_FILE`来配置交叉编译环境，但仍需专业的手写脚本配置。

&emsp;&emsp;其实，Celer最核心的功能是根据需要动态生成一个**toolchain_file.cmake**， 并在其中以相对路径配置了所有需要的编译工具，而且还指定了库的搜索路径以隔离系统库被寻找到，意味着在 **toolchain_file.cmake** 生成之前所有的工作都被Celer搞定了，这也是为什么重新发明Celer，而未采用其它的C/C++包管理的[**原因之一**](./00_why_reinvent_celer.md)。

# 主要功能：

Celer目前主要提供以下几个核心功能：

1. **根据选择的编译环境自动下载所需要的工具并完成配置**：  
根据当前选择的platform和目标编译的三方库，会自动下载 `toolchain`、`sysroot`、`CMake`、`ninja`、`msys2`、`strawberry-perl` 等，并自动完成配置；

2. **支持常见编译工具编译的三方库的托管**：  
在每个三方库版本目录里的**port.toml**文件里可以通过指定`build_system`字段为`cmake`、`makefiles`、`ninja`、`meson`等，从而实现三方库的各种编译工具的编译；

3. **支持生成CMake配置文件**:  
对于非CMake作为构建工具的三方库，可以自动生成对应的`cmake config`文件，方便在`CMake`项目中以`find_package`方式集成它们；

4. **支持精确的编译缓存共享**:  
通过配置`cache_dir`，可进行在局域网内的共享文件夹进行存储和访问`编译安装后的输出产物`，通过精确地管理编译缓存，避免重复编译以提升开发效率；

5. **支持项目私有库管理**:  
在实际项目中，不同项目往往需要使用三方库的不同版本，Celer 支持在对应的 project 配置文件里指定特定版本。有些库不属于公开的三方库，只属于当前项目内部所有，Celer 能通过在对应的 project 目录里创建并管理它们。

# 如何编编译

1. 下载`golang sdk`；
2. `go build`，即可编译成功；
3. 或者执行内置的脚本`build.sh`即可编译成功；

# 使用说明

```
./celer help
A pkg-manager for C/C++，it's simply a supplement to CMake.

Usage:
  celer [flags]
  celer [command]

Available Commands:
  about       About celer.
  autoremove  Remove installed package but unreferenced by current project.
  clean       Clean build cache for package or project
  configure   Configure to change platform or project.
  create      Create new [platform|project|port].
  deploy      Deploy with selected platform and project.
  help        Help about any command
  init        Init with conf repo.
  install     Install a package.
  integrate   Integrates celer into [bash|fish|powershell|zsh]
  remove      Uninstall a package.
  update      Update port's repo.
  tree        Show [dev_]dependencies of a port or a project.

Flags:
  -h， --help   help for celer

Use "celer [command] --help" for more information about a command.
```

关于cli模式的使用，请参考以下文章:

1. [Celer是如何工作的](./01_how_it_works.md)
2. [如何初始化Celer](./02_how_to_init.md)
3. [如何管理编译环境的平台](./03_how_to_manager_platform.md)
4. [如何管理多项目配置](./04_how_to_manager_project.md)
5. [如何托管新的三方库](./05_how_to_add_port.md)
8. [如何集成Celer](./06_how_to_integrate.md)
9. [如何安装一个三方库](./07_how_to_install.md)
10. [如何卸载一个三方库](./08_how_to_remove.md)
11. [如何生成cmake配置文件](./09_how_to_generate_cmake_config.md)
12. [如何共享安装的三方库 ](./10_how_to_share_installed_libraries.md)

# 如何参与贡献

1.  Fork 本仓库；
2.  新建 feature_xxx / bugfix_xxx 分支；
3.  提交代码；
4.  新建 Pull Request；