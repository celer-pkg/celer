# Clean command

&emsp;&emsp;The configure command allow to change global settings of current workspace.

## Command Syntax

```shell
celer configure [flags]
```

## Command Options

| Option	        | Description                                          |
| ----------------- | -----------------------------------------------------|
| --platform	    | configure platform.	                               |
| --project 	    | configure project.	                               |
| --build-type	    | configure build type.	                               |
| --cache-dir       | configure cache dir.                                 |
| --cache-token	    | configure cache token.                               |
| --jobs            | configure jobs.                                      |
| --offline         | configure offline mode.                              |

## Usage Examples

**1. Configure platform**

```shell
celer configure --platform xxxx
```

>The available platforms are file name of toml files under `conf/platforms`.

**2. Configure project**

```shell
celer configurte --project xxxx
```

>The available projects are file name of toml files under `conf/projects`.

**3. Configure build type**

```shell
celer configure --build-type Release
```

>Candicate build type are: Release, Debug, RelWithDebInfo, MinSizeRel

**4. Configure cache-dir, cache-token**

```shell
celer configure --cache-dir /home/xxx/cache --cache-token token_12345
```

>You can confiure --cache-dir and --cache-token at the same time or individually.

**5. Configure jobs**

```shell
celer configure --jobs 8
```

>The job number must be greater than zero.

**6. Configure offline**

```shell
celer configure --offline true|false
```

> The default offline mode is `false`.



