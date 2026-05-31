# Deploy 导出快照（`deploy --snapshot`）

`deploy --snapshot` 会先执行正常部署，再导出可复现的工作区快照。

## 命令语法

```shell
celer deploy --snapshot=<snapshot_dir>
```

## 重要行为

- 只有部署成功后才会开始导出。
- 目标导出目录会先清空再重建。
- 快照会固定依赖源码校验值，用于复现。

## 导出内容

- `ports/`：项目实际使用端口，写入固定 checksum 和匹配构建配置
- `conf/`：工作区配置目录（不包含 `.git`）
- `celer.toml`
- `toolchain_file.cmake`
- `snapshot.md`
- 当前 `celer` 可执行文件

## checksum 采集规则

- Git URL（`*.git`）：读取本地源码仓库实际 commit 哈希作为 checksum。
- 私有仓库且指定了 `package.checksum`：使用该固定 checksum。
- 压缩包 URL（`.zip/.tar...`）：使用 `sha256:<checksum>` 作为 checksum。

## 常用示例

```shell
# 部署并导出快照
celer deploy --snapshot=snapshots/2026-02-21

# 强制部署并导出
celer deploy --force --snapshot=snapshots/rebuild
```

## 说明

- 导出依赖 `toolchain_file.cmake`（通常由成功部署生成）。
- 部署失败时不会执行导出。

## Snapshot 样例

```markdown
# Snapshot for project_test_01

## Build Environment

- exported_at: 2026-05-31T11:20:42.175732704+08:00
- celer_version: v0.0.0
- platform: aarch64-linux-ubuntu-22.04-gcc-11.5.0
- project: project_test_01

## Resolved commits

| Name@Version | Type | URL | Ref | Resolved |
|---|---|---|---|---|
| opencv@4.11.0 | git | https://github.com/opencv/opencv.git | 4.11.0 | 1d3b34ddd080bbf3e3d3cec58e11038fca21dcfe |
| ffmpeg@5.1.6 | archive | https://ffmpeg.org/releases/ffmpeg-5.1.6.tar.xz | 5.1.6 | - |
```
