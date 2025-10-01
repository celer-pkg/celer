# Create command

&emsp;&emsp;The create command allow to create a new platform, a new project or a new port under conf.

## Command Syntax

```shell
celer create [flags]
```

## Command Options

| Option	        | Description              |
| ----------------- | -------------------------|
| --platform	    | create a new platform.   |
| --project 	    | create a new project.	   |
| --port	        | create a new port.	   |

## Usage Examples

### 1. Create a new platform

```shell
celer create --platform x86_64-linux-xxxx
```

>The suggested platform name pattern should be like `[arch]-[os]-xxxx`.
>The generated file is located in the **conf/platforms** directory.  
>Then you need to open the generated file and configure it according to your target environment.

For the details, you can read the [introduction of the platform](./introduce_platform.md).

### 2. Create a new project

```shell
celer create --project xxxx
```

>The generated file is located in the **conf/projects** directory.   
>Then you need to open the generated file and configure it with your target project.

For the details, you can read the [introduction of the project](./introduce_project.md).

### 3. Create a new port

```shell
celer create --port xxxx
```

>After creating the port, you need to open the generated file and configure it with your target library. The generated file is located in the **workspace/ports/glog/0.6.0/port.toml** directory.

For the details, you can read the [introduction of the port](./introduce_port.md).