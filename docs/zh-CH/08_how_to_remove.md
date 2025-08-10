# 如何删除已安装的库

## 概览

celer remove 命令用于从系统中卸载指定的软件包。它提供了灵活的删除选项，包括依赖项清理、构建缓存删除和开发模式软件包处理。

## 命令语法

```shell
celer remove [package_name] [flags]  
```

## 命令标志

| 命令标志 | 简写 | 描述 |
| ---- | --------- | ----------- |
| --build-type | -b | 卸载指定构建类型的软件包（默认："release"）。 |
| --dev | -d | 卸载开发模式软件包（用于开发依赖项）。 |
| --purge | -f | 激进删除：删除软件包及其所有关联文件（例如配置、数据）。 |
| --recurse | -r | 递归删除：卸载软件包及其依赖项（如果没有其他软件包依赖它们）。 |
| --remove-cache | -c | 清理构建缓存：删除软件包的缓存构建工件。 |

## 使用示例

1. 基本删除

```shell
celer remove ffmpeg@5.1.6
```

2. 递归删除

```shell
celer remove ffmpeg@5.1.6 --recurse/-r
```

3. 激进删除

```shell
celer remove ffmpeg@5.1.6 --purge/-p
```

4. 开发模式删除

```shell
celer remove ffmpeg@5.1.6 --dev/-d
```

5. 缓存删除

```shell
celer remove ffmpeg@5.1.6 --remove-cache/-c
```

>Notes
>1. 递归删除 (-r) 谨慎使用，因为它可能会破坏其他软件包，如果共享依赖项被删除。
>2. 激进删除 (-p) 不可逆；确保您不需要软件包的缓存。
>3. 缓存删除 (-c) 释放磁盘空间，但可能会减慢未来的编译速度。
