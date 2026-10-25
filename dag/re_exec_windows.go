//go:build windows

package dag

import (
	"fmt"
)

// reExecSelf is not supported on Windows.
func reExecSelf(path string) error {
	return fmt.Errorf("switching celer automatically is not supported on windows, run the command with '%s' instead", path)
}
