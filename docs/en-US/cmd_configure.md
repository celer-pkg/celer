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
| --binary-cache-dir        | string  | Configure binary cache directory       |
| --binary-cache-token      | string  | Configure binary cache token           |
| --ccache-compress         | boolean | Configure ccache compression           |
| --ccache-dir              | string  | Configure ccache working directory     |
| --ccache-enabled          | boolean | Enable ccache                          |
| --ccache-maxsize          | string  | Set ccache max size (e.g., "10G")      |

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

## 🗄️ Binary Cache Configuration

### Configure Binary Cache Directory and Token

```shell
celer configure --binary-cache-dir /home/xxx/cache --binary-cache-token token_12345
```

> You can configure --binary-cache-dir and --binary-cache-token together or separately.

---

## 📦 ccache Configuration

### Enable ccache to Accelerate Builds

```shell
celer configure --ccache-enabled true
celer configure --ccache-dir /home/xxx/.ccache
celer configure --ccache-maxsize 5G
celer configure --ccache-compress true
```

> Enable compiler caching to speed up repeated builds.
