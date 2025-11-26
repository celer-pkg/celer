
# ⚙️ configure Command

> Change global settings for your workspace, including platform, project, build type, cache, ccache, and more.

The `configure` command allows you to set or update global options for the current workspace. You can quickly switch platforms, projects, build types, enable ccache, and configure advanced options for efficient builds.


## 📝 Command Syntax

```shell
celer configure [flags]
```


## ⚙️ Command Options

| Option	            | Description                   |
| --------------------- | ------------------------------|
| --platform	        | configure platform.	        |
| --project 	        | configure project.	        |
| --build-type	        | configure build type.	        |
| --jobs                | configure jobs.               |
| --offline             | configure offline mode.       |
| --verbose             | configure verbose mode.       |
| --binary-cache-dir    | configure binary cache dir.   |
| --binary-cache-token  | configure binary cache token. |
| --proxy-host          | configure proxy host.         |
| --proxy-port          | configure proxy port.         |
| --ccache-enabled      | configure ccache enabled.     |
| --ccache-compress     | configure ccache compress.    |
| --ccache-dir          | configure ccache dir.         |
| --ccache-maxsize      | configure ccache maxsize.     |

---

## 🌐 Proxy Configuration

### Configure proxy host and port
```shell
celer configure --proxy-host 127.0.0.1 --proxy-port 7890
```
> In China, you may need to configure a proxy to access GitHub resources.

---

## 🗄️ Binary Cache Configuration

### Configure binary cache directory and token
```shell
celer configure --binary-cache-dir /home/xxx/cache --binary-cache-token token_12345
```
> You can set --binary-cache-dir and --binary-cache-token together or separately.

---

## 📦 Ccache Configuration

### Enable ccache for faster builds
```shell
celer configure --ccache-enabled true
celer configure --ccache-dir /home/xxx/.ccache
celer configure --ccache-maxsize 5G
celer configure --ccache-compress true
```
> Enable compiler caching to speed up repeated builds.

### CCache Support
You can enable ccache to accelerate C/C++ compilation. When enabled, Celer will automatically inject ccache into your toolchain.

- `--ccache-enabled true|false` : Enable or disable ccache
- `--ccache-dir <path>` : Set ccache working directory
- `--ccache-maxsize <size>` : Set maximum ccache size (e.g. "10G")
- `--ccache-compress true|false` : Enable ccache compression
