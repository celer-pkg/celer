# Celer

&emsp;&emsp;celer是一个用 **Go语言** 实现的 **C/C++ 包管理器**，它对普通开发者友好，不需要掌握额外的脚本语言，它将常见的各类编译工具交叉编译配置的细节进行了高度抽象，只需简单的 **TOML** 即可轻松管理和编译三方库。该包管理器只是 **CMake** 的补充，不颠覆**CMake**， 主要用于方便地处理三方库的编译以及交叉编译， 同时支持管理库与库之间的依赖以及工程项目内私有库的管理。

## Celer诞生的背景

&emsp;&emsp;CMake长期以来仅提供了 **find_package**、**find_program**、**find_library**、**find_path** 等功能，但缺乏对包的管理能力，特别是在以下几个方面：

1. 编译所需要的工具获取和环境配置，如toolchain， rootfs， cmake，nasm等需要手动安装和配置环境变量，而且很容易把系统环境搞污染了；
2. 三方库之间的依赖，网上信息非常分散，没有地方进行统一的存档，而且在编译它们期间，而且需要大量的手动工作以组织它们的依赖关系路径；
3. 交叉编译支持方面，CMake允许通过指定**CMAKE_TOOLCHAIN_FILE**来配置交叉编译环境，但仍需手动配置。

>所以，celer最核心的功能是根据项目的需要动态生成一个**toolchain_file.cmake**， 并在其中以相对路径配置了toolchain，rootfs，cmake， nasm等工具，并且指定了库的搜索路径，意味着在**toolchain_file.cmake**生成之前所有的工作都被celer搞定了。

## 主要功能：

celer目前主要提供以下几个核心功能：

1. **支持自动下载、修复编译所需要的工具**：  
根据当前的编译目标，自动下载 `toolchain`、`sysroot`、`CMake`、`ninja`、`msys2`、`perl` 等以及配置其环境变量；

2. **支持常见编译工具编译的三方库的托管**：  
    在每个三方库版本目录里的port.toml文件里可以通过指定`build_system`字段为`cmake`、`makefiles`、`ninja`、`meson`等，从而实现对非CMake编译的三方库的集成。

3. **支持生成CMake配置文件**:  
对于非CMake作为构建工具的三方库，可以自动生成对应的`cmake config`文件，方便在CMake项目中以**find_package**方式集成库；

4. **支持内部版本冲突检查**:  
内部版本冲突检查，即检查当前workspace下的三方库是否存在多个版本，若存在多个版本，会提示用户选择一个版本；

5. **支持编译缓存共享**:  
通过配置`cache_dir`，可进行局域网内网盘来托管和读取`编译安装后的输出产物`；

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

1. [celer是如何工作的](./docs/01_how_it_works.md)
2. [如何初始化celer](./docs/02_how_to_init.md)
3. [如何管理编译环境的平台](./docs/03_how_to_manager_platform.md)
4. [如何管理多项目配置](./docs/04_how_to_manager_project.md)
5. [如何托管新的三方库](./docs/05_how_to_add_port.md)
8. [如何集成celer](./docs/06_how_to_integrate.md)
9. [如何安装一个三方库](./docs/07_how_to_install.md)
10. [如何卸载一个三方库](./docs/08_how_to_remove.md)
11. [如何生成cmake配置文件](./docs/09_how_to_generate_cmake_config.md)
12. [如何共享安装的三方库 ](./docs/10_how_to_share_installed_libraries.md)

## 为什么重新发明celer

&emsp;&emsp;在C/C++包管理领域有几个比较知名的工具，如 Conan 和 Vcpkg，还有国内的XMake 等，它们都提供了相对成熟的包管理功能，而且现在都相当有规模了，重新发明celer的原因如下：

1. 托管三方库上手不太友好

