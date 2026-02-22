<div align="center">

# Celer

**轻量级、非侵入式的 C/C++ 包管理器，专为 CMake 项目设计**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Go Report Card](https://goreportcard.com/badge/github.com/celer-pkg/celer)](https://goreportcard.com/report/github.com/celer-pkg/celer)
[![GitHub release](https://img.shields.io/github/release/celer-pkg/celer.svg)](https://github.com/celer-pkg/celer/releases)

[🌍 English](../../README.md) | [中文](./README.md)

</div>

---

## ✨ 为什么选择 Celer？

Celer 是 C/C++ 的加速器，解决实际依赖管理中的核心挑战：

- 🎯 **项目零侵入** - 只需一个 `toolchain_file.cmake`你的项目就可以进行开发
- 🚀 **简化的交叉编译** - 平台感知的依赖管理，自动配置工具链编译环境
- 📦 **智能缓存** - 基于哈希的二进制制品缓存，节省数小时构建时间
- 🔧 **多构建系统支持** - 原生支持 CMake、MakeFiles、Meson、B2、QMake、GYP等
- 🏢 **企业级就绪** - 项目级配置防止依赖版本冲突和环形依赖
- 🔗 **非侵入式设计** - 可移植的 `toolchain_file.cmake` 生成后可独立使用

## 🚀 快速开始

```bash
# 1. 安装 Celer（或从 releases 下载预编译版本）
git clone https://github.com/celer-pkg/celer.git
cd celer && go build

# 2. 初始化配置仓库
celer init --url=https://github.com/celer-pkg/test-conf.git

# 3. 配置平台和项目
celer configure --platform=x86_64-linux-ubuntu-22.04-gcc-11.5.0
celer configure --project=my_project

# 4. 部署并生成工具链文件
celer deploy

# 5. 在 CMake 项目中使用
cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/workspace/toolchain_file.cmake ..
cmake --build .
```

📖 [完整快速入门指南](./quick_start.md)

## 💡 工作原理

![workflow](../assets/workflow.svg)

Celer 生成平台特定的 `toolchain_file.cmake`，作为项目与预配置构建环境及依赖之间的桥梁。这使得 Celer 完全与项目解耦 - 一旦生成工具链文件，你可以将其分享给团队，他们甚至不需要安装 Celer！

## 🌟 核心功能

| 功能特性 | 你将获得 |
| --- | --- |
| **🔧 可配置化的交叉编译平台** | 通过友好的 TOML 配置预定义 ARM、x86、Windows、Linux 等平台的工具链。 |
| **🎮 嵌入式系统支持** | 通过 `embedded_system` 标志完善支持 MCU 和裸机环境 - 无需操作系统依赖。 |
| **📁 项目级依赖管理** | 每个项目维护独立的依赖版本、环境变量、宏定义和 CMake 变量 - 避免全局冲突。 |
| **🛠️ 多构建系统支持** | 原生支持 **CMake**、**Makefiles**、**Meson**、**B2**、**QMake**、**GYP** - 无需编写复杂脚本。 |
| **📦 自动生成 CMake 配置** | 为预编译号的二进制库自动生成 CMake config 文件，确保无缝集成。 |
| **⚡ 智能二进制缓存** | 基于哈希的制品缓存，通过本地网络共享消除冗余构建，支持私有库的预编译二进制分发。 |
| **💻 开发者模式** | 通过 `celer deploy` 一次性生成 `toolchain_file.cmake`，然后使用任意 IDE 开发。 |
| **🔄 CI/CD 集成** | 在 `conf/projects` 中配置项目，无缝集成到持续集成流水线。 |
| 📸 **Workspace快照** | 可复现的工作区快照，简化错误修复与功能开发|

## 🆚 Celer vs 其他工具

Celer 解决了传统 C/C++ 包管理器难以应对的核心痛点：

| 挑战 | Conan / Vcpkg / XMake | ✅ Celer |
|-----------|----------------------|---------|
| **📦 简化库集成流程** | 需要复杂的配方脚本 | 只需声明构建系统类型 |
| **🏢 项目级依赖隔离** | 全局配置导致冲突 | 项目级隔离配置 |
| **🔗 平台多子工程管理** | 手动逐项目设置 | 单一 TOML，自动同步子项目 |
| **⚡ 智能哈希缓存** | 有限或手动缓存 | 精确的基于哈希的制品缓存 |
| **🔍 自动冲突检测** | 运行时发现 | 构建时检查并报告 |
| **🤝 无缝跨公司/团队协作** | 手动环境搭建 | 可移植的工具链文件 - 开箱即用 |

📖 [深入了解 & 详细对比说明： Celer 独特解决的问题](./why_celer.md)

## 📚 文档

**快速入门：**
- [快速开始指南](./quick_start.md) - 5 分钟上手
- [创建新平台](./cmd_create.md#1-创建一个新的平台) - 定义自定义交叉编译环境
- [创建新项目](./cmd_create.md#2-创建一个新的项目) - 配置项目特定设置
- [添加新包](./cmd_create.md#3-创建一个新的端口) - 托管你自己的库

**高级功能：**
- [生成 CMake 配置文件](./article_generate_cmake_config.md) - 为预编译好的二进制库自动生成配置
- [缓存构建产物](./article_package_cache.md) - 通过智能缓存每个库的编译产物来加速项目集成
- [支持CCache](./article_ccache.md) - 通过缓存编译结果来加速重新编译
- [库版本冲突和环形依赖检测](./article_detect_conflict_circular.md) - 在编译前提前发现导致环形依赖和冲突的错误配置
- [CUDA环境自动识别](./article_cuda_support.md) - 为 GPU 加速项目提供无缝的 CUDA 工具包集成
- [导出快照](./cmd_deploy_export.md) - 当部署项目成功后允许导出当前的workspace为一个可以回溯编译的快照

## 📋 命令列表

| 命令                                   | 描述  |
| ------------------------------------- | -----|
| [autoremove](./cmd_autoremove.md)     | 清理安装目录，移除项目不依赖的库 |
| [clean](./cmd_clean.md)               | 清理指定目标的构建缓存，或使用 `--all` 清理全部 |
| [configure](./cmd_configfure.md)      | 修改workspace的全局配置 |
| [create](./cmd_create.md)             | 创建平台、项目或端口 |
| [deploy](./cmd_deploy.md)             | 以选择的*平台*和*项目*部署项目 |
| [init](./quick_start.md#3-setup-conf) | 用一个conf仓库初始化Celer |
| [install](./cmd_install.md)           | 安装一个库 |
| [integrate](./cmd_integrate.md)       | 集成以支持tab补全 |
| [remove](./cmd_remove.md)             | 移除已安装的库 |
| [reverse](./cmd_reverse.md)           | 反向查询依赖指定的库 |
| [search](./cmd_search.md)             | 搜索库 |
| [tree](./cmd_tree.md)                 | 显示三方库或项目的依赖关系 | 
| [update](./cmd_update.md)             | 仓库模式不接收端口参数；端口模式至少需要一个 `name@version` |
| [version](./cmd_version.md)           | 显示Celer版本信息 |

## 🤝 贡献

Celer 是一个欢迎由社区贡献构建的开源项目，欢迎为以下部分做出贡献：

- **[celer](https://github.com/celer-pkg/celer)** - 核心包管理器实现
- **[ports](https://github.com/celer-pkg/ports)** - 包定义和构建配置

无论你想添加新功能、改进文档还是贡献新的包定义，我们都欢迎你的帮助！

## 📄 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](../../LICENSE) 文件。

ports 仓库中的第三方库依据其各自的原始许可条款进行授权。

---

<div align="center">

**用 ❤️ 为 C/C++ 社区打造**

[⭐ 在 GitHub 上为我们点星](https://github.com/celer-pkg/celer) | [📖 文档](./quick_start.md) | [🐛 报告问题](https://github.com/celer-pkg/celer/issues)

</div>
