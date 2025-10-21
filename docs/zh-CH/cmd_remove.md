# Remove 命令

&emsp;&emsp;Celer 移除命令从系统中卸载指定的软件包。它提供了灵活的移除选项，包括依赖项清理、构建缓存删除和开发模式软件包处理。

## 命令语法

```shell
celer remove [name@version] [flags]  
```

## 标志

| Flag              | Shorthand | Description                                               |
| ----------------- | --------- | --------------------------------------------------------- |
| --dev             | -d        | uninstall package for dev mode.                           |
| --purge           | -f        | uninstall package along with its package files.           |
| --recurse         | -r        | uninstall package along with its depedencies.             |
| --build-cache     | -c        | uninstall package along with build cache.                 |

## 用法示例

### 1. 基本移除

```shell
celer remove ffmpeg@5.1.6
```

### 2. 移除安装的库，同时删除已安装的依赖库

```shell
celer remove ffmpeg@5.1.6 --recurse/-r
```

### 3. 删除安装的库，同时包含它的package目录

```shell
celer remove ffmpeg@5.1.6 --purge/-p
```

### 4. 移除开发模式软件包

```shell
celer remove ffmpeg@5.1.6 --dev/-d
```

### 5. 移除软件包并清理构建缓存

```shell
celer remove ffmpeg@5.1.6 --build-cache/-c
```