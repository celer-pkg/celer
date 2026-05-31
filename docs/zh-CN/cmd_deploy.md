# Deploy 命令

`deploy` 命令会构建并安装当前项目定义的全部端口依赖。

## 命令语法

```shell
celer deploy [flags]
```

## 重要行为

- 执行部署前会检查项目端口的循环依赖和版本冲突。
- 部署使用当前工作空间上下文（`platform`、`project`、`build_type`）。
- `--force` 会以强制模式执行项目部署（重装逻辑）。
- `--snapshot=<path>` 仅在部署成功后触发快照导出。
- `--snapshot` 支持相对路径和绝对路径。
- `--snapshot` 不能为空路径。
- 部署前会一次性解析所有端口的 ref 到具体 commit，确保代码版本一致（见下文）。

## 命令选项

| 选项     | 简写 | 类型   | 说明                         |
|----------|------|--------|----------------------------|
| --force  | -f   | 布尔   | 强制部署，忽略已安装状态      |
| --snapshot | -    | 字符串 | 部署成功后导出工作区快照      |

## 常用示例

```shell
# 普通部署
celer deploy

# 强制部署
celer deploy --force

# 部署并导出快照
celer deploy --snapshot=snapshots/2026-02-21

# 强制部署并导出
celer deploy --force --snapshot=snapshots/rebuild
```

## 说明

- 运行前请先完成平台与项目配置。
- 如果部署失败，不会执行导出。
- 部署成功后可在 CMake 中通过 `-DCMAKE_TOOLCHAIN_FILE=...` 使用 `toolchain_file.cmake`。

## 预解析 Ref 机制

`deploy` 在克隆代码前，一次性将所有端口的 ref（分支名、标签名等）解析为 commit hash，再统一克隆。解析结果保存在 `<workspace>/deploy-refs/`。

这样做的目的是避免逐个克隆时远程推送导致同一分支被解析到不同 commit，保证整次部署基于一致的代码快照。

- **新克隆**：`git clone --branch <ref>` + `git reset --hard <commit>`，保留分支名。
- **已有仓库**：直接 `git reset --hard <commit>`。
