package python

import (
	"fmt"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
)

// SystemPython this is usually work for linux and mac.
type SystemPython struct {
	rootDir string
}

// Setup no need setup for linux and mac.
func (s *SystemPython) Setup() error {
	return nil
}

// GetVersion detects the system python3 version
// Returns version string (e.g., "3.11.0") or empty string if not found.
func (s *SystemPython) GetVersion() string {
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

func (s *SystemPython) GetExecutable() (string, error) {
	switch runtime.GOOS {
	case "windows":
		return filepath.Join(s.rootDir, "python.exe"), nil
	case "linux", "darwin":
		return "/usr/bin/python3", nil
	default:
		return "", fmt.Errorf("unsupported os to setup python")
	}
}
