# 缓存构建产物

> **通过智能制品缓存节省数小时的构建时间**

## 为什么需要缓存？

从源码编译 C/C++ 库非常耗时，会严重影响开发效率。一个包含 20+ 依赖的典型项目首次构建可能需要 **2-3 小时**。Celer 的智能缓存系统可以将后续构建缩短到**几分钟**。

**核心优势：**
- **显著加快构建** - 重用已编译的制品而非重新构建
- **团队协作** - 通过网络文件夹在团队间共享构建制品
- **私有库** - 分发预编译二进制文件而不暴露源代码
- **精确失效** - 任何依赖或配置变化时自动缓存失效

## 工作原理

Celer 使用**基于哈希的缓存**来存储和检索构建制品：

1. **计算哈希**：根据构建环境、选项、依赖和补丁生成唯一哈希
2. **检查缓存**：在配置的缓存目录中搜索匹配的制品
3. **使用或构建**：如果找到，提取并使用；如果没找到，从源码构建
4. **存储结果**：在满足条件时自动保存新的构建制品供未来使用

## 快速开始

### 步骤1：配置缓存位置

在 `celer.toml` 中添加 `[pkgcache]` 部分以启用缓存检索：

```toml
[global]
	conf_repo = "https://github.com/celer-pkg/test-conf.git"
	platform = "x86_64-linux-ubuntu-22.04-gcc-11.5.0.5"
	project = "project_01"
	jobs = 32

[pkgcache]
	dir = "/home/test/pkgcache"  # 本地或网络挂载目录
	writable = false             # 只读缓存（默认），只有为true时候才会在编译过程中写入缓存
```

**现在会发生什么：**
- Celer 在构建前搜索缓存的制品
- 如果找到匹配项，则立即提取并使用
- 如果未找到，则从源码构建（和平常一样）

> **提示**：使用网络挂载文件夹（如 NFS、SMB、FTP）在团队间共享缓存

### 步骤2：自动存储构建制品

```toml
[package]
	url = "https://gitlab.com/libeigen/eigen.git"
	ref = "3.4.0"

[[build_configs]]
	build_system = "cmake"
	options = ["-DEIGEN_TEST_NO_OPENGL=1", "-DBUILD_TESTING=OFF"]
```

**使用方法：**

```bash
celer install eigen@3.4.0
```

**会发生什么：**
1. 库从源码构建
2. 构建制品被打包成 `.tar.gz` 文件
3. 生成基于哈希的文件名, 哈希的计算源于**繁多的构建配置**
4. 制品存储到缓存目录
5. 创建元数据（`.meta` 文件）到 package 和 install 目录，用于跟踪，内部记录当前库安装了哪些文件

**自动跳过写缓存的常见情况：**
- `pkgcache` 没有配置
- `pkgcache.dir` 没有配置
- `pkgcache.writable=false` 配置了只读
- 源码仓库在构建前已有人为本地修改

**自动寻找匹配的存储制品的过程**
- 判断`pkgcache`和`pkgcache.dir`是否配置，如果没有配置则放弃寻找
- 检查当前仓库是否已经被修改，如果有修改则放弃寻找
- 读取当前仓库的git commit hash, 并根据当前的构建配置生成哈希
- 带着哈希以及构建配置参数去pkgcache/artifacts目录寻找匹配的制品存储
- 找到匹配的制品压缩包则直接模拟走编译成功后的安装过程

## 私有库分发

### 分发预编译二进制文件而不暴露源代码

对于私有或专有库，您可以在不暴露源代码的情况下分发预编译制品。通过在 `checksum` 中指定 git commit 哈希，Celer 可以直接从缓存中检索构建制品：

```toml
[package]
	url = "https://gitlab.com/libeigen/eigen.git"
	ref = "3.4.0"
	checksum = "3147391d946bb4b6c68edd901f2add6ac1f31f8c" # 赋值checksum则启用支持制品缓存

[[build_configs]]
	build_system = "cmake"
	options = ["-DEIGEN_TEST_NO_OPENGL=1", "-DBUILD_TESTING=OFF"]
```

**工作原理：**
1. Celer 使用 port.toml 中配置的`checksum` 和`众多的构建配置`计算缓存键
2. 在 `pkgcache.artifacts` 中搜索匹配的制品
3. 如果找到，提取并使用，**无需克隆仓库**
4. 如果未找到，则回退到从源码构建（如果可访问）

**使用场景：**
- 向合作伙伴分发内部库而不暴露源代码
- 通过跳过 git 克隆加速 CI/CD 流水线
- 控制专有代码访问同时共享二进制文件

## 缓存目录结构

Celer 以层次结构组织缓存制品，便于管理：

```
/home/test/pkgcache/
    └── artifacts
        └── x86_64-linux-ubuntu-22.04-gcc-11.5.0/     # 平台
            └── project_01/                           # 项目
                └── release/                          # 构建类型（release/debug）
                    ├── ffmpeg@3.4.13/                # 库名@版本
                    │   ├── d536728...09068.tar.gz    # 构建制品（压缩）
                    │   ├── f466728...a0906.tar.gz    # 不同配置变体
                    │   └── meta/                     # 元数据目录
                    │       ├── d536728...09068.meta  # 哈希键 + 构建信息
                    │       └── f466728...a0906.meta
                    │
                    ├── opencv@4.5.1/
                    │   ├── li98343...39a8.tar.gz
                    │   ├── 4324324...sfdf.tar.gz
                    │   └── meta/
                    │       ├── li98343...39a8.meta
                    │       └── 4324324...sfdf.meta
                    └── ...
```

**目录结构说明：**
- **平台级**：按目标平台和工具链分离制品
- **项目级**：隔离不同项目以防止冲突
- **构建类型**：分离 debug、release 等构建
- **库文件夹**：每个库一个文件夹，带版本号
- **制品**：哈希命名的 `.tar.gz` 文件，包含已构建的库
- **元数据**：`.meta` 文件存储哈希键和构建配置

## 缓存键工作原理

Celer 为每个构建配置生成一个**唯一哈希**。此哈希充当缓存键，确保仅在构建真正相同时才重用制品。

### 缓存键组成部分

哈希由多个因素计算得出：

#### 1. 构建环境
- **工具链**：URL、路径、名称、版本、系统架构
- **系统根目录**：URL、路径、`pkg_config_path`
- **编译器**：GCC/Clang 版本、目标三元组

#### 2. 构建参数
- **库选项**：例如 FFmpeg 的 `--enable-cross-compile`、`--enable-shared`、`--with-x264`
- **环境变量**：`CFLAGS`、`LDFLAGS`、`CXXFLAGS`
- **构建类型**：Debug vs Release，静态 vs 共享

#### 3. 源码修改
- **应用的补丁**：补丁文件内容被哈希化
- **源码校验值**：Git commit 哈希或归档文件 checksum

#### 4. 依赖图
- **递归依赖哈希**：所有依赖项的哈希（x264、nasm 等）
- **依赖配置**：它们的构建选项、版本和补丁

### 自动缓存失效

**对这些因素的任何更改都会触发新哈希：**

```
旧配置：FFmpeg + x264 1.0 + --enable-shared  → 哈希：abc123...
新配置：FFmpeg + x264 2.0 + --enable-shared  → 哈希：def456...  (x264 版本变化)
```

当哈希更改时：
1. 旧缓存不被使用（它用于不同的配置）
2. 库从源码重新构建
3. 新制品以新哈希存储

> **结果**：您永远不会使用过时或不兼容的缓存制品！