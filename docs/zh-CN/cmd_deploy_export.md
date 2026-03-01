# Deploy 导出（`deploy --export`）

`deploy --export` 会先执行正常部署，再导出可复现的工作区快照。

## 命令语法

```shell
celer deploy --export=<export_dir>
```

## 重要行为

- 只有部署成功后才会开始导出。
- 目标导出目录会先清空再重建。
- 快照会固定依赖提交信息，用于复现。

## 导出内容

- `ports/`：项目实际使用端口，写入固定 commit/ref 和匹配构建配置
- `conf/`：工作区配置目录（不包含 `.git`）
- `celer.toml`
- `toolchain_file.cmake`
- `snapshot.json`
- 当前 `celer` 可执行文件

## commit 采集规则

- Git URL（`*.git`）：读取本地源码仓库实际 commit。
- 私有仓库且指定了 `package.commit`：使用该固定 commit。
- 压缩包 URL（`.zip/.tar...`）：使用 `sha-256:<checksum>` 作为 commit。

## 常用示例

```shell
# 部署并导出快照
celer deploy --export=snapshots/2026-02-21

# 强制部署并导出
celer deploy --force --export=snapshots/rebuild
```

## 说明

- 导出依赖 `toolchain_file.cmake`（通常由成功部署生成）。
- 部署失败时不会执行导出。

## Snapshot 样例

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
