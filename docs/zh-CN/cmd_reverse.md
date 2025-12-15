# Reverse 命令

`celer reverse` 命令提供反向依赖查找功能，允许您查找哪些包依赖于指定的库。

## 用法

```bash
celer reverse [包名@版本] [标志]
```

## 描述

reverse 命令搜索所有已安装的包，找出依赖于指定包的包。这对以下场景很有用：

- 理解项目中包的使用情况
- 在移除包之前进行影响分析
- 从底层向上的依赖树分析
- 找到特定库的所有使用者

## 示例

### 基本用法

查找所有依赖于 Eigen 的包：
```bash
celer reverse eigen@3.4.0
```

### 包含开发依赖

查找所有依赖于 NASM 的包（包括开发依赖）：
```bash
celer reverse nasm@2.16.03 --dev
```

## 标志

- `-d, --dev`: 在反向查找中包含开发依赖
- `-h, --help`: 显示 reverse 命令的帮助信息

## 输出格式

命令以以下格式显示结果：

```
[Reverse Dependencies]:
  package1@version1
  package2@version2
  package3@version3

total: 3 package(s)
```

如果没有找到反向依赖：
```
[Reverse Dependencies]:
no reverse dependencies found.
```

## 使用场景

1. **影响分析**：在移除包之前，检查哪些包依赖它
2. **重构**：了解哪些包使用了特定库
3. **安全评估**：查找受易受攻击依赖影响的所有包
4. **架构分析**：了解项目中的依赖关系

## 相关命令

- [`celer tree`](./cmd_tree.md) - 显示包的正向依赖
- [`celer search`](./cmd_search.md) - 搜索可用包
- [`celer remove`](./cmd_remove.md) - 移除包（在影响分析后使用）