&emsp;&emsp;用它们编译已经托管的三方库还是很方便的，但普通开发者想给它们贡献包不是那么友好，需要学习并掌握工具提供的一系列API，甚至有些还引入了新的脚本语言，最后交给用户去组织每个库的configure、build以及install过程，看起来非常自由，且主动权都在用户手中，实则用户们往往没有足够的耐心去学习这些API。  
&emsp;&emsp;celer选择了另外一条路线：将各种编译工具的手动操作都抽象封装起来了，在每个库的配置文件中，只需要指定buildsystem是cmake、makefiles、meson、ninja等其中一个即可，大量的细节用户不再关心，唯一关心的是每个库的编译选项，以及它的依赖关系，这对于普通开发者来说是非常友好的。

2. 多平台和多项目定制化支持较弱

&emsp;&emsp;非常多的三方库的编译选项很多，很多时候需要根据不同的项目开启不同的编译选项，以开启最多的项目为准？少数服从多数? 可有时候不同的feature是互斥的咋办，而且开启的开关越多，往往意味着依赖了更多别的库， 会使得最终的包体积膨胀的厉害，再者就是大的项目往往会有一些私有基础库，这些库如何托管。  
&emsp;&emsp;在celer里允许根据不同的project重载依赖库的编译选项等一切配置，也可以针对特定project管理这个project所需要的私有库。因此，celer既可以管理纯三方库，又可以管理不同project的私有库。

3. 对平台多子工程的项目不友好

&emsp;&emsp;一些平台项目有多个子工程，它们往往是公用一套三方库以及公司内部开发的私有库，正常情况下我们需要确保这些依赖版本完全一致，常规的包管理器的库依赖清单都是定义在各个工程项目内部的， 如果子工程多，依赖的库也多，在迭代开发中很容易出现它们之间的不一致，重度依赖阶段地进行人为核对检查。  
&emsp;&emsp;在celer中，允许给每个项目定义一个project配置文件，在此配置文件里定义多个子工程依赖的库的并集清单，最终celer会生成这个project专属的`toolchain_file.cmake`，然后所有的子工程开发期间都用`CMAKE_TOOLCHAIN_FILE`指向它就可以确保所有的子工程编译环境高度一致。

4. 精确的库缓存管理麻烦

&emsp;&emsp;C/C++的项目编译慢是众所周知，每次编译项目时候连同三方库源码一起编译不现实。将三方库一次性预编译好并固定存储， 固然好， 但如果项目平台多，又或者新项目刚开始三方库的依赖在不确定性中，导致需要频繁替换更新缓存。因为开发期间，三方库一般是按bin、include、lib、share聚集在一起的，这样方便三方库的寻找和链接，所以手动替换三方库的编译产物会是个高风险的事情，非常容易出错。  
&emsp;&emsp;celer采取了一种精确的缓存隔离设计，缓存的key是一个hash，hash的值来自当前库的编译环境、编译选项、子依赖、子依赖编译选项等等信息的汇总，但凡任何一个环节有参数变动则重新编译，且以新计算的出来的hash作为key。如此，才能避免同样的库重复编译，减少编译时间在三方库上的浪费。

5. 内部依赖冲突检查弱

&emsp;&emsp;当依赖的库树层级比较深的时候，很容易遇到公共依赖的库版本不一致的情况，不容易发现， 且人工排查非常麻烦；  
&emsp;&emsp;正因为celer采取的是源码即时编译的方式，所有的三方库几乎都是通过清单管理的，因此celer在编译三方库的时候会自动检查依赖的版本是否一致，不一致则明确告知。

6. 和外部公司合作开发不友好 

&emsp;&emsp;当公司主导开发一个项目，合作公司参与开发，主导公司应该提供一切交叉编译环境和已经编译好的三方库，整个开发环境应该是开箱即用，而不是需要用户手动安装和配置环境变量。  
&emsp;&emsp;如上面**对平台多子工程的项目不友好**提到的，celer会自动生成一个`toolchain_file.cmake`，这个文件会内会以相对路径找所有编译工具以及文件系统等，因此用户只需要在工程里将`CMAKE_TOOLCHAIN_FILE`指向此`toolchain_file.cmake`的路径即可。

## 如何参与贡献

1.  Fork 本仓库；
2.  新建 feature_xxx 分支；
3.  提交代码；
4.  新建 Pull Request；