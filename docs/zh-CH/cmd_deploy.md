
# 🚀 部署命令（Deploy）

> **一键部署所有第三方库依赖，生成工具链配置文件**

&emsp;&emsp;`deploy` 命令根据当前选择的平台和项目配置，执行所有必需的第三方库的完整构建和部署周期。部署完成后，自动生成 `toolchain_file.cmake` 文件，用于与基于 CMake 的项目无缝集成。

## 📝 命令语法

```shell
celer deploy
```

## 🔄 执行流程

`deploy` 命令会按以下顺序执行：

### 1️⃣ 初始化与配置检查
- 读取 `celer.toml` 全局配置
- 加载选定的平台配置（`conf/platforms/`）
- 加载选定的项目配置（`conf/projects/`）
- 验证配置文件完整性

### 2️⃣ 平台环境设置
- 准备目标平台的工具链环境
- 配置交叉编译工具链（如有）
- 设置编译器、链接器等构建工具
- 初始化系统根目录（sysroot）

### 3️⃣ 依赖检查
- **循环依赖检测**：检查项目中配置的所有端口及其依赖树，确保不存在循环依赖
- **版本冲突检测**：检查多个端口是否依赖同一库的不同版本，避免版本冲突

### 4️⃣ 自动化构建与安装
- 按依赖顺序逐个安装项目中配置的所有端口
- 自动下载源码（如未缓存）
- 应用补丁（patch）
- 执行配置（configure）
- 编译构建（build）
- 安装到 `installed/` 目录
- 打包到 `packages/` 目录

### 5️⃣ 生成工具链配置文件
- 在项目根目录生成 `toolchain_file.cmake`
- 包含所有已安装库的头文件路径和库文件路径
- 配置交叉编译工具链信息（如有）

## ✅ 部署成功后

当部署成功后，`toolchain_file.cmake` 文件将在项目根目录生成，您可以使用它来开发您的项目，支持任何基于 CMake 的 IDE：

- **Visual Studio** - 通过 CMake 项目支持
- **CLion** - 原生 CMake 支持
- **Qt Creator** - CMake 工具链集成
- **Visual Studio Code** - CMake Tools 扩展

### 在 CMake 项目中使用

```cmake
# 在 CMakeLists.txt 中指定工具链文件
cmake_minimum_required(VERSION 3.15)

# 方式 1: 在 CMakeLists.txt 中设置
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/toolchain_file.cmake")

project(YourProject)
```

或者在命令行中指定：

```shell
cmake -DCMAKE_TOOLCHAIN_FILE=toolchain_file.cmake -B build
cmake --build build
```

## ⚠️ 注意事项

1. **确保配置完整**：执行 `deploy` 前请确保已通过 `celer configure` 配置了平台和项目
2. **依赖检查**：如果存在循环依赖或版本冲突，部署会失败并提示错误信息
3. **构建时间**：首次部署可能需要较长时间，因为需要下载和编译所有依赖库
4. **磁盘空间**：确保有足够的磁盘空间用于源码、构建目录和安装文件

---

## 📚 相关文档

- [快速开始](./quick_start.md)
- [Configure 命令](./cmd_configure.md) - 配置平台和项目
- [Install 命令](./cmd_install.md) - 安装单个端口
- [项目配置](./advance_project.md) - 配置项目依赖
- [平台配置](./advance_platform.md) - 配置目标平台

---

**需要帮助？** [报告问题](https://github.com/celer-pkg/celer/issues) 或查看我们的 [文档](../../README.md)
