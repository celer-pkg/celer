
# 🚀 Create Command

> Quickly create new platform, project, or port configurations

&emsp;&emsp;The `create` command generates configuration files for platforms, projects, or third-party library ports with a single command.


## 📝 Command Syntax

```shell
celer create [options]
```

## ⚙️ Command Options

| Option       | Description                 |
|--------------|-----------------------------|
| --platform   | Create a new platform       |
| --project    | Create a new project        |
| --port       | Create a new port           |


## 💡 Usage Examples

### 1️⃣ Create a New Platform

```shell
celer create --platform x86_64-linux-xxxx
```

> Recommended platform name pattern: `[arch]-[os]-xxxx`  
> Generated file location: `conf/platforms/`  
> Please edit the generated configuration file according to your target environment

For more platform configuration details, see [Platform Introduction](./article_platform.md)

### 2️⃣ Create a New Project

```shell
celer create --project xxxx
```

> Generated file location: `conf/projects/`  
> Please edit the generated configuration file according to your target project

For more project configuration details, see [Project Introduction](./article_project.md)

### 3️⃣ Create a New Port

```shell
celer create --port xxxx
```

> Generated file location example: `workspace/ports/glog/0.6.0/port.toml`  
> Please edit the generated configuration file according to your target library

For more port configuration details, see [Port Introduction](./article_port.md)

---

## 📚 Related Documentation

- [Quick Start](./quick_start.md)
- [Platform Introduction](./article_platform.md)
- [Project Introduction](./article_project.md)
- [Port Introduction](./article_port.md)