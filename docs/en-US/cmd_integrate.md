# ⚡ Integrate Command

&emsp;&emsp;The `integrate` command provides intelligent Tab completion for Celer, significantly improving command-line efficiency and user experience.

## Command Syntax

```shell
celer integrate [options]
```

## 🖥️ Supported Shells

Celer supports auto-completion for the following mainstream Shell environments:

| Shell       | Operating System | Description                    |
|-------------|------------------|--------------------------------|
| Bash        | Linux/macOS      | Most common Linux Shell        |
| Zsh         | Linux/macOS      | Powerful interactive Shell     |
| PowerShell  | Windows          | Native Windows Shell           |

## ⚙️ Command Options

| Option     | Description                    |
|------------|--------------------------------|
| --remove   | Remove Tab completion feature  |

## 💡 Usage Examples

### 1️⃣ Enable Tab Completion

```shell
celer integrate
```

> Celer can automatically detect the current Shell type and enable Tab completion for it.

### 2️⃣ Remove Tab Completion

```shell
celer integrate --remove
```

> When removing Tab completion, Celer can also automatically detect the current Shell type.

---

## 🎯 Features

### Command Completion
Press Tab after typing `celer` to auto-complete available commands:
```shell
celer <Tab>
# Shows: install, configure, deploy, create, remove, clean, etc.
```

### Option Completion
Press Tab after typing a command to auto-complete available options:
```shell
celer install --<Tab>
# Shows: --dev, --force, --jobs, --recursive, --store-cache, --cache-token
```

### Package Name Completion
Press Tab when typing a package name to auto-complete known ports:
```shell
celer install ffm<Tab>
# Auto-completes to: celer install ffmpeg@
```

---

## ⚠️ Notes

1. **Permission Requirements**: Administrator privileges may be required on some systems
2. **Terminal Restart**: Configuration may require restarting the terminal or reloading the configuration
3. **Shell Version**: Ensure the Shell version supports completion features
4. **Multiple Shells**: If using multiple Shells, run the integrate command in each Shell separately

---

## 📚 Related Documentation

- [Quick Start](./quick_start.md)
- [Commands Overview](./commands.md)

---

**Need Help?** [Report an Issue](https://github.com/celer-pkg/celer/issues) or check our [Documentation](../../README.md)