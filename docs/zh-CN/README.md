<div align="center">

# Celer

轻量、非侵入、面向工程交付的 C/C++ 包管理工具，适用于以 CMake 为主的项目

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Go Report Card](https://goreportcard.com/badge/github.com/celer-pkg/celer)](https://goreportcard.com/report/github.com/celer-pkg/celer)
[![GitHub release](https://img.shields.io/github/release/celer-pkg/celer.svg)](https://github.com/celer-pkg/celer/releases)

[🌍 English](../../README.md) | [中文](./README.md)

</div>

---

Celer 管理 C/C++ 第三方库：安装、编译、缓存，按「平台 × 项目 × 构建类型」隔离版本。  
同一套库在 Windows MSVC、Linux aarch64、QNX 上各编一份，互不串。  
你的 CMake 工程不用改 —— Celer 在外部生成 `toolchain_file.cmake`，把工具链、依赖和编译参数注进去。把这份文件交给同事或 CI，构建环境一起走。

## 🚀 30 秒上手

下载预编译包：[Releases](https://github.com/celer-pkg/celer/releases)，也可以 `git clone` 后 `go build`。

```bash
# 示例配置仓库，可换成团队自己的 conf
celer init --url=https://github.com/celer-pkg/test-conf.git

# 选定平台和项目（一个 platform.toml 就是一套工具链 + sysroot）
celer configure --platform=x86_64-linux-ubuntu-22.04-gcc-11.5.0
celer configure --project=project_test_01

# 安装依赖，切到 aarch64 / Windows / QNX：只改 --platform
celer install glog@0.6.0
```

然后在自己的工程里：

```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=<celer 生成的 toolchain_file.cmake>
cmake --build build
```

📖 [完整快速入门](./quick_start.md) · [为什么选择 Celer？](./why_celer.md)

## VS Code 插件

推荐安装：[celer-vscode](https://github.com/celer-pkg/celer-vscode)，以图形化操作celer。

## 💡 非侵入：工程本身不用改

![workflow](../assets/workflow.svg)

依赖、平台、编译参数活在 Celer 的 conf 里，不写进业务仓库。

- 保留现有 CMake / 目录结构，不必为包管理器改 recipe
- 切换目标：换一个 `platform.toml`，不用重写命令行和 profile
- 生成的工具链文件可单独带走——CI、交接、复现问题都用同一份

## 🎯 谁适合用

- 同一产品要在 **Windows / Linux / 嵌入式** 上用 **MSVC、GCC、Clang** 交货
- 要交给同事或流水线一份 **可复现的构建**，而不是「在我机器上是好的」
- 要发 **私有二进制**，或维护内部制品库
- 多个项目共用机器时，不想再被依赖版本互相污染

只是给个人电脑拉几个开源库，[Conan](https://conan.io) / [vcpkg](https://vcpkg.io) / [XMake](https://xmake.io) 更现成。Celer 面向把 **交叉编译、隔离和交付** 放在前面的团队。

## 🌟 你能直接得到

| 当你需要… | Celer 怎么做 |
| --- | --- |
| **切交叉编译目标** | 一个 `platform.toml` 收齐工具链、sysroot、环境变量；ARM / x86 / QNX / Windows / Linux |
| **项目之间不串依赖** | 按「平台 × 项目 × 构建类型」隔离版本、宏和 CMake 变量 |
| **多种上游构建系统** | CMake、Makefiles、Meson、B2、QMake、Bazel、GYP |
| **私有库 / 预编译包** | 按构建哈希缓存制品，团队和 CI 共用 |
| **外网不稳定** | 访问过的三方库的 Git 仓库和工具自动进缓存 |
| **可复现 CI** | 导出工作区快照，配置原样进流水线 |

Windows 与 Linux 上的 MSVC / Clang / GCC 已可用；macOS 仍在完善。更多目标（含 QNX）见平台配置说明。

## 📚 文档

**5 分钟上手：**
- [快速开始指南](./quick_start.md) · [初始化工作区](./quick_start.md#3-配置-conf)
- [创建平台 / 项目 / 端口](./cmd_create.md)
- [安装库](./cmd_install.md) · [部署](./cmd_deploy.md)

**深入阅读：**
- [生成 CMake 配置文件](./article_generate_cmake_config.md)
- [平台配置详解](./article_platform.md) · [端口（Port）配置详解](./article_port.md) · [项目配置详解](./article_project.md)
- [PkgCache：共享缓存(基于fs/minio)](./article_pkgcache.md) · [制品缓存](./article_pkgcache_artifacts.md) · [Repo 缓存](./article_pkgcache_repos.md) · [下载缓存](./article_pkgcache_downloads.md)
- [CCache 集成](./article_ccache.md)
- [CUDA 自动识别](./article_cuda_support.md)
- [动态变量](./article_expvars.md) 
- [依赖冲突检测](./article_detect_conflict_circular.md)
- [Python 版本管理](./article_python_management.md) 
- [构建工具](./article_build_tools.md)
- [导出快照](./cmd_deploy_snapshot.md)

**命令参考：**
- [configure](./cmd_configure.md) · [install](./cmd_install.md) · [remove](./cmd_remove.md) · [update](./cmd_update.md) · [search](./cmd_search.md) · [tree](./cmd_tree.md) · [clean](./cmd_clean.md) · [autoremove](./cmd_autoremove.md) · [reverse](./cmd_reverse.md) · [integrate](./cmd_integrate.md) · [version](./cmd_version.md)

## 🤝 贡献

欢迎参与核心、ports 和插件仓库：

- **[celer](https://github.com/celer-pkg/celer)** — 包管理器核心
- **[ports](https://github.com/celer-pkg/ports)** — 端口定义与构建配置
- **[celer-vscode](https://github.com/celer-pkg/celer-vscode)** — VS Code 插件

## 📄 许可证

MIT。详见 [LICENSE](../../LICENSE)。`ports/` 中的第三方库遵循各自原始许可证。

---

<div align="center">

**C/C++ 包管理器：不改你的工程，把交叉编译做成可交付的工具链文件**

[⭐ 在 GitHub 上点星](https://github.com/celer-pkg/celer) | [📖 文档](./quick_start.md) | [🐛 报告问题](https://github.com/celer-pkg/celer/issues)

</div>
