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
- `--dag=<file>` 会把当前项目的构建 DAG 以 JSON 导出后直接结束：不克隆、不构建、不安装（见下文）。
- 部署前会一次性解析所有端口的 ref 到具体 commit，确保代码版本一致（见下文）。

## 命令选项

| 选项       | 简写 | 类型   | 默认值    | 说明                       |
|------------|------|-------|----------|----------------------------|
| --force    | -    | 布尔   | false   | 强制部署，忽略已安装状态      |
| --snapshot | -    | 字符串 | 空字符串 | 部署成功后导出工作区快照      |
| --strip    | -    | 布尔   | false   | 部署成功后执行与 `celer strip` 相同的运行时剥离（见 [strip](./cmd_strip.md)） |
| --dag      | -    | 字符串 | 空字符串 | 不执行部署，改为把当前项目的构建 DAG 以 JSON 写到 `<file>` |

## 常用示例

```shell
# 普通部署
celer deploy

# 强制部署
celer deploy --force

# 部署并导出快照
celer deploy --snapshot=snapshots/2026-02-21

# 部署并生成运行时 stripped 树（同 celer strip）
celer deploy --strip

# 强制部署并导出, 并 strip
celer deploy --force --snapshot=snapshots/rebuild --strip

# 只导出当前项目的构建 DAG，不部署
celer deploy --dag=dag.json
```

## 说明

- 运行前请先完成平台与项目配置。
- 如果部署失败，不会执行导出。
- `--strip` 与独立命令 [`celer strip`](./cmd_strip.md) 共用同一套逻辑，结果写到 `workspace/stripped/`，不修改 `installed/`。
- 部署成功后可在 CMake 中通过 `-DCMAKE_TOOLCHAIN_FILE=...` 使用 `toolchain_file.cmake`。

## 预解析 Ref 机制

`deploy` 在克隆代码前，一次性将所有端口的 ref（分支名、标签名等）解析为 commit hash，再统一克隆。解析结果保存为 `snapshot.md`，位于 `<workspace>/installed/celer/deployments/`。

这样做的目的是避免逐个克隆时远程推送导致同一分支被解析到不同 commit，保证整次部署基于一致的代码快照。

- **新克隆**：`git clone --branch <ref>` + `git reset --hard <commit>`，保留分支名。
- **已有仓库**：直接 `git reset --hard <commit>`。

## JSON 输出（构建 DAG）

`--dag=<file>` 把目标平台的构建计划写到指定文件，按 pkgcache build hash 索引，供外部编排器使用。图的根就是当前工作区的 project，因此入口节点是它 `celer.toml` 里声明的 ports；导出只写文件，不执行部署：

| 字段 | 说明 |
|------|------|
| `celer_version` / `platform` / `project` / `build_type` | 工作区身份；`platform` 同时也是构建 agent 的选择依据 |
| `scheduled_nodes[]` | 需要 agent 去构建的节点（没有需要构建的节点时该键不输出） |
| `scheduled_nodes[].name_version` | `name@version` |
| `scheduled_nodes[].build_hash` | pkgcache 键：`artifacts-<celer_version>/.../<build_hash>.tar.gz` |
| `scheduled_nodes[].checksum` | 导出时解析好的源码 revision（git commit 或压缩包 sha256） |
| `scheduled_nodes[].dependencies` | 必须先装好的依赖：要么前面已构建，要么直接从 pkgcache 恢复 |
| `local_nodes[]` | 每个 agent 自己编的 dev/host 依赖：不参与调度，只用来钉源码 revision |
| `local_nodes[].name_version` / `local_nodes[].checksum` | 包名与它被钉住的源码 revision |
| `cached` | 构件已在 pkgcache 里的节点（`name@version` 列表）：不参与调度，由用到它的 job 恢复 |

导出会逐节点查 pkgcache，把闭包拆成 `scheduled_nodes` 与 `cached`；`scheduled_nodes` 为空时该键不输出，也就完全不需要构建。每个节点查两次（它的 metadata 和归档），所以只读的 pkgcache 就够了；cache 不可达时导出直接失败，不做推测。未配置 pkgcache 或处于 offline 模式时，所有节点一律进 `scheduled_nodes`。

dev/host 依赖**不参与调度**，也**不做 cache 判定**：它们的缓存键里含 workspace 路径，无法跨 agent 复用。每个 agent 本地编一次，之后靠自己的 `devcache` 复用。

把这个文件交给 `celer install --dag=<file> <node>` 即可按计划构建某个被调度的节点：agent 会改用计划里的 `platform`、`project` 和 `build_type`，而不是自己的 `celer.toml`（这样才会命中正确的 pkgcache 前缀），然后把每个源码 revision 钉在导出的 `checksum` 上，节点 build hash 与 DAG 不一致时直接报错。列在 `cached` 里的节点不参与调度，这样装会被拒绝。
