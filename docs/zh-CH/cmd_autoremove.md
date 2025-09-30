# Autoremove 命令

&emsp;&emsp;**Autoremove**命令能帮忙移除当前项目已安装但不需要的库文件，这有助于清理未使用的依赖项并确保有刚需依赖库的开发环境。

## 命令语法

```shell
celer autoremove [flags]
```

## 命令选项

| Option	        | Short flag | Description                                              	|
| ----------------- | ---------- | ------------------------------------------------------------ |
| --purge           | -p         | autoremove packages along with its package file.             |
| --remove‑cache	| -c	     | autoremove packages along with build cache.  	            |

## 命令示例

**1. 移除当前项目不需要的库文件**

```shell
celer autoremove  
```

**2. 移除当前项目不需要的库文件，同时删除对应的package目录**

```shell
celer autoremove --purge/-p
```

**3. 移除当前项目不需要的库文件，同时删除对应的package目录，以及它们的构建缓存**

```shell
celer autoremove --purge/-p --remove-cache/-c  
```