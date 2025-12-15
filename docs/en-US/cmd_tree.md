# 🌳 Tree Command

&emsp;&emsp;The `tree` command is used to visualize the dependency tree of a package or project, displaying both runtime dependencies and development dependencies by default.

## Command Syntax

```shell
celer tree [package_name|project_name] [options]
```

## ⚙️ Command Options

| Option       | Description                    |
|--------------|--------------------------------|
| --hide-dev   | Hide development dependencies  |

## 💡 Usage Examples

### 1️⃣ Show Complete Dependency Tree

```shell
celer tree ffmpeg@5.1.6
```

> Display all runtime and development dependencies for FFmpeg.

### 2️⃣ Hide Development Dependencies

```shell
celer tree ffmpeg@5.1.6 --hide-dev
```

> Show only runtime dependencies, hiding build tools required during compilation.

### 3️⃣ Show Project Dependency Tree

```shell
celer tree project_xxx
```

> Execute in the project directory to display all dependencies for the current project.

---

## 📊 Example Output

```
display dependencies in tree view:
--------------------------------------------
libffi@3.4.8
├── macros@1.20.2 -- [dev]
│   └── automake@1.18 -- [dev]
│       └── autoconf@2.72 -- [dev]
│           └── m4@1.4.19 -- [dev]
└── libtool@2.5.4 -- [dev]
    └── m4@1.4.19 -- [dev]
---------------------------------------------
summary: dependencies: 0  dev_dependencies: 5
```

### Output Description

- **Regular items**: Runtime dependencies, required for the library to run
- **[dev] prefix**: Development dependencies, only needed during build, not included in final deployment

---

## 🎯 Use Cases

### Case 1: Analyze Dependencies
Before installing a new library, check its dependencies to understand what additional libraries will be installed.

```shell
celer tree opencv@4.11.0
```

### Case 2: Troubleshoot Dependency Issues
When encountering dependency conflicts or build errors, view the complete dependency tree to locate the problem.

```shell
celer tree ffmpeg@5.1.6
```

### Case 3: Verify Project Configuration
Validate that dependencies configured in the project are correct.

```shell
celer tree
```

---

## 📝 Notes

1. **Circular Dependencies**: If circular dependencies exist, the command will report an error
2. **Large Projects**: For projects with many dependencies, output may be very long
3. **Version Information**: Version numbers shown in the tree structure are the actually installed versions
4. **Development Dependencies**: Use `--hide-dev` to simplify output and focus only on runtime dependencies

---

## 📚 Related Documentation

- [Quick Start](./quick_start.md)
- [Install Command](./cmd_install.md) - Install third-party libraries
- [Reverse Command](./cmd_reverse.md) - Analyze reverse dependencies
- [Project Configuration](./article_project.md) - Configure project dependencies

---

**Need Help?** [Report an Issue](https://github.com/celer-pkg/celer/issues) or check our [Documentation](../../README.md)