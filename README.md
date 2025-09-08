# Celer overview [🌍中文](./docs/zh-CH/README.md)

&emsp;&emsp;Celer is a very lightweight C/C++ package manager written in Go. The name "Celer" is inspired by the vision of being C/C++'s accelerator. Celer is explicitly positioned as a non-intrusive CMake assistant for any CMake projects. It is designed to make it easy for developers to manage and compile third-party libraries with only TOML files.

# The background of Celer

&emsp;&emsp;CMake has become the mainstream build system for compiling C/C++ projects, particularly for cross-compiling. While CMake excels at managing the build process—including configuration, compilation, and installation—it primarily focuses on locating dependencies (via **find_package**) rather than handling higher-level package management tasks. In real-world development, many additional tedious tasks fall outside CMake's responsibilities, such as: **clone repos and setup build tools**, **organize dependencies between third-party libraries**, **setup cross-compilation environment**, etc.  
&emsp;&emsp;In fact, the core functionality of Celer is to dynamically generate a **toolchain_file.cmake** as required. Within this file, it configures all required build tools with relative paths, and also specifies the library search paths to isolate system libraries from being found. This means that all the work required  is handled by Celer before generating the toolchain_file.cmake, this is one of the [**REASONS**](./docs/en-US/why_reinvent_celer.md) why Celer was reinvented, rather than using other C/C++ package managers.

# Key features

Celer now has below key features:

**1. Automatically downloads and configures build tools**  
It automatically downloads and configures tools like toolchain, sysroot, CMake, ninja, msys2, and strawberry-perl based on the selected platform and target libraries.

**2. Supports hosting of libraries building with common build tools**  
In the port.toml file of each third-party library version directory, you can specify the **build_system** field as **cmake**, **makefiles**, **ninja**, **meson**, etc., to compile with different build tools.

**3. Supports generating cmake configs**  
Celer can generate cmake configs for any libraries, especially for libraries that not build by CMake, then you can **find_package** it easily.

**4. Support cache and share build artifacts**  
Celer supports precise build artifact management. Currently, you can configure the **cache_dir** in **celer.toml** to store and access artifacts in a shared folder on the local network. This aims to avoid redundant compilation of source code and improve development efficiency.

**5. Multi-Project Management and Customization of Third-Party Libraries**  
Celer supports multi-project management, allowing project-specific customization of third-party libraries, including version and compilation options. It also enables the management of internal libraries within each project's folder.

# Get started

We have docs to guide you in using Celer:

- [Quick start.](./docs/en-US/quick_start.md)
- [How to create a new platform.](./docs/en-US/config_add_platform.md)
- [How to createa new project.](./docs/en-US/config_add_project.md)
- [How to create a new port.](./docs/en-US/config_add_port.md)

Advanced features:

- [Generate cmake configs.](./docs/en-US/config_generate_cmake_config.md)
- [Cache build artifacts.](./docs/en-US/config_cache_artifacts.md)

Supported commands:

| Command                                         | Description                                                                   |
| ----------------------------------------------- | ----------------------------------------------------------------------------- |
| [about](./docs/en-US/cmd_about.md)              | About Celer.                                                                  |
| [autoremove](./docs/en-US/cmd_autoremove.md)    | Tidy up installation directory - removing project's unnecessary files.        |
| [clean](./docs/en-US/cmd_clean.md)              | Remove build cache and clean repo for packages or projects.                   |
| [configure](./docs/en-US/quick_start.md#4-configure-platform-or-project) | Configure to change gloabal settings.                       |
| create                                | Create a [platform](./docs/en-US/config_add_platform.md), [project](./docs/en-US/config_add_project.md) or [port](./docs/en-US/config_add_port.md). |
| [deploy](./docs/en-US/cmd_deploy.md)            | Deploy with selected platform and project.                                    |
| [init](./docs/en-US/quick_start.md#3-setup-conf)| Init with conf repo.                                                          |
| [install](./docs/en-US/cmd_install.md)          | Install a package.                                                            |
| [integrate](./docs/en-US/cmd_integrate.md)      | Integrate tab completion.                                                     |
| [remove](./docs/en-US/cmd_remove.md)            | Remove a package.                                                             |
| [search](./docs/en-US/cmd_search.md)            | Search matched ports.                                                             |
| [tree](./docs/en-US/cmd_tree.md)                | Show the dependencies of a port or a project.                                 |
| [update](./docs/en-US/cmd_update.md)            | Update conf repo, ports config repo or third-party repo.                      |

# Contribute

&emsp;&emsp;Celer is an open source project, and is thus built with your contributions. Celer is consist of two parts: [Celer](https://github.com/celer-pkg/celer.git) and [ports](https://github.com/celer-pkg/ports.git), you can contribute any of them.

# License

&emsp;&emsp;The code in this repository is licensed under the MIT License. The libraries provided by ports are licensed under the terms of their original authors.
