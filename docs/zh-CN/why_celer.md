# 为什么选择 Celer？

> *Celer 是一个面向 C/C++ 工程交付的包管理器，希望成为 C/C++ 项目的 Accelerator，专注解决依赖管理里最贵、最痛、最容易反复踩坑的部分。*

## 🎯 Celer 的核心设计

### 1. TOML 声明式配置 —— 几秒钟托管一个库

`port.toml` + `project.toml` + `platform.toml` 全部 TOML 声明式，不写脚本。修改一个 port 的构建选项 = 改一行 TOML，不需要编程。

声明式的最大好处是**低门槛**：不需要学习 Python 脚本或 CMake 脚本语法，任何一个能读懂配置文件的开发者都能托管一个新库。这大幅降低了新库集成的周期 —— 从"找熟手写 recipe"变成"改一行 TOML"。

### 2. 交付导向 —— 一键构建+安装+可复现交付

`celer deploy` 一键构建并安装整个项目的所有依赖，生成可交付的安装目录。内置 snapshot 导出（`--snapshot`），记录每个 port 的精确 commit + 构建环境，实现可复现交付。

这意味着构建结果不只停留在"编译通过"，而是直接产出可交付、可回溯的产物。同一份 snapshot 在任何机器上都能复现完全一致的构建环境。

### 3. 项目-平台-构建类型三维隔离

`installed/<platform>@<project>@<buildType>/` 目录结构天然隔离。同一台机器上可以同时存在 `x86_64-linux@project_xx@release`、`aarch64-linux@project_xx@debug`，互不干扰。切换 `celer configure --project=project_xx --build-type=debug` 即可。

不需要靠命名约定或 profile 文件来隔离 —— 目录结构本身就是隔离边界，一眼可见、不会混淆。

### 4. 项目级 port 覆盖 + vendor 目录

项目可以在 `conf/projects/<proj>/ports/` 下放自己的 port，覆盖全局 ports 仓库的版本。三方库和项目自有 port 物理隔离，一眼分清。支持三种查找位置（项目顶层 → 项目 vendor → 全局 ports），同名冲突报错。

这让"项目定制某个三方库的构建选项"变得极简 —— 在项目 vendor 目录放一个 port.toml，不影响全局仓库，不污染其它项目。

### 5. 交叉编译原生支持

平台配置（`conf/platforms/*.toml`）定义 toolchain、sysroot、rootfs，`celer configure --platform=aarch64-linux-xxx` 一切就绪。内置 toolchain file 生成、cross-compiler 自动检测。

交叉编译的所有复杂度（工具链路径、sysroot、ABI 兼容、环境变量）被收敛到一个 TOML 文件里，不需要用户自己拼凑 toolchain、profile、triplet。

### 6. 精确的缓存管理 —— Meta 驱动的缓存 Key

C/C++ 项目编译最大的痛点之一就是编译慢。充分利用缓存并增大缓存利用率是 Celer 的核心设计之一。

通过 Celer 编译的每个库都会记录这个库的编译环境信息、commit hash、依赖信息、以及依赖的 commit hash，最终生成一个 hash，通过 hash 来寻找匹配的缓存。当发现底层依赖的 hash 变了，会自动触发关联库的重新编译。

Celer 计算 **meta** —— 一个包含**全部影响编译结果的因素**的元数据字符串，取 `sha256(meta)` 作为缓存 key。meta 自动递归收集：

| 因素 | 说明 |
| --- | --- |
| **port.toml 原文** | url、ref、checksum、patches、build_options、envs、dependencies... |
| **传递依赖的完整 port.toml** | 递归展开，任何一层依赖变了都影响 hash |
| **精确 commit hash** | 从 git 解析，不是 ref 名（同一 branch 不同 commit 产物不同） |
| **平台 toolchain 配置** | compiler、sysroot、flags... |
| **构建类型** | release / debug / relwithdebinfo... |
| **构建工具版本** | cmake 版本、ninja 版本... |

**任何一项变了，meta 就变，hash 就变，缓存自动 miss** —— 零误命中风险，不需要用户手动声明哪些因素影响编译。

恢复缓存时还校验 `.meta` 文件内容是否被篡改（`sha256(meta文件内容) == buildhash`），保证缓存不可被静默污染。

### 7. 非侵入式 —— 业务代码不需要适配包管理器

业务代码不需要改 CMakeLists.txt 来适配 Celer。Celer 通过 toolchain file + 环境变量注入路径，`find_package` 自然找到依赖。业务 CMake 跟普通 CMake 项目一样写。

这意味着现有项目接入 Celer 不需要改造 CMake 结构 —— 设置 `CMAKE_TOOLCHAIN_FILE` 指向 Celer 生成的文件，现有构建流程立即工作。

### 8. 多级缓存体系 —— 从编译工具到构建产物的全覆盖

为了让 Celer 能用于真实项目开发，作为日常开发工具使用，Celer 构建了多级缓存体系：

| 缓存层级 | 说明 |
| --- | --- |
| **编译工具缓存** | cmake、msys2、ninja、git 等构建工具自动下载并缓存，国内用户无需手动配置 |
| **源码仓库缓存** | GitHub 等外网源码仓库的 clone 结果缓存到局域网，外网不稳定时不影响构建 |
| **编译产物缓存（pkgcache）** | 基于 meta hash 的编译产物缓存，支持 NFS 共享，团队级复用 |
| **开发者本地缓存（devcache）** | devDep/hostDep 的编译产物缓存到 `~/.celer`，跨 workspace 复用，不污染共享缓存 |

通过配置 pkgcache 达到局域网共享缓存，团队成员首次构建后，后续成员直接从局域网拉取缓存，免去重复编译和外部网络下载。

---

## 🚀 Celer 适合什么团队

- 需要频繁接入新 C/C++ 三方库以及内部开发公共基础库
- 管理多个平台/多个子工程的企业项目
- 对构建速度、可重现性和协作效率有硬要求
- 希望把依赖管理从"经验活"变成"工程化流程"

[开始使用 →](./quick_start.md) | [返回 README →](../../README.md)
