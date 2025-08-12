# Autoremove 命令

&emsp;&emsp;**Autoremove**命令移除当前项目不再需要的已安装软件包。这有助于清理未使用的依赖项并维护整洁的开发环境。

## 命令语法

```shell
celer autoremove [flags]  
```

## 命令选项

| Option	        | Short flag | Description                                              	|
| ----------------- | ---------- | ------------------------------------------------------------ |
| --purge           | -p         | Remove packages along with associated package files.         |
| --remove‑cache	| -c	     | Remove packages along with their build cache.	            |

## 命令示例

**1. 标准自动移除不需要的库**

```shell
celer autoremove  
```

**2. 自动移除不需要的库，以及它们的软件包**

```shell
celer autoremove --purge/-p
```

**3. 自动移除不需要的库，以及它们的构建缓存**

```shell
celer autoremove --purge/-p --remove-cache/-c  
```

>**Autoremove**命令对于优化磁盘空间和保持项目环境干净非常有用，因为它可以移除不必要的依赖项。
