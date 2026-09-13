# Strip 命令

`strip` 从当前平台的 **installed** 树生成一份面向运行时交付的目录，写入 `workspace/stripped/<platform>/<project>/<buildType>/`。

它会处理 **ELF**（Linux 等）与 **PE**（Windows `.exe` / `.dll`）可执行文件和共享库：用 `toolchain.strip` 去除调试符号；并保留运行所需的配置、shell 脚本等文件。头文件、静态库、CMake / pkg-config、PDB 等开发用产物不会进入该目录。`installed/` 本身不会被修改。

## 命令语法

```shell
celer strip
```

无额外参数与选项。使用当前工作区上下文（`platform`、`project`、`build_type`）。

## 前置条件

- 已完成 `configure`，且目标 `installed` 目录中已有安装产物。
- 平台必须配置 `toolchain.strip`，且工具可在工具链目录下找到：
  - Linux：如 `x86_64-linux-gnu-strip`
  - Windows：如 `llvm-strip.exe`（需能处理 PE）

未配置 `strip` 时命令直接失败，不会退化为仅 copy。

详见 [平台配置](./article_platform.md)。

## 输出内容

| 处理方式 | 内容 |
|----------|------|
| **strip** | ELF / PE 可执行文件与共享库 |
| **copy** | `.so` 版本符号链接、shell / 配置与其它运行时数据 |
| **跳过** | 静态库（`.a` / `.lib`）、头文件、`.cmake` / `.pc`、`.pdb`、`.exp` / `.ilk`、目标文件等 |
| **跳过目录** | `include`、`cmake`、`pkgconfig`、`man`、`doc`、`info`、`aclocal` |

日志示例：

```text
[✔] [copy ] lib/libffi.so
[✔] [copy ] lib/libffi.so.8
[✔] [strip] lib/libffi.so.8.1.4
```

Windows 示例：

```text
[✔] [strip] bin/app.exe
[✔] [strip] bin/foo.dll
[✔] [copy ] etc/app.conf
```

## 常用示例

```shell
# 安装或部署完成后再生成运行时树
celer install glog@0.6.0
celer strip

# 部署成功后自动执行同等逻辑
celer deploy --strip
```

## 说明

- 输出路径：`stripped/<platform>/<project>/<buildType>/`（与 `installed` 的 library folder 对齐）。
- `deploy --strip` 与 `celer strip` 共用同一套实现；单独执行 `strip` 适合在已安装树上重复生成运行时产物。
- 未配置 `toolchain.strip`，或配置了但找不到工具时，都会报错。
