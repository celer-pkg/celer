# 缓存源码仓库

> **通过 Repo Cache 避免重复 克隆 / 下载源码**

## 🎯 为什么需要 Repo Cache？

构建产物缓存解决的是“已经编译好的库能否复用”，而 **repo 缓存** 解决的是“源码能否直接复用”。

设计 repo cache 的一个核心原因，并不只是为了提速，而是为了应对**源码访问不稳定**的问题。在一些国家或企业网络环境里，访问 GitHub、GitLab 或其他源码托管服务经常受到限制，不能假设每个开发者、每台 CI 机器、每个合作方环境都具备稳定外网连接，更不能指望所有人都配置好了网络代理。

当一个项目依赖很多 git 仓库或源码压缩包时，即使最终仍然需要重新编译，重复的 clone、下载、解压也会浪费不少时间；而一旦外网受限，这些步骤甚至会直接失败。Celer 支持把源码树打包到 `pkgcache/repos` 中，在后续安装时优先恢复源码，而不是再次访问远端仓库或重新下载压缩包。

**适合的场景：**
- **GitHub 访问受限** - 团队成员并不都具备稳定访问外网源码站点的条件
- **不能要求每个人配置代理** - 希望通过共享缓存降低环境门槛
- **减少重复 clone / 下载** - 尤其适合大型仓库或网络较慢的环境
- **加速 CI / 本地重装** - 重新拉起源码目录时更快
- **应对上游短暂不可用** - 远端仓库或文件临时不可达时，仍可从本地缓存恢复
- **和构建产物缓存配合** - 先复用源码，再决定是否继续复用已编译产物

## 🔍 Repo Cache 和 Artifact Cache 的区别

| 能力 | Repo Cache | 构建产物缓存 |
|------|------------|--------------|
| 缓存内容 | 源码 | 已安装/已编译产物 |
| 生效阶段 | `Clone()` 阶段 | `Install()` 阶段 |
| 解决问题 | 避免重复 clone / 下载 | 避免重复 configure / build / install |
| 存储位置 | `pkgcache/repos` | `pkgcache/artifacts` |

简单理解：
- **Repo 缓存** 命中后，仍可能需要继续编译
- **Artifact 缓存** 命中后，通常意味着直接跳过编译过程，而走了模拟安装过程

## 💡 工作原理

在需要准备源码时，Celer 的流程如下：

1. 检查当前源码目录是否已存在且非空
2. 如果源码目录已经可用，直接复用，不再读 repo 缓存
3. 如果源码目录不存在，并且端口启用了 `package.cache_repo=true`，则先尝试从 `pkgcache/repos` 恢复源码
4. 如果缓存未命中，再执行正常的 git clone 或压缩包下载/解压
5. 当源码准备完成后，如果 `pkgcache.writable=true` 且当前不是 offline 模式，则把源码打包写入 repo 缓存

## 🚀 快速开始

### 步骤1：配置 pkgcache

在 `celer.toml` 中配置缓存目录：

```toml
[global]
	conf_repo = "https://github.com/celer-pkg/test-conf.git"
	platform = "x86_64-linux-ubuntu-22.04-gcc-11.5.0"
	project = "project_01"

[pkgcache]
	dir = "/home/test/pkgcache"
	writable = true
```

说明：
- `dir` 必须是一个已经存在的目录
- `writable=true` 时，Celer 才会把新的源码缓存写入 `pkgcache/repos`
- `writable=false` 时，仍然可以只读方式尝试恢复已有缓存

### 步骤2：在端口里启用 repo 缓存

在 `port.toml` 的 `[package]` 段里开启：

```toml
[package]
	url = "https://gitlab.com/libeigen/eigen.git"
	ref = "3.4.0"
	checksum = "31e19f92f00c7003fa115047ce50978bc98c3a0d"
	cache_repo = true

[[build_configs]]
	build_system = "cmake"
	options = ["-DEIGEN_TEST_NO_OPENGL=1", "-DBUILD_TESTING=OFF"]
```

**推荐做法：**
- **`checksum=[commit-hash/sha-256]`**：对于 git 仓库，建议固定为 git commit hash；对于压缩包，建议固定为文件的 `sha-256`。只有 commit hash 和 `sha-256` 能精确标识源码内容是否一致
- **`cache_repo=true`**：默认是 `false`，只有访问困难或希望通过共享缓存分发源码的端口才需要开启它。

这样 repo 缓存才能在新的工作空间里稳定命中。

## 🧭 两类源码的缓存键

### 1. Git 仓库

git 类型的源码缓存以 **实际 checkout 后的 commit hash** 作为缓存键。

目录示例：

```text
pkgcache/repos/x264@stable/31e19f92f00c7003fa115047ce50978bc98c3a0d.tar.gz
```

