<div align="center">

# Celer

**轻量、非侵入、面向工程交付的 C/C++ 包管理工具，适用于以 CMake 为主的项目**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Go Report Card](https://goreportcard.com/badge/github.com/celer-pkg/celer)](https://goreportcard.com/report/github.com/celer-pkg/celer)
[![GitHub release](https://img.shields.io/github/release/celer-pkg/celer.svg)](https://github.com/celer-pkg/celer/releases)

[🌍 English](../../README.md) | [中文](./README.md)

</div>

---

> **Celer 专注于交付效率、交叉编译和团队协作，而非通用生态规模。**

## 🚀 快速开始

```bash
# 1. 安装
git clone https://github.com/celer-pkg/celer.git
cd celer && go build

# 2. 初始化配置仓库
celer init --url=https://github.com/celer-pkg/test-conf.git

# 3. 选择平台和项目
celer configure --platform=x86_64-linux-ubuntu-22.04-gcc-11.5.0
celer configure --project=project_test_01

# 4. 安装一个库
celer install glog@0.6.0
```

📖 [完整快速入门指南](./quick_start.md)

## 🎯 谁适合使用

Celer 为面临 **真实 C/C++ 交付挑战** 的团队而设计：

- 你要在 **多平台**（Windows、Linux、MacOS）上用 **不同编译器**（MSVC、GCC、Clang）维护项目。
- 你需要 **可复现的构建环境** 交付给同事或 CI，而不是"在我机器上是好的"。
- 你发布 **私有二进制库**，或维护内部制品仓库。
- 你受够了依赖版本冲突在不同项目之间串扰。

如果只是获取开源库，[Conan](https://conan.io) / [vcpkg](https://vcpkg.io) / [XMake](https://xmake.io) 已经是成熟方案。Celer 适合把 **交付一致性** 放在生态广度前面的工程团队。

## 💡 工作原理

![workflow](../assets/workflow.svg)

Celer 根据平台和项目配置生成 `toolchain_file.cmake`。你的 CMake 项目保持不变——依赖、工具链、环境变量和编译参数由外部注入。

- ✅ 保留原有 CMake 结构
- ✅ 依赖和平台配置独立于代码库
- ✅ 生成的工具链文件可独立分发——用于 CI、交接或问题复现

## 🖥️ 平台与编译器支持

Celer 致力于为各主流平台和编译器提供一流的交叉编译支持：

|              | 💻 **Windows** | 🐧 **Linux** | 🍎 **MacOS** |
| ------------ | :---: | :---: | :---: |
| **MSVC**     | ✅   | —    | —    |
| **Clang-CL** | ✅   | —    | —    |
| **Clang**    | ✅   | ✅   | 🚧   |
| **GCC**      | —    | ✅   | 🚧   |

> ✅ = 已支持 &nbsp;&nbsp; 🚧 = 规划中 &nbsp;&nbsp; — = 不适用

详见 [`conf/platforms/`](../../conf/platforms/) 所有平台配置（x86_64、aarch64、QNX 等）。

## 🌟 能力一览

| 当你需要… | Celer 的处理方式 |
| --- | --- |
| **交叉编译** | 统一 TOML 配置工具链、sysroot、环境变量，覆盖 ARM / x86 / QNX / Windows / Linux |
| **项目隔离** | 每个项目独立的依赖版本、宏和 CMake 变量——无全局泄漏 |
| **多构建系统** | 原生支持 CMake、Makefiles、Meson、B2、QMake、GYP |
| **二进制分发** | 基于哈希的制品缓存，适用于私有库和预编译包 |
| **离线/隔离环境** | Repo 缓存在无法访问 GitHub/GitLab 时复用源码树 |
| **可复现 CI** | 导出工作区快照，配置可直接进入流水线 |
| **嵌入式/MCU** | `embedded_system` 支持，不强依赖传统操作系统运行时 |

## 🆚 Celer 的差异化

| 维度 | Conan / vcpkg / XMake | ✅ Celer |
| --- | --- | --- |
| **侵入性** | 需适配 recipe 或生态约定 | 通过 `toolchain_file.cmake` 集成——项目保持不变 |
| **交叉编译** | 工具链路径、sysroot、target triple 分散在命令行和多个配置文件中，切换目标平台需重写大量参数 | 一个 `platform.toml` 集中定义：编译器路径、系统根、依赖、环境变量，切换平台只需改一个文件 |
| **项目隔离** | 共享配置易导致版本冲突 | 依赖作用域按项目隔离 |
| **多工程协同** | 常需逐项目手工接线 | 一份配置协调多个子工程 |
| **私有二进制** | 需额外封装和流程补丁 | 为内部制品库和定制交付链路而设计 |
| **缓存** | 缺乏团队级复用关注 | 基于哈希的缓存，强调团队级构建稳定性 |
| **复现** | 使用方需理解完整工具链体系 | 自包含的工具链文件和快照 |

## 📚 文档

**5 分钟上手：**
- [快速开始指南](./quick_start.md) · [初始化工作区](./quick_start.md#3-配置-conf)
- [创建平台 / 项目 / 端口](./cmd_create.md)
- [安装库](./cmd_install.md) · [部署](./cmd_deploy.md)

**深入阅读：**
- [为预编译库生成 CMake 配置](./article_generate_cmake_config.md)
- [PkgCache：共享缓存与 NFS](./article_pkgcache.md) · [制品缓存](./article_pkgcache_artifacts.md) · [Repo 缓存](./article_pkgcache_repos.md) · [下载缓存](./article_pkgcache_downloads.md)
- [CCache 集成](./article_ccache.md) · [CUDA 检测](./article_cuda_support.md)
- [动态变量](./article_expvars.md) · [依赖冲突检测](./article_detect_conflict_circular.md)
- [Python 版本管理](./article_python_management.md) · [构建工具](./article_build_tools.md)
- [导出快照](./cmd_deploy_snapshot.md)

**命令参考：**
- [全部命令](./cmd_configure.md) — `configure` · `install` · `remove` · `update` · `search` · `tree` · `clean` · `autoremove` · `reverse` · `integrate` · `version`

## 🤝 贡献

欢迎参与核心和 ports 两个仓库：

- **[celer](https://github.com/celer-pkg/celer)** — 包管理器核心
- **[ports](https://github.com/celer-pkg/ports)** — 端口定义与构建配置

## 📄 许可证

MIT。详见 [LICENSE](../../LICENSE)。`ports/` 中的第三方库遵循各自原始许可证。

---

<div align="center">

**为复杂 C/C++ 工程交付而设计**

[⭐ 在 GitHub 上点星](https://github.com/celer-pkg/celer) | [📖 文档](./quick_start.md) | [🐛 报告问题](https://github.com/celer-pkg/celer/issues)

</div>