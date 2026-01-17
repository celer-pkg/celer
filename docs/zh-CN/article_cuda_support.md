# CUDA 环境支持

> **为 GPU 加速项目提供无缝的 CUDA 工具包集成**

## 🎯 概述

Celer 提供自动 CUDA 工具包检测和配置，使使用 CMake 构建 GPU 加速的 C/C++ 项目变得轻松。当在项目配置中检测到 CUDA 依赖项时，Celer 会自动在 CMake 工具链文件中配置所有必要的 CUDA 设置。

**主要特性：**
- 🔍 **自动检测** - 从项目依赖项或已安装文件中检测 CUDA 工具包
- 🛠️ **自动配置** - 自动配置 `CMAKE_CUDA_COMPILER`、`CUDA_TOOLKIT_ROOT_DIR` 和相关 CMake 变量
- 🪟 **Visual Studio 集成** - 在 Windows 上完全支持 Visual Studio CUDA 项目
- 🐧 **跨平台** - 在 Windows 和 Linux 上均可无缝工作

## 💡 工作原理

Celer 通过两种方式检测 CUDA 依赖项：

1. **项目配置检测**：检查项目配置中是否有任何端口以 `cuda` 开头（不区分大小写）
2. **向后兼容**：如果未在项目配置中找到，则回退到检查已安装目录中的 CUDA 文件（`nvcc` 编译器和 `cuda_runtime.h` 头文件）

当检测到 CUDA 时，Celer 会自动在 `toolchain_file.cmake` 中添加以下配置：

- `CUDA_TOOLKIT_ROOT_DIR` - CUDA 工具包的根目录
- `CUDAToolkit_ROOT` - CUDA 工具包根目录的替代变量（CMake 3.17+）
- `CMAKE_CUDA_COMPILER` - NVCC 编译器的路径（Linux 上为 `nvcc`，Windows 上为 `nvcc.exe`）
- `CMAKE_CUDA_FLAGS_INIT` - 默认 CUDA 编译器标志
- Windows 特定：`CMAKE_GENERATOR_TOOLSET` 和 `CMAKE_VS_PLATFORM_TOOLSET_CUDA` 用于 Visual Studio 集成

## 🚀 快速开始

### 步骤 1：安装 CUDA 工具包

在项目依赖项中添加 CUDA 工具包：

```toml
[project]
ports = [
    "cuda_toolkit@12.9.1",
    # ... 其他依赖项
]
```

或手动安装：

```bash
celer install cuda_toolkit@12.9.1
```

### 步骤 2：配置项目

配置您的平台和项目：

```bash
celer configure --platform=x86_64-linux-ubuntu-22.04-gcc-11.5.0
celer configure --project=my_project
celer deploy
```

Celer 将自动检测 CUDA 依赖项并配置工具链文件。

### 步骤 3：在 CMake 项目中使用

在 CMake 项目中使用生成的 `toolchain_file.cmake`：

```bash
cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/workspace/toolchain_file.cmake ..
cmake --build .
```

CMake 将自动检测并使用 Celer 配置的 CUDA 编译器。

## 📝 CMake 项目示例

以下是一个使用 CUDA 的最小 CMake 项目示例：

**CMakeLists.txt:**

```cmake
cmake_minimum_required(VERSION 3.18)
project(my_cuda_project LANGUAGES CXX CUDA)

# 启用 CUDA 语言
enable_language(CUDA)

# 添加 CUDA 源文件
add_executable(test_cuda
    test_cuda.cu
    main.cpp
)

# 链接 CUDA 库
target_link_libraries(test_cuda
    CUDA::cudart
)
```

**test_cuda.cu:**

```cuda
#include <cuda_runtime.h>
#include <cstdio>

__global__ void helloKernel() {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    printf("Hello from GPU thread %d\n", idx);
}

int main() {
    // 启动内核：1 个块，4 个线程
    helloKernel<<<1, 4>>>();
    cudaDeviceSynchronize();
    return 0;
}
```

当您使用 Celer 生成的工具链文件构建时，CMake 将自动：
- 检测 CUDA 编译器（`nvcc`）
- 查找 CUDA 工具包根目录
- 链接 CUDA 库
- 应用适当的编译器标志

## 🔧 CUDA 编译器标志

Celer 自动使用以下标志设置 `CMAKE_CUDA_FLAGS_INIT`：

- `-Wno-deprecated-gpu-targets` - 抑制已弃用 GPU 架构的警告
- `--forward-unknown-opts` - 将未知选项转发给主机编译器
- `--forward-slash-prefix-opts` - 转发以斜杠开头的选项（**仅 Windows**）

> **注意**：`--forward-slash-prefix-opts` 标志仅在 Windows 平台上自动添加。

这些标志确保在不同平台上的兼容性和正确编译。

## 🪟 Windows Visual Studio 集成

在 Windows 上，Celer 配置 Visual Studio 特定的 CUDA 设置：

- `CMAKE_GENERATOR_TOOLSET` - 为 Visual Studio 生成器设置 CUDA 工具集
- `CMAKE_VS_PLATFORM_TOOLSET_CUDA` - 为 Visual Studio 配置 CUDA 工具包路径

这使得与 Visual Studio 项目的无缝集成成为可能，允许您：
- 使用 Visual Studio 的 CUDA IntelliSense
- 使用 Nsight Visual Studio Edition 调试 CUDA 内核
- 直接从 Visual Studio 构建 CUDA 项目

### Visual Studio 项目示例

要使用 Visual Studio 的 CUDA：

```bash
cmake -G "Visual Studio 17 2022" -A x64 \
    -DCMAKE_TOOLCHAIN_FILE=C:/path/to/toolchain_file.cmake ..
cmake --build . --config Release
```

工具链文件将自动为 Visual Studio 配置 CUDA 工具集。

## 🔍 手动 CUDA 检测

如果您不想在项目依赖项中声明 CUDA，Celer 仍然可以通过向后兼容模式检测它。只需确保：

1. `nvcc` 编译器存在于 `installed/<platform>/bin/` 中
2. `cuda_runtime.h` 头文件存在于 `installed/<platform>/include/` 中

Celer 将自动检测这些文件并配置 CUDA 支持。

## 🛠️ 高级配置

### 使用不同的 CUDA 版本

要使用特定的 CUDA 版本，请在项目依赖项中指定：

```toml
[project]
ports = [
    "cuda_toolkit@12.9.1",  # 使用 CUDA 12.9.1
    # ... 其他依赖项
]
```

这确保工具链文件反映当前的 CUDA 安装。

## 📚 其他 CUDA 库

Celer 支持安装核心工具包之外的其他 CUDA 库：

- `cuda_cudart@*` - CUDA 运行时库
- `libcufft@*` - CUDA 快速傅里叶变换库
- `libcurand@*` - CUDA 随机数生成库
- `libcusolver@*` - CUDA 稠密和稀疏直接求解器
- `libcusparse@*` - CUDA 稀疏矩阵库
- `libnpp@*` - NVIDIA 性能原语
- `libnvjpeg@*` - NVIDIA JPEG 库
- `nsight_compute@*` - NVIDIA Nsight Compute 性能分析器
- `nsight_systems@*` - NVIDIA Nsight Systems 性能分析器
- `visual_studio_integration@*` - Visual Studio 集成（仅 Windows）

按需安装这些库：

```bash
celer install libcufft@12.9.1
celer install libcurand@12.9.1
# ... 等等
```
