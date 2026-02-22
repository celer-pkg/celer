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
- `--export=<path>` 仅在部署成功后触发快照导出。
- `--export` 必须是工作区内的非空相对路径。
- `--export` 不能指向受保护的工作区根目录（`conf`、`ports`、`buildtrees`、`packages`、`installed`、`tmp`、`.git`）。

## 命令选项

| 选项     | 简写 | 类型   | 说明                         |
|----------|------|--------|----------------------------|
| --force  | -f   | 布尔   | 强制部署，忽略已安装状态      |
| --export | -    | 字符串 | 部署成功后导出工作区快照      |

## 常用示例

```shell
# 普通部署
celer deploy

# 强制部署
celer deploy --force

# 部署并导出快照
celer deploy --export=snapshots/2026-02-21

# 强制部署并导出
celer deploy --force --export=snapshots/rebuild
```

## 说明

- 运行前请先完成平台与项目配置。
- 如果部署失败，不会执行导出。
- 部署成功后可在 CMake 中通过 `-DCMAKE_TOOLCHAIN_FILE=...` 使用 `toolchain_file.cmake`。
