# Update 命令

&emsp;&emsp;Celer 更新命令同步本地存储库与其远程副本，确保您具有最新的软件包配置和构建定义。它支持针对不同存储库类型的目标更新。

## 命令语法

```shell
celer update [flags]
```

## 命令选项

| 选项	             | 短选项     | 描述                                                                                         |
| ----------------- | ---------- | ---------------------------------------------------------------------------------------------|
| --conf-repo	    | -c	     | Update only the workspace conf repository.                                                   |
| --ports-repo      | -p         | Update only the ports repository.                                                            |
| --force	        | -f	     | Combine with --conf-repo or --ports-repo to force update the repository.                     |
| --recurse         | -r         | Combine with --conf-repo or --ports-repo to recursively update all dependencies of a package.|

## 用法示例

### 1. 更新 conf 仓库

```shell
celer update --conf-repo/-c
```

### 2. 更新三方库配置托管仓库

```shell
celer update --ports-repo/-p
```

### 3. 更新三方库的源码仓库

```shell
celer update ffmpeg@3.4.13
```

### 4. 使用 --force 和 --recurse 组合更新

```shell
celer update --force/-f --recurse/-r ffmpeg@3.4.13
```

> **Note:**  
> 组合使用 --force 和 --recurse 标志可以强制更新软件包及其递归依赖项。
