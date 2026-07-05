//go:build windows

package envs

import (
	"os"
	"path/filepath"

	"github.com/celer-pkg/celer/pkgs/env"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

var preserveKeys = []string{
	"TEMP",
	"TMP",
	"OS",
	"HOMEDRIVE",
	"HOMEPATH",
	"USERNAME",
	"USERPROFILE",
	"SystemRoot",
	"SystemDrive",
	"LOCALAPPDATA",
	"PROCESSOR_ARCHITECTURE",
	"PROCESSOR_IDENTIFIER",
	"PROCESSOR_LEVEL",
	"PROCESSOR_REVISION",
	"NUMBER_OF_PROCESSORS",
	"CELER_PORTS_REPO",
	"GITHUB_ACTIONS",
	"HTTP_PROXY",
	"HTTPS_PROXY",
	"http_proxy",
	"https_proxy",
}

// CleanEnv clear all environments that not required and reset PATH.
func CleanEnv() {
	// Cache preserved key-value.
	preservedEnvs := make(map[string]string, len(preserveKeys))
	for _, key := range preserveKeys {
		if val := os.Getenv(key); val != "" {
			preservedEnvs[key] = val
		}
	}

	// Clear and preserve.
	os.Clearenv()
	for key, value := range preservedEnvs {
		os.Setenv(key, value)
	}

	// Reset PATH.
	var paths []string
	paths = append(paths, `C:\Windows`)
	paths = append(paths, `C:\Windows\System32`)
	paths = append(paths, `C:\Windows\SysWOW64`)
	paths = append(paths, `C:\Windows\System32\Wbem`)
	paths = append(paths, `C:\Windows\System32\downlevel`)
	paths = append(paths, `C:\Windows\SysWOW64\WindowsPowerShell\v1.0`)
	paths = append(paths, `C:\Windows\System32\WindowsPowerShell\v1.0`)
	paths = append(paths, `C:\ProgramData\chocolatey\bin`)

	// Use PATH instead of Path.
	os.Unsetenv("Path")
	os.Setenv("PATH", env.JoinPaths("PATH", paths...))
}

// AppendPythonBinDir appends the Python user "Scripts" directory to PATH if it exists.
func AppendPythonBinDir(userBaseDir string) {
	scriptsDir := filepath.Join(userBaseDir, "Scripts")
	if fileio.PathExists(scriptsDir) {
		os.Setenv("PATH", env.JoinPaths("PATH", scriptsDir))
		return
	}
}
