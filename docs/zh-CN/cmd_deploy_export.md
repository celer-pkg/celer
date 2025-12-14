# 📦 Deploy 导出功能

## 概述

`deploy --export` 命令将部署和工作区快照导出合二为一, 在成功构建并安装所有项目依赖后，导出可复现的工作区快照。

## 用法

```bash
celer deploy --export=<导出目录>
```

## 导出内容

1. **ports/**: 项目使用的所有端口配置文件
2. **conf/**: 配置目录（平台、项目）
3. **celer.toml**: 工作区配置
4. **toolchain_file.cmake**: CMake 工具链文件
5. **snapshot.json**: 依赖快照，包含实际的 git commit
6. **celer**: 当前在使用的celer可执行文件

## 核心特性

### 实际 Git Commit
与独立的 `celer export` 不同，deploy 命令导出的是 `buildtrees/` 中克隆仓库的**实际 git commit hash**，而不仅仅是 port.toml 中的 ref。

对于每个基于 git 的依赖，snapshot 包含：
- 来自 `git rev-parse HEAD` 的精确 commit hash
- 在成功构建后捕获
- 保证可复现性

### 仅在成功后导出
导出仅在部署成功后进行。这确保：
- 所有依赖都成功构建
- 源代码正确克隆
- Commit hash 来自已验证的构建

## 示例

```bash
# 部署项目并导出快照
celer deploy --export=snapshots/2025-12-14

# 检查 snapshot
cat snapshots/2025-12-14/snapshot.json
```

### Snapshot 输出示例

```json
{
  "exported_at": "2025-12-14T16:51:10.290199679+08:00",
  "celer_version": "0.1.0",
  "platform": "aarch64-linux-ubuntu-22.04-gcc-11.5.0",
  "project": "project_test_01",
  "dependencies": [
    {
      "name": "opencv",
      "version": "4.11.0",
      "commit": "0e5254ebf54d2aed6e7eaf6660bf3b797cf50a02",
      "url": "https://github.com/opencv/opencv.git"
    }
  ]
}
```

## 工作流程

1. **构建**: Deploy 从头构建所有依赖
2. **克隆**: Git 仓库克隆到 `buildtrees/{name}@{version}/src/`
3. **编译**: 每个依赖被编译和安装
4. **快照**: 如果成功，导出工作区到指定目录
5. **Commit 捕获**: 从每个 git 仓库读取实际 commit

## 使用场景

### CI/CD 可复现性
```bash
# 在 CI 中构建并生成快照
celer deploy --export=build-artifacts/snapshot

# 归档和分享
tar -czf build-snapshot.tar.gz build-artifacts/snapshot
```

### 版本锁定
```bash
# 锁定当前工作版本
celer deploy --export=snapshots/working-$(date +%Y%m%d)

# 之后需要时恢复
cd snapshots/working-20251214 && ./celer deploy
```

## 注意事项

- 导出目录不存在时会自动创建
- 现有导出目录会被覆盖
- 归档下载（.zip, .tar.gz）使用配置的 ref 作为 commit
- 只有 git 仓库有实际的 commit hash
