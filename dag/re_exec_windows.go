//go:build windows

package dag

import (
	"os"
	"os/exec"
)

// reExecSelf runs path with the same arguments, stdin/stdout/stderr and
// environment, waits for it, then exits with its exit code.
func reExecSelf(path string) error {
	cmd := exec.Command(path, os.Args[1:]...)
	cmd.Env = os.Environ()
	cmd.Stdin = os.Stdin
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr

	if err := cmd.Run(); err != nil {
		if exitErr, ok := err.(*exec.ExitError); ok {
			os.Exit(exitErr.ExitCode())
		}
		return err
	}

	os.Exit(0)
	return nil // unreachable
}
