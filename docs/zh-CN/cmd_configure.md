# Clean command

&emsp;&emsp;Configure命令允许用户配置当前工作空间的全局设置。

## 命令语法

```shell
celer configure [flags]
```

## ⚙️ 命令选项
| 选项                      | 类型    | 说明                                   |
|---------------------------|---------|--------------------------------------|
| --platform                | 字符串  | 配置平台                               |
| --project                 | 字符串  | 配置项目                               |
| --build-type              | 字符串  | 配置构建类型                           |
| --downloads               | 字符串  | 配置资源下载路径                        |
| --jobs                    | 整数    | 配置并行构建任务数                     |
| --offline                 | 布尔    | 启用离线模式                           |
| --verbose                 | 布尔    | 启用详细日志模式                       |
| --proxy-host              | 字符串  | 配置代理地址                           |
| --proxy-port              | 整数    | 配置代理端口                           |
| --package-cache-dir        | 字符串  | 配置二进制缓存目录                     |
| --package-cache-token      | 字符串 | 配置二进制缓存令牌                     |
| --ccache-enabled          | 布尔    | 配置 ccache 是否启用                |
| --ccache-dir              | 字符串  | 配置 ccache 工作目录                   |
| --ccache-maxsize          | 字符串  | 设置 ccache 最大空间（如 "10G"）        |
| --ccache-remote-storage   | 字符串  | 设置 ccache 缓存服务地址                |
| --ccache-remote-only      | 布尔    | 设置 ccache 是否只启用服务器缓存        |

### 1️⃣ 配置平台

```shell
celer configure --platform xxxx
```

>配置可用的平台来自`conf/platforms`目录下的toml文件。

### 2️⃣ 配置项目

```shell
celer configure --project xxxx
```

>配置可用的项目来自`conf/projects`目录下的toml文件。

### 3️⃣ 配置构建类型

```shell
celer configure --build-type Release
```

>候选的构建类型有: Release, Debug, RelWithDebInfo, MinSizeRel
>默认的构建类型是Release。

### 3️⃣ 配置资源下载路径

```shell
celer configure --downloads=$HOME/downloads
```

>默认的资源下载路径是$workspace/downloads, 但支持配置多个worksapce为一个downloads目录以节省磁盘空间。

### 5️⃣ 配置并发任务数

```shell
celer configure --jobs 8
```

>并发任务数必须大于0。

### 6️⃣ 配置离线模式

```shell
celer configure --offline true|false
```

> 默认的离线模式是`false`。

### 7️⃣ 配置详细日志模式

```shell
celer configure --verbose true|false
```

> 默认的详细日志模式是`false`。

---

## 🌐 代理相关配置

### 配置代理地址和端口

```shell
celer configure --proxy-host 127.0.0.1 --proxy-port 7890
```
>在中国为了能访问github资源，你可能需要通过配置代理访问。

---

## 🗄️ 二进制缓存相关配置

### 配置二进制缓存目录和令牌

```shell
celer configure --package-cache-dir /home/xxx/cache --package-cache-token token_12345
```

>你可以同时配置 --package-cache-dir 和 --package-cache-token，也可以分别配置。

---

## 📦 ccache 相关配置

### 启用 ccache 加速构建

```shell
celer configure --ccache-enabled true
celer configure --ccache-dir /home/xxx/.ccache
celer configure --ccache-maxsize 5G
celer configure --ccache-remote-storage http://SERVER_IP:8080/ccache
celer configure --ccache-remote-only true
```

>启用编译器缓存以加速重复构建。