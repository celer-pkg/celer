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
	NotSupportShell ShellType = iota
	BashShell
	ZshShell
	TypePowerShell
)

func CurrentShell() ShellType {
	if runtime.GOOS == "windows" {
		return TypePowerShell
	}

	switch runtime.GOOS {
	case "windows":
		return TypePowerShell

	case "linux":
		switch os.Getenv("SHELL") {
		case "/bin/bash":
			return BashShell
		case "/bin/zsh":
			return ZshShell
		default:
			return NotSupportShell
		}

	default:
		panic("unsupported os: " + runtime.GOOS)
	}
}
