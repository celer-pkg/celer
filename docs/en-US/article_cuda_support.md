# CUDA Environment Support

> **Seamless CUDA toolkit integration for GPU-accelerated projects**

## 🎯 Overview

Celer provides automatic CUDA toolkit detection and configuration, making it easy to build GPU-accelerated C/C++ projects with CMake. When CUDA dependencies are detected in your project configuration, Celer automatically configures the CMake toolchain file with all necessary CUDA settings.

**Key Features:**
- 🔍 **Automatic Detection** - Detects CUDA toolkit from project dependencies or installed files
- 🛠️ **Auto Configuration** - Automatically configures `CMAKE_CUDA_COMPILER`, `CUDA_TOOLKIT_ROOT_DIR`, and related CMake variables
- 🪟 **Visual Studio Integration** - Full support for Visual Studio CUDA projects on Windows
- 🐧 **Cross-Platform** - Works seamlessly on both Windows and Linux

## 💡 How It Works

Celer detects CUDA dependencies in two ways:

1. **Project Configuration Detection**: Checks if any port in your project configuration starts with `cuda` (case-insensitive)
2. **Backward Compatibility**: Falls back to checking for CUDA files (`nvcc` compiler and `cuda_runtime.h` header) in the installed directory

When CUDA is detected, Celer automatically adds the following configurations to `toolchain_file.cmake`:

- `CUDA_TOOLKIT_ROOT_DIR` - Root directory of the CUDA toolkit
- `CUDAToolkit_ROOT` - Alternative variable for CUDA toolkit root (CMake 3.17+)
- `CMAKE_CUDA_COMPILER` - Path to the NVCC compiler (`nvcc` on Linux, `nvcc.exe` on Windows)
- `CMAKE_CUDA_FLAGS_INIT` - Default CUDA compiler flags
- Windows-specific: `CMAKE_GENERATOR_TOOLSET` and `CMAKE_VS_PLATFORM_TOOLSET_CUDA` for Visual Studio integration

## 🚀 Quick Start

### Step 1: Install CUDA Toolkit

Add CUDA toolkit to your project dependencies:

```toml
[project]
ports = [
    "cuda_toolkit@12.9.1",
    # ... other dependencies
]
```

Or install it manually:

```bash
celer install cuda_toolkit@12.9.1
```

### Step 2: Configure Your Project

Configure your platform and project:

```bash
celer configure --platform=x86_64-linux-ubuntu-22.04-gcc-11.5.0
celer configure --project=my_project
celer deploy
```

Celer will automatically detect the CUDA dependency and configure the toolchain file.

### Step 3: Use in CMake Project

Use the generated `toolchain_file.cmake` in your CMake project:

```bash
cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/workspace/toolchain_file.cmake ..
cmake --build .
```

CMake will automatically detect and use the CUDA compiler configured by Celer.

## 📝 CMake Project Example

Here's a minimal example of a CMake project that uses CUDA:

**CMakeLists.txt:**

```cmake
cmake_minimum_required(VERSION 3.18)
project(my_cuda_project LANGUAGES CXX CUDA)

# Enable CUDA language
enable_language(CUDA)

# Add CUDA source file
add_executable(test_cuda
    test_cuda.cu
    main.cpp
)

# Link CUDA libraries
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
    // Launch kernel with 1 block of 4 threads
    helloKernel<<<1, 4>>>();
    cudaDeviceSynchronize();
    return 0;
}
```

When you build with the Celer-generated toolchain file, CMake will automatically:
- Detect the CUDA compiler (`nvcc`)
- Find the CUDA toolkit root directory
- Link against CUDA libraries
- Apply appropriate compiler flags

## 🔧 CUDA Compiler Flags

Celer automatically sets `CMAKE_CUDA_FLAGS_INIT` with the following flags:

- `-Wno-deprecated-gpu-targets` - Suppress warnings about deprecated GPU architectures
- `--forward-unknown-opts` - Forward unknown options to the host compiler
- `--forward-slash-prefix-opts` - Forward slash-prefixed options (**Windows only**)

> **Note**: The `--forward-slash-prefix-opts` flag is automatically added only on Windows platforms.

These flags ensure compatibility and proper compilation across different platforms.

## 🪟 Windows Visual Studio Integration

On Windows, Celer configures Visual Studio-specific CUDA settings:

- `CMAKE_GENERATOR_TOOLSET` - Sets the CUDA toolset for Visual Studio generator
- `CMAKE_VS_PLATFORM_TOOLSET_CUDA` - Configures the CUDA toolkit path for Visual Studio

This enables seamless integration with Visual Studio projects, allowing you to:
- Use Visual Studio's CUDA IntelliSense
- Debug CUDA kernels with Nsight Visual Studio Edition
- Build CUDA projects directly from Visual Studio

### Visual Studio Project Example

To use CUDA with Visual Studio:

```bash
cmake -G "Visual Studio 17 2026" -A x64 \
    -DCMAKE_TOOLCHAIN_FILE=C:/path/to/toolchain_file.cmake ..
cmake --build . --config Release
```

The toolchain file will automatically configure the CUDA toolset for Visual Studio.

## 🔍 Manual CUDA Detection

If you prefer not to declare CUDA in your project dependencies, Celer can still detect it through backward compatibility mode. Simply ensure that:

1. The `nvcc` compiler exists in `installed/<platform>/bin/`
2. The `cuda_runtime.h` header exists in `installed/<platform>/include/`

Celer will automatically detect these files and configure CUDA support.

## 🛠️ Advanced Configuration

### Using Different CUDA Versions

To use a specific CUDA version, specify it in your project dependencies:

```toml
[project]
ports = [
    "cuda_toolkit@12.9.1",  # Use CUDA 12.9.1
    # ... other dependencies
]
```

This ensures the toolchain file reflects the current CUDA installation.

## 📚 Additional CUDA Libraries

Celer supports installing additional CUDA libraries beyond the core toolkit:

- `cuda_cudart@*` - CUDA Runtime library
- `libcufft@*` - CUDA Fast Fourier Transform library
- `libcurand@*` - CUDA Random Number Generation library
- `libcusolver@*` - CUDA Dense and Sparse Direct Solvers
- `libcusparse@*` - CUDA Sparse Matrix library
- `libnpp@*` - NVIDIA Performance Primitives
- `libnvjpeg@*` - NVIDIA JPEG library
- `nsight_compute@*` - NVIDIA Nsight Compute profiler
- `nsight_systems@*` - NVIDIA Nsight Systems profiler
- `visual_studio_integration@*` - Visual Studio integration (Windows only)

Install these as needed:

```bash
celer install libcufft@12.9.1
celer install libcurand@12.9.1
# ... etc.
```