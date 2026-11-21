package integration

import (
	"os"
	"testing"

	"github.com/celer-pkg/celer/internal/dirs"
)

// TestMain pins the workspace to this package directory.
//
// dirs otherwise looks upward for the nearest directory holding celer.toml,
// conf or ports. In a developer checkout that is the repository root, where the
// tests would replace the developer's conf clone and delete their packages,
// installed and buildtrees directories. go test runs the binary with the
// package directory as the working directory, which is the workspace these
// tests expect.
func TestMain(m *testing.M) {
	if wd, err := os.Getwd(); err == nil {
		dirs.Init(wd)
	}
	os.Exit(m.Run())
}
