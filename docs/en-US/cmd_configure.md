# Clean command

&emsp;&emsp;The configure command allow to change global settings of current workspace.

## Command Syntax

```shell
celer configure [flags]
```

## Command Options

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

## Usage Examples

### 1. Configure platform

```shell
celer configure --platform xxxx
```

>The available platforms are file name of toml files under `conf/platforms`.

### 2. Configure project

```shell
celer configurte --project xxxx
```

>The available projects are file name of toml files under `conf/projects`.

### 3. Configure build type

```shell
celer configure --build-type Release
```

>Candicate build type are: Release, Debug, RelWithDebInfo, MinSizeRel

### 4. Configure jobs

```shell
celer configure --jobs 8
```

>The job number must be greater than zero.

### 5. Configure offline

```shell
celer configure --offline true|false
```

> The default offline mode is `false`.


### 6. Configure verbose

```shell
celer configure --verbose true|false
```

> The default verbose mode is `false`.

### 7. Configure cache-dir with dir and token

```shell
celer configure --cache-dir /home/xxx/cache --cache-token token_12345
```

>You can confiure --cache-dir and --cache-token at the same time or individually.

### 8. Configure porxy with host and port

```shell
celer configure --proxy-host 127.0.0.1 --proxy-port 7890
```

>You can confiure --proxy-host and --proxy-port at the same time or individually.
