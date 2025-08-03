//go:build darwin || netbsd || freebsd || openbsd || dragonfly || linux

package envs

import (
	"celer/pkgs/env"
	"os"
)

// CleanEnv clear all environments and reset PATH.
func CleanEnv() {
	home := os.Getenv("HOME")
	shell := os.Getenv("SHELL")

	// Clear all environments.
	os.Clearenv()

	os.Setenv("SHELL", shell)
	os.Setenv("HOME", home)

	// Reset PATH.
	var paths []string
	paths = append(paths, "/usr/local/bin")
	paths = append(paths, "/usr/bin")
	paths = append(paths, "/usr/sbin")
	paths = append(paths, home+"/.local/bin")
	os.Setenv("PATH", env.JoinPaths("PATH", paths...))
}
