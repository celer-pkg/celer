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
- `--export-dag=<file>` 会把当前项目的构建 DAG 以 JSON 导出后直接结束：不克隆、不构建、不安装（见下文）。
- `--apply-dag=<file>` 反过来：不部署项目端口，而是按这份调度文件安装它点名的节点——工作区采用这份调度的身份、钉住的源码 revision 和 build hash（见下文）。
- `--export-dag` 与 `--apply-dag` 都要求工作区有 conf 仓库：导出要把 port 配置所属的 revision 记进调度文件，应用要把 conf 切回那个 revision。没有 conf 仓库时两者都会直接拒绝执行，请先用 `celer init --url=<conf repo>` 初始化。
- 部署前会一次性解析所有端口的 ref 到具体 commit，确保代码版本一致（见下文）。`--apply-dag` 会跳过这一步：revision 已经在调度文件里了。

## 命令选项

| 选项       | 简写 | 类型   | 默认值    | 说明                       |
|------------|------|-------|----------|----------------------------|
| --force    | -    | 布尔   | false   | 强制部署，忽略已安装状态      |
| --snapshot | -    | 字符串 | 空字符串 | 部署成功后导出工作区快照      |
| --strip    | -    | 布尔   | false   | 部署成功后执行与 `celer strip` 相同的运行时剥离（见 [strip](./cmd_strip.md)） |
| --export-dag | -  | 字符串 | 空字符串 | 不执行部署，改为把当前项目的构建 DAG 以 JSON 写到 `<file>` |
| --apply-dag | -   | 字符串 | 空字符串 | 不部署项目端口，改为按 `--export-dag` 导出的调度汇总：全部从 pkgcache 取，缺一个即失败 |

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
celer deploy --export-dag=dag.json

# 按调度汇总：只从 pkgcache 取，缺一个就报错
celer deploy --apply-dag=dag.json

# 附带导出可复现包（配置与 pin，不含编译产物）
celer deploy --apply-dag=dag.json --snapshot=snapshots/2026-02-21
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

`--export-dag=<file>` 把目标平台的构建调度写到指定文件，按 pkgcache build hash 索引，供外部编排器使用。图的根就是当前工作区的 project，因此入口节点是它 `celer.toml` 里声明的 ports；导出只写文件，不执行部署：

| 字段 | 说明 |
|------|------|
| `celer_version` / `platform` / `project` / `build_type` | 工作区身份；`platform` 同时也是构建 agent 的选择依据 |
| `conf_ref` | 导出所用 conf 仓库的精确 revision（commit，不是分支）。apply 与 `install --dag` 会先把工作区的 conf 移到它，让每台机器读到同一份 port 配置（配置本身也是 build hash 的一部分） |
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

把这个文件交给 `celer install --dag=<file> <node>` 即可按调度构建某个被调度的节点：agent 会改用调度里的 `platform`、`project` 和 `build_type`，而不是自己的 `celer.toml`（这样才会命中正确的 pkgcache 前缀），然后把每个源码 revision 钉在导出的 `checksum` 上，节点 build hash 与 DAG 不一致时直接报错。列在 `cached` 里的节点不参与调度，这样装会被拒绝。

## 应用调度（把产物收回来）

在需要拿到成品树的那台机器上执行——通常是 master，等所有 node job 跑完之后：

```shell
celer deploy --apply-dag=dag.json
```

它读 `celer deploy --export-dag=dag.json` 导出的调度文件，采用其中的工作区身份
（`platform`、`project`、`build_type`）和钉住的源码 revision，逐个安装调度里点名的
节点。结果是 `installed/` 下一棵正常的 celer 安装树，可以直接快照、打包或自己校验。

执行前需要知道几件事：

- **只从 pkgcache 取。** 某个节点不在里面时命令会停下并报
  `<node> is not available from pkgcache`——它不会自己把缺的节点编出来。这个报错说明
  分布式构建不完整（有 node job 失败或没上传产物），正确做法是修好构建再重跑，而不是
  在这台机器上补编。
- **它会把工作区的 `conf` 仓库切到调度记录的那个 revision**，保证这里读到的 port
  配置与导出时一致。conf 有本地改动请先提交或丢弃，命令不会替你丢。
- **dev/host 依赖会在这台机器上编**（调度里把它们列为 `local_nodes[]`，从不共享），
  所以这一步可能顺带编几个工具。
- 加 `--snapshot=<dir>` 会额外写一份可复现包（只有配置和 pin，不含产物），`--strip`
  会就地剥离收集到的树；产物本身只在 `installed/` 里。

调度文件必须与当前 celer 版本一致，否则命令会直接报错。
