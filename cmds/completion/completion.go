package completion

import (
	"os"
	"runtime"
)

type Completion interface {
	Register() error
	Unregister() error

	installBinary() error
	uninstallBinary() error

	installCompletion() error
	uninstallCompletion() error

	registerRunCommand() error
	unregisterRunCommand() error
}

type ShellType int8

const (
	NotSupported ShellType = iota
	BashShell
	ZshShell
	TypePowerShell
)

func CurrentShell() ShellType {
	switch runtime.GOOS {
	case "windows":
		return TypePowerShell

	case "linux":
		shell := os.Getenv("SHELL")
		switch shell {
		case "/usr/bin/bash", "/bin/bash":
			return BashShell
		case "/usr/bin/zsh", "/bin/zsh":
			return ZshShell
		default:
			return NotSupported
		}

	default:
		panic("unsupported os: " + runtime.GOOS)
	}
}
