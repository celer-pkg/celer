//go:build darwin || netbsd || freebsd || openbsd || dragonfly || linux

package envs

import (
	"os"
	"path/filepath"

	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/env"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

var preserveKeys = []string{
	"HOME",
	"SHELL",
	"USER",
	"SHELL",
	"USER",
	"SUDO_USER",
	"UID",
	"HOSTNAME",
	"HOSTTYPE",
	"MACHTYPE",
	"OSTYPE",
	"CELER_PORTS_REPO",
	"HTTP_PROXY",
	"HTTPS_PROXY",
	"http_proxy",
	"https_proxy",
}

// CleanEnv clear all environments and reset PATH.
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
	os.Setenv("GIT_SSL_NO_VERIFY", "true")
	for key, value := range preservedEnvs {
		os.Setenv(key, value)
	}

	// Reset PATH.
	var paths []string
	paths = append(paths, "/usr/local/bin")
	paths = append(paths, "/usr/bin")
	paths = append(paths, "/usr/sbin")
	paths = append(paths, filepath.Join(dirs.PythonUserBase, "bin"))
	os.Setenv("PATH", env.JoinPaths("PATH", paths...))
}

// AppendPythonBinDir adds the Python virtual environment bin directory to PATH.
func AppendPythonBinDir(venvDir string) {
	if venvDir == "" {
		return
	}

	binDir := filepath.Join(venvDir, "bin")
	if fileio.PathExists(binDir) {
		os.Setenv("PATH", env.JoinPaths("PATH", binDir))
	}
}
