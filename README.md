# Celer overview [🌍中文](./docs/zh-CH/README.md)

&emsp;&emsp;Celer is a very lightweight C/C++ package manager written in Go. The name "Celer" is inspired by the vision of being C/C++'s accelerator. The goal of Celer is to serve as a supplement to CMake, and not to replace it. It is designed to be user-friendly for developers to manager and compile third-party libraries with toml files only.

# The background of Celer

&emsp;&emsp;CMake has been the mainstream choice for compiling C/C++ projects, especially when cross-compiling. However, CMake plays the role of finding libraries (via find_package) only. In actual project development, there are often additional tedious tasks CMake isn't  responsible, including:

1. **Clone repos and setup build tools**: CMake doesn't download source code of libraries, setup toolchains, rootfs, and tools that required for compilation, such as autoconf, automake, pkg-config, nasm, windows-perl, cmake, etc., all of that need to be prepared and configured manually by developers themself.
2. **Dependencies between third-party libraries**: Information is scattered across the internet, with no centralized place for archiving. During the compilation process, a lot of manual work is required to organize the dependencies.
3. **Setup cross-compilation environment**: Although CMake supports configuring a cross-compilation environment via `CMAKE_TOOLCHAIN_FILE`, it still requires manually written scripts for proper configuration.

&emsp;&emsp;In fact, the core functionality of Celer is to dynamically generate a **toolchain_file.cmake** as required. Within this file, it configures all required build tools with relative paths, and also specifies the library search paths to isolate system libraries from being found. This means that all the work required  is handled by Celer before generating the toolchain_file.cmake, this is [**one of the reasons**](./docs/en-US/00_why_reinvent_celer.md) why Celer was reinvented, rather than using other C/C++ package managers.

# Key features

Celer now has below core features:

1. **Automatically downloads and configures build tools**：  
It automatically downloads and configures tools like toolchain, sysroot, CMake, ninja, msys2, and strawberry-perl based on the selected platform and target libraries.

2. **Supports hosting of libraries compiled with common build tools**：  
In the port.toml file of each third-party library version directory, you can specify the `build_system` field as **cmake**, **makefiles**, **ninja**, **meson**, etc., to compile with different build tools.

3. **Supports generating cmake configs**:  
Celer can genereate cmake configs for any libraries, espically for libraries that not build by CMake, then you can `find_package` it easily.

4. **Support cache and share build artifacts**:  
Celer supports precise build artifact management. Currently, you can configure the `cache_dir` in **celer.toml** to store and access artifacts in a shared folder on the local network. This aims to avoid redundant compilation of source code and improve development efficiency.

5. **Supports overriding compile options for third-party libraries and managing project-specific libraries**:  
Celer supports overriding third-party libraries with different versions and compile options within individual project folders. It also allows adding project-specific internal libraries within the project's folder.

# How to build Celer

1. Install the Go SDK by referring https://go.dev/doc/install.
2. git clone https://github.com/celer-pkg/celer.git.
3. cd celer && go build.

# Get started

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

We have documentations to guide you in using Celer as below:

1. [How Celer works](./docs/en-US/01_how_it_works.md)
2. [How to init Celer](./docs/en-US/02_how_to_init.md)
3. [How to manager platform](./docs/en-US/03_how_to_manager_platform.md)
4. [How to manager projects](./docs/en-US/04_how_to_manager_project.md)
5. [How to add a new port](./docs/en-US/05_how_to_add_port.md)
8. [How to support tab completion](./docs/en-US/06_how_to_integrate.md)
9. [How to install a third-party library](./docs/en-US/07_how_to_install.md)
10. [How to uninstall a package](./docs/en-US/08_how_to_remove.md)
11. [How to generate cmake configs](./docs/en-US/09_how_to_generate_cmake_config.md)
12. [How to share build artifacts](./docs/en-US/10_how_to_share_installed_libraries.md)

# How to contribute

1.  Fork this repo: https://github.com/celer-pkg/celer.git.
2.  create branch like: feature_xxx or bugfix_xxx.
3.  Submit code to your branch.
4.  Create Pull Request.
