# Quick start

## 1. Clone the repository

The first step is to clone the celer repository from GitHub. This the source of whole celer project. To do this, run:

```shell
git clone https://github.com/celer-pkg/celer.git
```

## 2. Build Celer

  - Install the Go SDK by referring https://go.dev/doc/install.
  - cd celer && go build.

  **Tips:**  
  In China, you may need to set proxy for go, like this:

  ```shell
  export GOPROXY=https://goproxy.cn
  ```

>**Note:** When a stable version is released, users can directly download the pre-built binaries, skipping the first two steps.

## 3. Setup conf

Different C++ projects often require distinct build environments and dependencies. Celer recommends using **conf** to define cross-compiling environments and third-party dependencies for each project. The strucure of **conf** should be like as below:

```
conf
├── platforms
│   ├── aarch64-linux-gnu-gcc-9.2.toml
│   ├── x86_64-linux-ubuntu-20.04.toml
│   ├── x86_64-linux-ubuntu-22.04.toml
│   └── x86_64-windows-msvc-14.44.toml
└── projects
    ├── test_project_01 --------------- project_01's dependencies
    │   ├── boost --------------------- override public build options
    │   │   └── 1.87.0
    │   │       └── port.toml
    │   └── sqlite3 ------------------- override public build options
    │       └── 3.49.0
    │           └── port.toml
    ├── test_project_01.toml ---------- project_01
    ├── test_project_02 --------------- project_02's dependencies
    │   ├── ffmpeg -------------------- override public build options
    │   │   └── 5.1.6
    │   │       └── port.toml
    │   ├── lib_001 ------------------- second project's private library
    │   │   └── port.toml
    │   └── lib_002 ------------------- project_02's private library
    │       └── port.toml
    └── test_project_02.toml ---------- project_02
```

>About how to create new **platform**, **project** and **port**, you can refer: [**add platform**](./cmd_create.md#1-create-a-new-platform), [**add project**](./cmd_create.md#2-create-a-new-project) and [**add port**](./cmd_create.md#3-create-a-new-port).

The following are conf files and their descriptions:

| file | description |
| ----- | ---------- |
| platforms/*.toml | Define platforms with toolchains and rootfs. |
| projects/*.toml  | Defines projects with dependencies, CMake variables, C++ macros, and build options for it.|
| projects/*/port.toml | Used to override project-specific third-party library versions, custom build parameters, and define private libraries for projects. |

>**Note:**  
&emsp;&emsp;Although **conf** is highly recommend for Celer, Celer can still work without **conf**. In that case, We can only use Celer to build third-party libraries with locally toolchains:
>
>- In Windows, Celer locates installed Visual Studio via **vswhere** as the default toolchain. For makefile-based libraries, it automatically downloads and configures MSYS2.
>
>- In Linux, Celer automatically uses locally installed x86_64 gcc/g++ toolchain.

To setup conf, run:

```
celer setup --url=https://github.com/celer-pkg/test-conf.git
```

Then the **celer.toml** file will be generated in the workspace directory:

```toml
[global]
  conf_repo = "https://github.com/celer-pkg/test-conf.git"
  platform = ""
  project = ""
  jobs = 16
  build_type = "release"
```

>**Tips:**  
>  **https://github.com/celer-pkg/test-conf.git** is a test conf repo, you can use it to experience celer, and you can also create your own conf repo as a reference.

## 4. Configure platform or project

**platform** and **project** are two combinations, they can be freely combined. For example, although the target environment is **aarch64-linux**, you can choose to compile/develop/debug in the **x86_64-linux** platform.

```shell
celer configure --platform=x86_64-linux-20.04
celer configure --project=test_project_02
```

Then the **celer.toml** file would be updated as below:

```toml
[global]
  conf_repo = "https://github.com/celer-pkg/test-conf.git"
  platform = "aarch64-linux-gnu-gcc-9.2"
  project = "test_project_02"
  jobs = 16
  build_type = "release"

[cache_dir]
  dir = "/home/phil/celer_cache"
```

The following are fields and their descriptions:

| field | description |
| ----- | ----------- |
| conf_repo |  Url of repo used to save configurations of platforms and projects |
| platform | Selected platform for current workspace, when it's empty, celer will use detect local toolchain to compile your libraires and projects. |
| project | Selected project for current workspace, When it's empty, there'll be a project name called "unname". |
| jobs | The max cpu cores for celer to compile, default is the number of cores of your cpu. |
| build_type | Default is **release**, you can also set it to **debug**. |
| cache_dir | Celer supports cache build artifact, which can avoid redundant compilation. [You can configure it as a local directory or a shared folder in the LAN](./introduce_cache_artifacts.md). |

## 5. Depoy Celer

Deploy Celer is to build third-party libraries required in project with build environment in selected platform. To deploy celer, run:

```shell
celer deploy
```

&emsp;&emsp;A successful celer deploy generates **toolchain_file.cmake** under workspace directory, allowing projects to depend solely on this file - making Celer no longer required thereafter. Furthermore, you can pack workspace with **installed folder**, **downloaded folder** and **toolchain_file.cmake** inside, this can be the build environment with others.  

## 6. Build your CMake project

With the **toolchain_file.cmake** generated by Celer, build your cmake projects will be quite easy as below:

```shell
# option1: 
set(CMAKE_TOOLCHAIN_FILE "/xx/workspace/toolchain_file.cmake")  

# option2: 
cmake .. -DCMAKE_TOOLCHAIN_FILE="/xx/workspace/toolchain_file.cmake"
```