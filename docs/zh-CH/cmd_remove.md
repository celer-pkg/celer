# Remove 命令

&emsp;&emsp;Celer 移除命令从系统中卸载指定的软件包。它提供了灵活的移除选项，包括依赖项清理、构建缓存删除和开发模式软件包处理。

## 命令语法

```shell
celer remove [name@version] [flags]  
```

## 标志

| 标志 | 速记 | 描述 |
| ---- | --------- | ----------- |
| --build-type | -b | Uninstall a package with the specified build type (default: "release"). |
| --dev | -d | Uninstall a development-mode package (used for dev dependencies). |
| --purge | -f | Aggressive removal: Delete the package along with all its associated files (e.g., configs, data). |
| --recurse | -r | Recursive removal: Uninstall the package and its dependencies (if no other packages require them). |
| --remove-cache | -c | Clean build cache: Remove cached build artifacts for the package. |

## 用法示例

**1. 基本移除**

```shell
celer remove ffmpeg@5.1.6
```

**2. 移除依赖项**

```shell
celer remove ffmpeg@5.1.6 --recurse/-r
```

**3. 完全删除软件包**

```shell
celer remove ffmpeg@5.1.6 --purge/-p
```

**4. 移除开发模式软件包**

```shell
celer remove ffmpeg@5.1.6 --dev/-d
```

**5. 移除软件包并清理构建缓存**

```shell
celer remove ffmpeg@5.1.6 --remove-cache/-c
```