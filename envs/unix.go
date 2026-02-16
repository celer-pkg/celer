//go:build darwin || netbsd || freebsd || openbsd || dragonfly || linux

package envs

import (
	"celer/pkgs/dirs"
	"celer/pkgs/env"
	"os"
	"path/filepath"
)

// CleanEnv clear all environments and reset PATH.
func CleanEnv() {
	home := os.Getenv("HOME")
	shell := os.Getenv("SHELL")
	portsRepo := os.Getenv("CELER_PORTS_REPO")
	githubActions := os.Getenv("GITHUB_ACTIONS")

	// Clear all environments.
	os.Clearenv()

	os.Setenv("SHELL", shell)
	os.Setenv("HOME", home)
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

// In linux, python user scripts are always in ${PYTHONUSERBASE}/bin.
// It has been added to PATH in CleanEnv, so here do nothing.
func AppendPythonBinDir(_ string) {}
