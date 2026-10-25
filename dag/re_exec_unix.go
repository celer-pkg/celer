//go:build darwin || netbsd || freebsd || openbsd || dragonfly || linux

package dag

import (
	"os"
	"syscall"
)

// reExecSelf replaces the current process image with path, keeping the arguments and the
// environment, so the pinned celer continues the very command that triggered the switch.
func reExecSelf(path string) error {
	return syscall.Exec(path, append([]string{path}, os.Args[1:]...), os.Environ())
}
