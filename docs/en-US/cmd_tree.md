# Tree command

&emsp;&emsp;The tree command visualizes dependency relationships for packages or projects, displaying both runtime dependencies and development dependencies by default.

## Command Syntax

```shell
celer tree [package_name|project_name] [flags]
```

## Command Options

| Option	        | Description                                          |
| ----------------- | -----------------------------------------------------|
| --hide-dev	    | Hide dev dependencies.	                           |

## Usage Examples

**1. Show complete dependency tree**

```shell
celer tree ffmpeg@5.1.6
```

**2. Show dependencies without runtime dependencies**

```shell
celer tree ffmpeg@5.1.6 --hide-dev
```

## Example Output

```
libcurl@3.8.1  
├── zlib@1.3.1  
├── openssl@3.1.4  
└── [dev] cmake@3.28.3  
    └── [dev] ninja@1.12.0  
```

- Regular items: Runtime dependencies.
- [dev] prefix: Development dependencies.