这意味着：
- 首次在线 clone 后，Celer 会把当前 commit 对应的源码树打包进 repo 缓存
- 后续如果再次安装时拿到的是同一个 commit hash，就可以直接从缓存恢复

> 💡 如果 `ref` 使用的是浮动分支或 tag，而不是固定 commit，那么首次 clone 后虽然会写入缓存，但后续在真正访问远端之前不一定能稳定命中该缓存。想稳定命中，建议把固定 commit hash 写入 `checksum` 字段。

### 2. 源码压缩包

压缩包类型的源码缓存以 **压缩包的 `sha-256`** 作为缓存键。

示例：

```toml
[package]
	url = "https://example.com/x264-20250101.tar.gz"
	ref = "20250101"
	checksum = "3147391d946bb4b6c68edd901f2add6ac1f31f8c"
	cache_repo = true
```

目录示例：

```text
pkgcache/repos/x264@stable/3147391d946bb4b6c68edd901f2add6ac1f31f8c.tar.gz
```

这类缓存适合的场景与 git 仓库类似，本质上都是为了在网络受限时仍然能稳定拿到源码。

## 🔄 实际行为细节

### 什么时候会尝试读取 repo 缓存？

满足以下条件时，Celer 会在 clone/download 之前先尝试读取 repo 缓存：

- 已配置 `pkgcache.dir`
- 当前端口配置了 `package.cache_repo=true`
- 当前源码目录不存在，或为空目录
- 当前包不是虚拟端口（`url != "_"`）
- 有可用于定位缓存的 `ref` 或 `checksum`

### 什么时候会写入 repo 缓存？

满足以下条件时，Celer 会把准备好的源码树写入 `pkgcache/repos`：

- 已配置 `pkgcache.dir`
- `pkgcache.writable=true`
- 当前端口配置了 `package.cache_repo=true`
- 当前不是 offline 模式
- clone / download / 解压已经成功完成

### 什么时候不会命中？

常见情况包括：

- 没有配置 `pkgcache`
- `pkgcache.dir` 不存在
- 端口没有设置 `package.cache_repo=true`
- 源码目录已经存在且非空，此时 Celer 会直接复用现有目录
- 请求的 commit / checksum 对应缓存不存在
- 开启了 offline 模式

## 📁 目录结构

repo 缓存在 `pkgcache/repos` 下按 `name@version` 分类：

```text
/home/test/pkgcache/
    └── repos
        ├── x264@stable
        │   ├── 31e19f92f00c7003fa115047ce50978bc98c3a0d.tar.gz
        │   └── 3147391d946bb4b6c68edd901f2add6ac1f31f8c.tar.gz
        ├── ffmpeg@6.1.1
        │   └── 1f2e3d4c....tar.gz
        └── opencv@4.10.0
            └── aabbccdd....tar.gz
```

说明：
- 第一层是固定的 `repos`
- 第二层是库名和版本，例如 `x264@stable`
- 第三层是缓存键命名的 `.tar.gz` 文件

## 🧩 和构建产物缓存如何配合

一次典型安装中，Celer 可能按下面顺序工作：

1. 先尝试从 **repo 缓存** 恢复源码
2. 再尝试从 **构建产物缓存** 恢复已编译结果
3. 如果构建产物缓存未命中，则继续正常构建
4. 构建成功后，分别视条件写回 repo 缓存和构建产物缓存

因此两者并不冲突，而是互补：
- repo 缓存解决“源码从哪里来”
- 构建产物缓存解决“编译结果能不能直接复用”

## ⚠️ 当前注意事项

- **Repo Cache 不是离线源替代品**：当前实现里，`offline=true` 时不会读写 repo 缓存。
- **Repo Cache 不包含最终安装结果**：命中 repo 缓存并不代表能跳过编译。
- **已有源码目录优先级更高**：如果 `buildtrees/.../src` 已经存在且非空，Celer 直接复用，不会再尝试恢复 repo 缓存。
- **建议锁定源码版本**：想跨工作空间稳定命中 repo 缓存，最好使用固定 `commit hash` 或固定 `sha-256`，而不是浮动 branch 或 tag。

## ✅ 建议配置

如果你的项目同时支持 repo 缓存和构建产物缓存，推荐这样使用：

- 在 `celer.toml` 中统一配置共享的 `pkgcache.dir`
- 在网络较差或访问 GitHub 受限的团队环境里，把 `pkgcache.dir` 放到局域网共享目录
- 对访问有困难的 port，在其 `port.toml` 里开启 `package.cache_repo=true`
- 对稳定版本的 git 依赖使用固定 `commit hash`
- 对压缩包源码提供明确的 `sha-256` 作为 `checksum`
- 对可复用的构建结果继续启用构建产物缓存

这样可以同时减少：
- 拉源码的时间
- 重复编译的时间
