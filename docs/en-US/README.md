# Celer overview [🌍中文](../zh-CH/README.md)

&emsp;&emsp;Celer is a very lightweight C/C++ package manager written in Go. The name "Celer" is inspired by the vision of being C/C++'s accelerator. Celer is explicitly positioned as a non-intrusive CMake assistant for any CMake projects. It is designed to be user-friendly for developers to manager and compile third-party libraries with toml files only.

# The background of Celer

&emsp;&emsp;CMake has been the mainstream choice for compiling C/C++ projects, especially when cross-compiling. However, CMake plays the role of finding libraries (via find_package) only. In actual project development, there are often additional tedious tasks CMake isn't responsible, for examples as below:

**1. Clone repos and setup build tools**  
CMake doesn't download source code of libraries, setup toolchains, rootfs, and tools that required for compilation, such as autoconf, automake, pkg-config, nasm, windows-perl, cmake, etc., all of that need to be prepared and configured manually by developers themself.

**2. Dependencies between third-party libraries**  
Information is scattered across the internet, with no centralized place for archiving. During the compilation process, a lot of manual work is required to organize the dependencies.

**3. Setup cross-compilation environment**  
Although CMake supports cross-compilation by providing `-D CMAKE_TOOLCHAIN_FILE` to specify a **toolchain_file.cmake**, it still requires manually written scripts for proper configuration.

>In fact, the core functionality of Celer is to dynamically generate a **toolchain_file.cmake** as required. Within this file, it configures all required build tools with relative paths, and also specifies the library search paths to isolate system libraries from being found. This means that all the work required  is handled by Celer before generating the toolchain_file.cmake, this is [**one of the reasons**](./why_reinvent_celer.md) why Celer was reinvented, rather than using other C/C++ package managers.

# Key features

Celer now has below key features:

**1. Automatically downloads and configures build tools**  
It automatically downloads and configures tools like toolchain, sysroot, CMake, ninja, msys2, and strawberry-perl based on the selected platform and target libraries.

**2. Supports hosting of libraries compiled with common build tools**  
In the port.toml file of each third-party library version directory, you can specify the `build_system` field as **cmake**, **makefiles**, **ninja**, **meson**, etc., to compile with different build tools.

**3. Supports generating cmake configs**  
Celer can generate cmake configs for any libraries, especially for libraries that not build by CMake, then you can `find_package` it easily.

**4. Support cache and share build artifacts**  
Celer supports precise build artifact management. Currently, you can configure the `cache_dir` in **celer.toml** to store and access artifacts in a shared folder on the local network. This aims to avoid redundant compilation of source code and improve development efficiency.

**5. Supports overriding compile options for third-party libraries and managing project-specific libraries**  
Celer supports overriding third-party libraries with different versions and compile options within individual project folders. It also allows adding project-specific internal libraries within the project's folder.

# Get started

We have documentations to guide you in using Celer:

- [Quick start Celer.](./quick_start.md)
- [Add a new platform.](./config_add_platform.md)
- [Add a new project.](./config_add_project.md)
- [Add a new port.](./config_add_port.md)

Advanced features:

- [Generate cmake configs.](./config_generate_cmake_config.md)
- [Cache build artifacts.](./config_cache_management.md)

Supported commands:

| Command                               | Description                                                                   |
| ------------------------------------- | ----------------------------------------------------------------------------- |
| [about](./cmd_about.md)               | About Celer.                                                                  |
| [autoremove](./cmd_autoremove.md)     | Remove installed packages of current project.                                 |
| [clean](./cmd_clean.md)               | Clean build cache for package or project.                                     |
| [configure](./quick_start.md#4-configure-platform-or-project) | Configure platform or project.                        |
| create                                | Create [a platform](./config_add_platform.md), [a project](./config_add_project.md) or [a port](./config_add_port.md). |
| [deploy](./cmd_deploy.md)             | Deploy with selected platform and project.                                    |
| [setup](./quick_start.md#3-setup-conf)| Setup Celer with conf repo.                                                   |
| [install](./cmd_install.md)           | Install a package.                                                            |
| [integrate](./cmd_integrate.md)       | Integrate to support tab completion.                                          |
| [remove](./cmd_remove.md)             | Remove a package.                                                             |
| [tree](./cmd_tree.md)                 | Show the dependency tree of a port or a project.                              |
| [update](./cmd_update.md)             | Update conf repo, ports repo and port source.                                 |

# Contribute

&emsp;&emsp;Celer is an open source project, and is thus built with your contributions. Celer is consist of two parts: [Celer](https://github.com/celer-pkg/celer.git) and [conf](https://github.com/celer-pkg/ports.git) you can contribute any of them.

# License

&emsp;&emsp;The code in this repository is licensed under the MIT License. The libraries provided by ports are licensed under the terms of their original authors.