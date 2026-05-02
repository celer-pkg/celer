<div align="center">

# Celer

**轻量、非侵入、面向工程交付的 C/C++ 包管理工具，适用于以 CMake 为主的项目**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Go Report Card](https://goreportcard.com/badge/github.com/celer-pkg/celer)](https://goreportcard.com/report/github.com/celer-pkg/celer)
[![GitHub release](https://img.shields.io/github/release/celer-pkg/celer.svg)](https://github.com/celer-pkg/celer/releases)

[🌍 English](../../README.md) | [中文](./README.md)

</div>

---

## ✨ 为什么选择 Celer？

Celer 的设计目标是通过屏蔽 C/C++ 项目在**构建**、**编译**、**安装**等环节的实现细节，降低代码库托管、接手和复用的门槛；同时将项目的**依赖管理**与**编译环境**从业务代码中独立出来。通过这种多维度解耦，项目的结构边界更清晰，管理方式也更直观。

在实际的 C/C++ 工程中，常见问题通常包括：

- 🎯 **工程接入成本高**：现有项目往往依赖特定构建约定，接手时需要改代码、改构建脚本，迁移成本高。
- 🚀 **交叉编译环境复杂**：编译器、sysroot、ABI、环境变量和依赖来源分散，平台切换和环境复现成本高。
- 📦 **构建产物难复用**：重复编译频繁发生，不同机器和不同阶段产物不一致，容易引入环境漂移。
- 🔧 **三方库的构建工具多样化**：CMake、Make、Meson、B2、QMake等应对不过来。
- 🏢 **项目之间相互污染**：依赖版本、宏、环境变量容易全局共享，导致冲突和踩踏。
- 🔗 **团队协作与交付困难**：环境依赖强绑定在本地机器上，项目交接、协作和 CI 落地成本高。

## 🚀 快速开始

```bash
# 1. 安装 Celer（或从 releases 下载预编译版本）
git clone https://github.com/celer-pkg/celer.git
cd celer && go build

# 2. 初始化配置仓库
celer init --url=https://github.com/celer-pkg/test-conf.git

# 3. 配置平台和项目
celer configure --platform=x86_64-linux-ubuntu-22.04-gcc-11.5.0
celer configure --project=project_test_01

# 4. 测试构建、编译和安装一个代码库
celer install glog@0.6.0
```

📖 [完整快速入门指南](./quick_start.md)

## 💡 工作原理

![workflow](../assets/workflow.svg)

Celer 会根据选定的平台和项目配置生成一个平台感知的 `toolchain_file.cmake`，把你的工程与预定义的工具链、依赖、环境变量和构建参数连接起来。

这意味着：

- 你的项目仍然保持原有的 CMake 组织方式；
- 依赖和平台配置由外部统一管理；
- 工具链文件生成后可以独立使用，适合团队协作、CI/CD 和问题复现。

## 🌟 核心能力

| 能力 | 价值 |
| --- | --- |
| **交叉编译** | 用 TOML 统一描述 ARM、x86、QNX、Windows、Linux 等平台的工具链与构建环境。 |
| **项目隔离** | 每个项目拥有独立的依赖版本、环境变量、宏和 CMake 变量，降低多项目并行开发的冲突风险。 |
| **构建系统** | 原生兼容 **CMake**、**Makefiles**、**Meson**、**B2**、**QMake**、**GYP**，降低三方库托管难度。 |
| **CMake 配置** | 对预编译二进制库自动补齐 CMake 配置，降低接入门槛。 |
| **制品缓存** | 基于哈希缓存构建产物，适合私有库、二进制分发和大规模重复构建场景。 |
| **Repo 缓存** | 通过缓存源码仓库复用源代码，在外网不能访问或者github、gitlab不能访问时候，降低集成成本。 |
| **嵌入式** | 通过 `embedded_system` 支持 MCU 和裸机环境，不强依赖传统操作系统运行时。 |
| **开发模式** | 通过 `celer deploy` 生成工具链后，可直接使用任意 IDE 继续开发。 |
| **CI/CD 集成** | 平台和项目配置可直接进入流水线，减少 CI 环境与开发环境的偏差。 |
| **快照** | 支持导出可复现的工作区快照，便于问题追踪、回溯和协作交接。 |

## 🆚 Celer 相对其他 C++ 包管理器的优势

如果你的诉求只是“获取开源库”，那么 Conan、vcpkg、XMake 都有成熟解法。

Celer 的优势在于它更关注**复杂工程环境里的交付效率和一致性**：

| 维度 | Conan / vcpkg / XMake 常见做法 | ✅ Celer 的优势 |
| --- | --- | --- |
| **侵入性** | 往往需要适配 recipe、port、专用集成方式 | 通过 `toolchain_file.cmake` 对接现有 CMake 工程，侵入性低 |
| **交叉建模** | 需要额外拼接 toolchain、profile、triplet | 平台、工具链、环境变量和依赖在同一套配置里统一描述 |
| **项目隔离** | 容易落入全局配置或共享环境带来的冲突 | 项目维度维护依赖、变量和构建参数，边界更清晰 |
| **多工程协同** | 常需逐项目手工接线 | 单一配置可同步多个子工程，降低维护成本 |
| **私有二进制** | 需要额外封装和流程补丁 | 更适合企业内部制品库、预编译包和定制交付链路 |
| **缓存与重建** | 缓存能力可用，但通常不聚焦工程级制品复用 | 基于哈希的制品缓存更强调团队级复用和构建稳定性 |
| **共享与复现** | 通常要求使用方也理解完整工具链体系 | 生成的工具链文件和快照更利于共享、复现和交接 |

一句话概括：

> **通用生态不是 Celer 的核心卖点；工程落地、交叉编译、私有依赖治理和团队协作效率才是。**

📖 [深入说明：Celer 解决了哪些实际问题](./why_celer.md)

## 📚 文档

**快速入门：**
- [快速开始指南](./quick_start.md) - 5 分钟上手
- [创建新平台](./cmd_create.md) - 定义自定义交叉编译环境
- [创建新项目](./cmd_create.md) - 配置项目级设置
- [添加新端口](./cmd_create.md) - 托管和管理自己的库

**高级功能：**
- [生成 CMake 配置文件](./article_generate_cmake_config.md) - 为预编译二进制库自动生成配置
- [缓存构建产物](./article_pkgcache_artifacts.md) - 复用已构建产物，降低重复集成成本
- [缓存源码仓库](./article_pkgcache_repos.md) - 上游仓库不稳定时，通过 Repo Cache 复用源码树
- [支持 CCache](./article_ccache.md) - 通过缓存编译结果加速重复编译
- [动态变量](./article_expvars.md) - 查看 TOML 配置中可用的动态变量
- [库版本冲突与环形依赖检测](./article_detect_conflict_circular.md) - 在构建前提前发现冲突和错误依赖关系
- [CUDA 环境自动识别](./article_cuda_support.md) - 为 GPU 工程自动对接 CUDA 工具链
- [Python 版本管理](./article_python_management.md) - 为编译依赖配置和管理 Python 版本
- [导出快照](./cmd_deploy_export.md) - 在部署完成后导出可回溯、可复现的工作区快照

## 📋 命令列表

| 命令 | 描述 |
| --- | --- |
| [autoremove](./cmd_autoremove.md) | 清理安装目录，移除当前项目不再依赖的库 |
| [clean](./cmd_clean.md) | 清理指定目标的构建缓存，或使用 `--all` 清理全部 |
| [configure](./cmd_configure.md) | 修改当前 workspace 的全局配置 |
| [create](./cmd_create.md) | 创建平台、项目或端口 |
| [deploy](./cmd_deploy.md) | 使用当前选择的平台和项目执行部署 |
| [init](./quick_start.md#3-配置-conf) | 使用配置仓库初始化 Celer |
| [install](./cmd_install.md) | 安装一个库 |
| [integrate](./cmd_integrate.md) | 集成 shell 补全 |
| [remove](./cmd_remove.md) | 移除已安装的库 |
| [reverse](./cmd_reverse.md) | 反向查询依赖指定库的项目或库 |
| [search](./cmd_search.md) | 搜索可用端口 |
| [tree](./cmd_tree.md) | 查看库或项目的依赖树 |
| [update](./cmd_update.md) | 仓库模式不接收端口参数；端口模式至少需要一个 `name@version` |
| [version](./cmd_version.md) | 查看 Celer 版本信息 |

## 🤝 贡献

Celer 是一个面向社区协作的开源项目，欢迎参与以下部分：

- **[celer](https://github.com/celer-pkg/celer)** - 包管理器核心实现
- **[ports](https://github.com/celer-pkg/ports)** - 端口定义与构建配置

如果你希望补充功能、改进文档，或新增端口定义，欢迎提交贡献。

## 📄 许可证

本项目基于 MIT 许可证发布，详见 [LICENSE](../../LICENSE) 文件。

`ports` 仓库中的第三方库遵循各自原始许可证条款。

---

<div align="center">

**为复杂 C/C++ 工程交付而设计**

[⭐ 在 GitHub 上点星](https://github.com/celer-pkg/celer) | [📖 文档](./quick_start.md) | [🐛 报告问题](https://github.com/celer-pkg/celer/issues)

</div>
