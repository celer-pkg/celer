# Clean command

&emsp;&emsp;Configure命令允许用户配置当前工作空间的全局设置。

## 命令语法

```shell
celer configure [flags]
```

## 命令选项

| Option	        | Description                |
| ----------------- | ---------------------------|
| --platform	    | configure platform.	     |
| --project 	    | configure project.	     |
| --build-type	    | configure build type.	     |
| --jobs            | configure jobs.            |
| --offline         | configure offline mode.    |
| --verbose         | configure verbose mode.    |
| --cache-dir       | configure cache dir.       |
| --cache-token	    | configure cache token.     |
| --proxy-host      | configure proxy host.      |
| --proxy-port	    | configure proxy port.      |

## 命令示例

### 1. 配置平台
 
```shell
celer configure --platform xxxx
```

>配置可用的平台来自`conf/platforms`目录下的toml文件。

### 2. 配置项目

```shell
celer configurte --project xxxx
```

>配置可用的项目来自`conf/projects`目录下的toml文件。

### 3. 配置构建类型

```shell
celer configure --build-type Release
```

>候选的构建类型有: Release, Debug, RelWithDebInfo, MinSizeRel
>默认的构建类型是Release。

### 4. 配置并发任务数

```shell
celer configure --jobs 8
```

>并发任务数必须大于0。

### 5. 配置离线模式

```shell
celer configure --offline true|false
```

> 默认的离线模式是`false`。

### 6. 配置详细日志模式

```shell
celer configure --verbose true|false
```

> 默认的详细日志模式是`false`。

### 7. 配置缓存目录和缓存令牌

```shell
celer configure --cache-dir /home/xxx/cache --cache-token token_12345
```

>你可以同时配置缓存目录和缓存令牌，也可以分别配置。

### 8. 配置代理地址和端口

```shell
celer configure --proxy-host 127.0.0.1 --proxy-port 7890
```

>你可以同时配置代理地址和端口，也可以分别配置。