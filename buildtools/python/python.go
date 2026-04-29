package python

import (
	"os/exec"
	"runtime"
	"strings"
)

// Python interface defines methods for Python tools, including system Python and conda Python.
type Python interface {
	Setup() error
	GetVersion() string
	GetExecutable() (string, error)
}

// GetSystemPythonVersion detects the system python version.
func GetSystemPythonVersion() string {
	var (
		output []byte
		err    error
	)

	if runtime.GOOS == "windows" {
		// On Windows, try python.exe or python3.exe
		output, err = exec.Command("python", "--version").Output()
		if err != nil {
			output, err = exec.Command("python3", "--version").Output()
		}
	} else {
		// On Unix-like systems, try python3
		output, err = exec.Command("python3", "--version").Output()
		if err != nil {
			output, err = exec.Command("python", "--version").Output()
		}
	}

	if err != nil {
		return ""
	}

	// Parse "Python 3.11.0" -> "3.11.0"
	versionStr := strings.TrimSpace(string(output))
	parts := strings.Fields(versionStr)
	if len(parts) >= 2 {
		return parts[1]
	}
	return ""
}
