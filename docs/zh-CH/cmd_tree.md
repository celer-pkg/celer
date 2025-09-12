# Tree 命令

&emsp;&emsp;Celer 树命令可视化显示软件包或项目的依赖关系，默认显示运行时依赖项和开发依赖项。

## 命令语法

```shell
celer tree [package_name|project_name] [flags]
```

## 命令选项

| 选项	        | 描述                                          |
| ----------------- | -----------------------------------------------------|
| --hide-dev	    | Hide dev dependencies.	                           |

## 用法示例

**1. 显示完整的依赖树**

```shell
celer tree ffmpeg@5.1.6
```

**2. 显示不包含运行时依赖项的依赖项**

```shell
celer tree ffmpeg@5.1.6 --hide-dev
```

## 示例输出

```
libcurl@3.8.1  
├── zlib@1.3.1  
├── openssl@3.1.4  
└── [dev] cmake@3.28.3  
    └── [dev] ninja@1.12.0  
```

- Regular items: Runtime dependencies.
- [dev] prefix: Development dependencies.