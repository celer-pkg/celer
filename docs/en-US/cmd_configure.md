# Configure Command

&emsp;&emsp;The Configure command allows users to configure global settings for the current workspace.

## Command Syntax

```shell
celer configure [flags]
```

## ⚙️ Command Options
| Option                    | Type    | Description                            |
|---------------------------|---------|----------------------------------------|
| --platform                | string  | Configure platform                     |
| --project                 | string  | Configure project                      |
| --build-type              | string  | Configure build type                   |
| --jobs                    | integer | Configure parallel build jobs          |
| --offline                 | boolean | Enable offline mode                    |
| --verbose                 | boolean | Enable verbose logging mode            |
| --proxy-host              | string  | Configure proxy host                   |
| --proxy-port              | integer | Configure proxy port                   |
| --package-cache-dir       | string  | Configure package cache directory      |
| --package-cache-token     | string  | Configure package cache token          |
| --ccache-enabled          | boolean | Configure ccache enabled               |
| --ccache-dir              | string  | Configure ccache working directory     |
| --ccache-maxsize          | string  | configure ccache maxsize               |
| --ccache-remote-storage   | string  | configure ccache remote storage        |
| --ccache-remote-only      | string  | configure ccache remote only           |


### 1️⃣ Configure Platform

```shell
celer configure --platform xxxx
```

> Available platforms come from toml files in the `conf/platforms` directory.

### 2️⃣ Configure Project

```shell
celer configure --project xxxx
```

> Available projects come from toml files in the `conf/projects` directory.

### 3️⃣ Configure Build Type

```shell
celer configure --build-type Release
```

> Available build types: Release, Debug, RelWithDebInfo, MinSizeRel  
> The default build type is Release.

### 4️⃣ Configure Parallel Jobs

```shell
celer configure --jobs 8
```

> The number of parallel jobs must be greater than 0.

### 5️⃣ Configure Offline Mode

```shell
celer configure --offline true|false
```

> The default offline mode is `false`.

### 6️⃣ Configure Verbose Logging Mode

```shell
celer configure --verbose true|false
```

> The default verbose logging mode is `false`.

---

## 🌐 Proxy Configuration

### Configure Proxy Host and Port

```shell
celer configure --proxy-host 127.0.0.1 --proxy-port 7890
```
> In China, you may need to configure a proxy to access GitHub resources.

---

## 🗄️ Package Cache Configuration

### Configure Package Cache Directory and Token

```shell
celer configure --package-cache-dir /home/xxx/cache --package-cache-token token_12345
```

> You can configure --package-cache-dir and --package-cache-token together or separately.

---

## 📦 ccache Configuration

### Enable ccache to Accelerate Builds

```shell
celer configure --ccache-enabled true
celer configure --ccache-dir /home/xxx/.ccache
celer configure --ccache-maxsize 5G
celer configure --ccache-remote-storage http://SERVER_IP:8080/ccache
celer configure --ccache-remote-only true
```

> Enable compiler caching to speed up repeated builds.
