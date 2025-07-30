//go:build darwin || netbsd || freebsd || openbsd || dragonfly || linux

package envs

import (
	"celer/pkgs/env"
	"log"
	"os"
)

// CleanEnv clear all environments and reset PATH.
func CleanEnv() {
	// Get user home dir.
	homeDir, err := os.UserHomeDir()
	if err != nil {
		log.Fatalf("get home directory: %s", err)
	}

	shell := os.Getenv("SHELL")

	// Clear all environments.
	os.Clearenv()

	os.Setenv("SHELL", shell)

	// Reset PATH.
	var paths []string
	paths = append(paths, "/usr/local/bin")
	paths = append(paths, "/usr/bin")
	paths = append(paths, "/usr/sbin")
	paths = append(paths, homeDir+"/.local/bin")
	os.Setenv("PATH", env.JoinPaths("PATH", paths...))
}
