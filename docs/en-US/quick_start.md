# 🚀 Quick Start

## 📋 Table of Contents

1. [Clone the Repository](#1-clone-the-repository)
2. [Build Celer](#2-build-celer)
3. [Setup Conf](#3-setup-conf)
4. [Configure Platform or Project](#4-configure-platform-or-project)
5. [Deploy Celer](#5-deploy-celer)
6. [Build Your CMake Project](#6-build-your-cmake-project)

---

## 1. 📦 Clone the Repository

The first step is to clone the Celer repository from GitHub. This is the source code of the entire Celer project.

**Run the command:**

```shell
git clone https://github.com/celer-pkg/celer.git
```

---

## 2. 🔨 Build Celer

### Build Steps

1. **Install the Go SDK**  
   Refer to the official documentation: https://go.dev/doc/install

2. **Build Celer**  
   Navigate to the Celer directory and run:
   ```shell
   cd celer
   go build
   ```

### 💡 Tips

In China, you may need to set a proxy for Go:

```shell
export GOPROXY=https://goproxy.cn
```

### 📌 Note

Currently, Celer has released stable versions. You can directly download the pre-built binaries, skipping the first two steps.

**Download Link:** https://github.com/celer-pkg/celer/releases

---

## 3. ⚙️ Setup Conf

### What is Conf?

Different C++ projects often require distinct build environments and dependencies. Celer recommends using **conf** to define cross-compiling environments and third-party dependencies for each project.

### Conf Directory Structure

```
conf
├── buildtools                        # Build tools configuration
│   ├── x86_64-linux.toml
│   └── x86_64-windows.toml
├── platforms/                        # Platform configurations
│   ├── aarch64-linux-gnu-gcc-9.2.toml
│   ├── x86_64-linux-ubuntu-22.04-gcc-11.5.0.toml
│   └── x86_64-windows-msvc-community-14.toml
└── projects                          # Project configurations
    ├── project_test_01               # Project 01 dependencies
    │   ├── boost                     # Override public build options
    │   │   └── 1.87.0
    │   │       └── port.toml
    │   └── sqlite3                   # Override public build options
    │       └── 3.49.0
    │           └── port.toml
    ├── project_test_01.toml          # Project 01 configuration
    ├── project_test_02               # Project 02 dependencies
    │   ├── ffmpeg                    # Override public build options
    │   │   └── 5.1.6
    │   │       └── port.toml
    │   ├── lib_001                   # Private library
    │   │   └── port.toml
    │   └── lib_002                   # Private library
    │       └── port.toml
    └── project_test_02.toml          # Project 02 configuration
```

### 📚 Conf Files Description

| File Path | Description |
|-----------|-------------|
| `buildtools/*.toml` | Define extra build tools required by some libraries |
| `platforms/*.toml` | Define platform configurations with toolchains and rootfs |
| `projects/*.toml` | Define project configurations with dependencies, CMake variables, C++ macros, and build options |
| `projects/*/port.toml` | Override project-specific third-party library versions, custom build parameters, and define private libraries |

### 🔗 Related Documentation

- [Create a New Platform](./cmd_create.md#1-create-a-new-platform)
- [Create a New Project](./cmd_create.md#2-create-a-new-project)
- [Create a New Port](./cmd_create.md#3-create-a-new-port)

### 📌 Note

Although **conf** is highly recommended for Celer, Celer can still work without **conf**. In that case, Celer will use local toolchains to build third-party libraries:

- **Windows Environment:** Celer locates installed Visual Studio via `vswhere` as the default toolchain. For makefile-based libraries, it automatically downloads and configures MSYS2.
- **Linux Environment:** Celer automatically uses the locally installed x86_64 gcc/g++ toolchain.

### Initialize Conf

Run the following command to setup conf:

```shell
celer init --url=https://github.com/celer-pkg/test-conf.git
```

### 🌏 Configure Proxy (Optional)

**If you're in China, it's recommended to configure a proxy to access GitHub and other resources:**

```shell
celer configure --proxy-host=127.0.0.1 --proxy-port=7890
```

### 📄 Generated Configuration File

After initialization, a `celer.toml` configuration file will be generated in the workspace directory:

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

### 💡 Tips

- **Test Repository:** `https://github.com/celer-pkg/test-conf.git` is a test conf repository. You can use it to experience Celer, and you can also create your own conf repository based on it.
- **Ports Repository:** During initialization, Celer will clone a ports repository into the current workspace, which contains configuration files for all available third-party libraries.
  - Celer will use the ports repository specified in the `CELER_PORTS_REPO` environment variable if it's set
  - If the environment variable is not set, Celer will use the default ports repository: `https://github.com/celer-pkg/ports.git`

---

## 4. 🎯 Configure Platform or Project

### Flexible Combination

**platform** and **project** are two independent configurations that can be freely combined. For example, even if the target environment is **aarch64-linux**, you can choose to compile/develop/debug on the **x86_64-linux** platform.

### Configuration Commands

```shell
# Configure platform
celer configure --platform=x86_64-linux-ubuntu-22.04-gcc-11.5.0

# Configure project
celer configure --project=project_test_02
```

### Updated Configuration File

After configuration, the `celer.toml` file will be updated as follows:

```toml
[global]
  conf_repo = "https://github.com/celer-pkg/test-conf.git"
  platform = "x86_64-linux-ubuntu-22.04-gcc-11.5.0"
  project = "project_test_02"
  jobs = 16
  build_type = "release"
  offline = false
  verbose = false

[pkgcache]
  dir = "/home/phil/celer_cache"
```

### 📊 Configuration Fields Description

| Field | Description |
|-------|-------------|
| `conf_repo` | URL of the repository used to save platform and project configurations |
| `platform` | Selected platform for the current workspace. When empty, Celer will auto-detect and use the local toolchain to compile libraries and projects |
| `project` | Selected project for the current workspace. When empty, a default project named \"unname\" will be created |
| `jobs` | Maximum CPU cores for Celer to compile, default is the number of CPU cores in your system |
| `build_type` | Build type, default is `release`, can also be set to `debug` |
| `offline` | Offline mode. When enabled, Celer will not try to update repositories or download resources |
| `verbose` | Verbose mode. When enabled, Celer will show more detailed logs during building |
| `pkgcache` | Package cache configuration. Celer supports both [build artifact caching](./article_pkgcache_artifacts.md) and [source repository caching](./article_pkgcache_repos.md). It can be configured as a local directory or a shared LAN folder. |

---

## 5. 🚀 Deploy Celer

### What is Deployment?

Deploying Celer is the process of building all third-party libraries required by the project in the build environment of the selected platform.

### Execute Deployment

```shell
celer deploy
```

### Deployment Artifacts

After a successful deployment, the following files and directories will be generated in the workspace directory:

- **`toolchain_file.cmake`** - CMake toolchain file, allowing projects to depend solely on this file without requiring Celer
- **`installed/`** - Directory containing installed third-party libraries
- **`downloads/`** - Directory containing downloaded source code

### 💡 Tips

You can package the workspace with the following three parts to distribute the build environment to other users:

1. `installed/` folder
2. `downloads/` folder
3. `toolchain_file.cmake` file

---

## 6. 🏗️ Build Your CMake Project

With the `toolchain_file.cmake` generated by Celer, building your CMake projects becomes very easy.

### Option 1: Set in CMakeLists.txt

```cmake
set(CMAKE_TOOLCHAIN_FILE "/path/to/workspace/toolchain_file.cmake")
```

### Option 2: Specify via Command Line

```shell
cmake .. -DCMAKE_TOOLCHAIN_FILE="/path/to/workspace/toolchain_file.cmake"
```

### 📌 Notes

- Replace `/path/to/workspace/toolchain_file.cmake` with the actual path to your toolchain file
- After using the toolchain file, CMake will automatically find all installed third-party libraries
- The project no longer depends on Celer and can be built independently

---

## 📚 Related Documentation

- [Advanced Platform Configuration](./article_platform.md)
- [Advanced Port Configuration](./article_port.md)
- [Caching Build Artifacts](./article_pkgcache_artifacts.md)
- [Caching Source Repositories](./article_pkgcache_repos.md)
- [Command Reference](./cmd_install.md)

---

## ❓ Get Help

For more help, run:

```shell
celer --help
```

Or visit the [Celer Official Documentation](https://github.com/celer-pkg/celer)
