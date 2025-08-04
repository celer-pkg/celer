# Celer

&emsp;&emsp;Celer是一个用Go语言实现的C/C++ 包管理器，Celer的取名的愿景是 **C/C++'s accelerator**, Celer的目标只是作为 **CMake** 的补充，不颠覆**CMake**。它对普通开发者友好，不需要掌握额外的脚本语言，它将常见的各种编译工具交叉编译配置的细节进行了高度抽象，只需简单的TOML即可轻松管理和编译三方库。

## Celer诞生的背景

&emsp;&emsp;CMake长期以来仅提供了 **find_package**、**find_program**、**find_library**、**find_path** 等功能，但缺乏对包的管理能力，特别是在以下几个方面：

1. 编译所需要的工具获取和环境配置，如：toolchain、rootfs、cmake、nasm、autoconf、automake、libtool、pkg-config等需要手动安装和配置环境变量，而且很容易把系统环境搞污染了；
2. 三方库之间的依赖，网上信息非常分散，没有地方进行统一的存档，而且在编译它们期间，而且需要大量的手动工作以组织它们的依赖关系路径；
3. 交叉编译支持方面，CMake允许通过指定`CMAKE_TOOLCHAIN_FILE`来配置交叉编译环境，但仍需大量手动配置。

>所以，Celer最核心的功能是根据需要动态生成一个**toolchain_file.cmake**， 并在其中以相对路径配置了toolchain，rootfs，cmake， nasm等工具，并且指定了库的搜索路径，意味着在**toolchain_file.cmake**生成之前所有的工作都被Celer搞定了。

## 主要功能：

Celer目前主要提供以下几个核心功能：

1. **支持自动下载并配置编译所需要的工具**：  
根据当前的编译目标，根据目标编译平台所需要自动下载 `toolchain`、`sysroot`、`CMake`、`ninja`、`msys2`、`strawberry-perl` 等，并自动配置；

2. **支持常见编译工具编译的三方库的托管**：  
在每个三方库版本目录里的**port.toml**文件里可以通过指定`build_system`字段为`cmake`、`makefiles`、`ninja`、`meson`等，从而实现三方库的各种编译工具的编译；

3. **支持生成CMake配置文件**:  
对于非CMake作为构建工具的三方库，可以自动生成对应的`cmake config`文件，方便在`CMake`项目中以`find_package`方式集成它们；

4. **支持编译缓存共享**:  
通过配置`cache_dir`，可进行在局域网内的共享文件夹进行存储和访问`编译安装后的输出产物`，通过精确地管理编译缓存，避免重复编译；

5. **支持三方库和项目私有库的隔离管理**:  
通常三方库有多个版本，在实际项目中，往往需要使用不同的版本，这时候就需要在指定的项目里指定特定版本，甚至有些库不属于公开的三方库，只属于当前项目内部所有，Celer能通过目录清晰的界定并管理它们。

## 如何编编译

1. 下载`golang sdk`；
2. `go build`，即可编译成功；
3. 或者执行内置的脚本`build.sh`即可编译成功；

## 使用说明

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

1. [Celer是如何工作的](./docs/01_how_it_works.md)
2. [如何初始化Celer](./docs/02_how_to_init.md)
3. [如何管理编译环境的平台](./docs/03_how_to_manager_platform.md)
4. [如何管理多项目配置](./docs/04_how_to_manager_project.md)
5. [如何托管新的三方库](./docs/05_how_to_add_port.md)
8. [如何集成Celer](./docs/06_how_to_integrate.md)
9. [如何安装一个三方库](./docs/07_how_to_install.md)
10. [如何卸载一个三方库](./docs/08_how_to_remove.md)
11. [如何生成cmake配置文件](./docs/09_how_to_generate_cmake_config.md)
12. [如何共享安装的三方库 ](./docs/10_how_to_share_installed_libraries.md)

## 为什么重新发明Celer

&emsp;&emsp;在C/C++包管理领域有几个比较知名的工具，如 Conan 和 Vcpkg，还有国内的XMake 等，它们都提供了相对成熟的包管理功能，而且现在都相当有规模了，重新发明Celer的原因如下：

1. 托管三方库上手不太友好

&emsp;&emsp;托管平台的三方库使用方便，但贡献复杂——开发者需学习专用API/脚本语言，手动处理编译流程（configure/build/install），学习成本高。   
&emsp;&emsp;Celer通过抽象编译工具，配置文件只需声明构建系统（如cmake/makefiles/meson等），隐藏底层细节。聚焦核心配置后开发者仅需关注编译选项和依赖关系，大幅降低上手门槛。

2. 多平台和多项目定制化支持较弱

&emsp;&emsp;三方库通常提供大量可选特性，不同项目可能需要不同的配置组合，甚至存在互斥选项。开启过多选项会导致依赖链扩张，增加最终构建体积，企业级项目常依赖内部基础库，传统方案难以统一管理。  
&emsp;&emsp;Celer支持项目维度的库版本管理，避免全局配置冲突，同时支持在项目维度的目录里托管私有库。

3. 对平台且多子工程的项目不友好  

&emsp;&emsp;平台型项目包含多个子工程时，传统方案需在每个子工程单独维护依赖清单，导致版本分散管理，难以确保统一，依赖库数量多时，人工核对成本高，迭代过程中易出现隐式版本漂移；  
&emsp;&emsp;Celer是集中式依赖控制：在项目根目录定义project配置文件，统一声明所有子工程依赖的库并集，自动生成全局**toolchain_file.cmake**，固化编译环境；子工程通过`CMAKE_TOOLCHAIN_FILE`指向统一工具链文件，编译时自动继承预定义的依赖版本和配置，消除人工干预；如此，既保持子工程独立性，又实现依赖的集中管控，支持动态更新，修改根配置后，所有子工程自动同步。

4. 缺乏精确的缓存管理

&emsp;&emsp;C/C++ 项目编译慢，主要原因在于反复编译相同三方库。传统预编译方案（如集中存放 lib/include）在多平台或依赖频繁变动时难以维护，手动替换易出错。  
&emsp;&emsp;Celer以Hash值作为缓存Key，Hash由编译环境、选项、依赖链等综合计算生成。
变动即重编译：任何参数（如编译器版本、依赖选项）变化都会触发新 Hash，确保相同库不重复编译。最终显著减少三方库的重复编译时间，同时避免人工管理风险。

5. 内部依赖冲突检查弱

&emsp;&emsp;当依赖的库树层级比较深的时候，很容易遇到公共依赖的库版本不一致的情况，不容易发现， 且人工排查非常麻烦；  
&emsp;&emsp;正因为Celer采取的是源码即时编译的方式，所有的三方库几乎都是通过清单管理的，因此Celer在编译三方库的时候会自动检查依赖的版本是否一致，不一致则明确告知。

6. 和外部公司合作开发不友好 

&emsp;&emsp;当公司主导开发一个项目，合作公司参与开发，主导公司应该提供一切交叉编译环境和已经编译好的三方库，整个开发环境应该是开箱即用，而不是需要用户手动安装和配置环境变量。  
&emsp;&emsp;如上面**对平台多子工程的项目不友好**提到的，Celer会自动生成一个`toolchain_file.cmake`，这个文件会内会以相对路径找所有编译工具以及文件系统等，因此用户只需要在工程里将`CMAKE_TOOLCHAIN_FILE`指向此`toolchain_file.cmake`的路径即可。

## 如何参与贡献

1.  Fork 本仓库；
2.  新建 feature_xxx 分支；
3.  提交代码；
4.  新建 Pull Request；