# Clean 命令

&emsp;&emsp;**Clean**命令移除软件包或项目的构建缓存文件，帮助释放磁盘空间或解决由于过时缓存数据导致的构建问题。

## 命令语法

```shell
celer clean [flags]
```

## 命令选项

| Option	        | Short flag | Description                                          |
| ----------------- | ---------- | -----------------------------------------------------|
| --all	            | -a	     | clean all packages.	                                |
| --dev             | -d         | clean package/project for dev mode.                  |
| --recurse	        | -r	     | clean package/project along with its dependencies.   |

## 命令示例

**1. 清理指定项目的所有依赖项的构建缓存**

```shell
celer clean project_xxx
```

**2. 清理多个软件包的构建缓存**

```shell
celer clean ffmpeg@5.1.6 opencv@4.11.0
```

**3. 清理开发模式下的软件包构建缓存**

```shell
celer clean --dev/-d pkgconf@2.4.3
```

**4. 递归清理软件包及其依赖项的构建缓存**

```shell
celer clean --recurse/-r ffmpeg@5.1.6
```

**5. 清理所有构建缓存**

```shell
celer clean --all
```
