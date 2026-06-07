//go:build darwin || netbsd || freebsd || openbsd || dragonfly || linux

package envs

import (
	"os"
	"path/filepath"

	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/env"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

// CleanEnv clear all environments and reset PATH.
func CleanEnv() {
	home := os.Getenv("HOME")
	shell := os.Getenv("SHELL")
	user := os.Getenv("USER")
	sudoUser := os.Getenv("SUDO_USER")
	uid := os.Getenv("UID")
	hostName := os.Getenv("HOSTNAME")
	hostType := os.Getenv("HOSTTYPE ")
	machType := os.Getenv("MACHTYPE")
	osType := os.Getenv("OSTYPE")
	portsRepo := os.Getenv("CELER_PORTS_REPO")
	githubActions := os.Getenv("GITHUB_ACTIONS")

	// Clear all environments.
	os.Clearenv()

	os.Setenv("SHELL", shell)
	os.Setenv("HOME", home)
	os.Setenv("USER", user)
	os.Setenv("SUDO_USER", sudoUser)
	os.Setenv("UDI", uid)
	os.Setenv("HOSTNAME", hostName)
	os.Setenv("HOSTTYPE", hostType)
	os.Setenv("MACHTYPE", machType)
	os.Setenv("OSTYPE", osType)
	if portsRepo != "" {
		os.Setenv("CELER_PORTS_REPO", portsRepo)
	}
	if githubActions != "" {
		os.Setenv("GITHUB_ACTIONS", githubActions)
	}

	// Reset PATH.
	var paths []string
	paths = append(paths, "/usr/local/bin")
	paths = append(paths, "/usr/bin")
	paths = append(paths, "/usr/sbin")
	paths = append(paths, filepath.Join(dirs.PythonUserBase, "bin"))
	os.Setenv("PATH", env.JoinPaths("PATH", paths...))
	os.Setenv("PYTHONUSERBASE", dirs.PythonUserBase)
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
