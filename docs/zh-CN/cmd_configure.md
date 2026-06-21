# Configure 命令

&emsp;&emsp;`configure` 命令用于修改当前工作空间的全局配置。

## 命令语法

```shell
celer configure [flags]
```

## 重要行为

- 单次命令只能修改一个配置组。
- 混用不同组的 flag 会报错。
- 只有同一组内的多个 flag 才能在一条命令里一起配置（例如 pkgcache、proxy、ccache）。

## 命令选项
| 选项                       | 类型    | 说明                                  |
|----------------------------|---------|--------------------------------------|
| --platform                 | 字符串  | 设置目标平台                           |
| --project                  | 字符串  | 设置当前项目                           |
| --build-type               | 字符串  | 设置构建类型                           |
| --downloads                | 字符串  | 设置下载目录                           |
| --jobs                     | 整数    | 设置并行构建任务数                     |
| --offline                  | 布尔    | 开启/关闭离线模式                      |
| --verbose                  | 布尔    | 开启/关闭详细日志模式                   |
| --proxy-host               | 字符串  | 设置代理地址                           |
| --proxy-port               | 整数    | 设置代理端口                           |
| --pkgcache-dir             | 字符串  | 设置 pkgcache 目录                     |
| --pkgcache-writable        | 布尔    | 设置 pkgcache 是否可写                  |
| --pkgcache-cache-artifacts | 布尔    | 是否将构建产物缓存到 pkgcache           |
| --pkgcache-cache-downloads | 布尔    | 是否将下载源码缓存到 pkgcache           |
| --ccache-enabled           | 布尔    | 开启/关闭 ccache                       |
| --ccache-dir               | 字符串  | 设置 ccache 工作目录                   |
| --ccache-maxsize           | 字符串  | 设置 ccache 最大容量                   |
| --ccache-remote-storage    | 字符串  | 设置 ccache 远端存储 URL               |
| --ccache-remote-only       | 布尔    | 开启/关闭仅远端缓存模式                 |
| --port                     | 字符串  | 要更新的 port，格式为 `name@version`    |
| --port-url                 | 字符串  | port 的新源 URL（需配合 `--port`）      |
| --port-ref                 | 字符串  | port 的新 ref：分支/标签/commit（需配合 `--port`）|

## 常用示例

```shell
# 平台 / 项目
celer configure --platform=x86_64-linux-ubuntu-22.04-gcc-11.5.0
celer configure --project=project_test_02

# 构建配置
celer configure --build-type=Release
celer configure --downloads=/home/xxx/Downloads
celer configure --jobs=8

# 运行时开关
celer configure --offline=true
celer configure --verbose=false

# pkgcache 组（可同命令组合）
celer configure --pkgcache-dir=/home/xxx/cache --pkgcache-writable=true
celer configure --pkgcache-cache-artifacts=true
celer configure --pkgcache-cache-downloads=true

# proxy 组（可同命令组合）
celer configure --proxy-host=127.0.0.1 --proxy-port=7890

# ccache 组（可同命令组合）
celer configure --ccache-enabled=true --ccache-maxsize=5G --ccache-remote-only=true
celer configure --ccache-remote-storage=http://server:8080/ccache

# port 组（更新 port 的 url/ref，需配合 --port 一起使用）
celer configure --port=eigen@3.4.0 --port-ref=3.4.1
celer configure --port=eigen@3.4.0 --port-url=https://example.com/eigen.git --port-ref=main
```

## 参数校验规则

- `--platform`：需对应 `conf/platforms` 下的 TOML 文件名。
- `--project`：需对应 `conf/projects` 下的 TOML 文件名。
- `--build-type`：支持 `Release`、`Debug`、`RelWithDebInfo`、`MinSizeRel`（保存时转为小写）。
- `--downloads`：目录必须已存在。
- `--jobs`：必须大于 `0`。
- `--pkgcache-dir`：不能为空，且目录必须已存在。
- `--pkgcache-writable`：布尔值；使用前需先配置 `--pkgcache-dir`（可同命令一起配置）。
- `--pkgcache-cache-artifacts` / `--pkgcache-cache-downloads`：布尔值；使用前需先配置 `--pkgcache-dir`。
- `--proxy-host`：不能为空。
- `--proxy-port`：必须大于 `0`。
- `--ccache-dir`：目录必须已存在。
- `--ccache-maxsize`：必须以 `M` 或 `G` 结尾（例如 `512M`、`5G`）。
- `--ccache-remote-storage`：允许为空（用于清空配置）；非空时必须是包含 scheme 和 host 的合法 URL，例如 `http://server:8080/ccache`。
- `--ccache-remote-only`：布尔值（`true` 或 `false`）。
- `--port`：必须为 `name@version` 形式，且对应的 port 已存在；与 `--port-url` 或 `--port-ref` 至少使用其一。
- `--port-url` / `--port-ref`：必须与 `--port` 同时提供，单独使用会报错。
