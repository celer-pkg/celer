package cmds

import (
	"io"
	"os"
	"slices"
	"strings"
	"testing"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/spf13/cobra"
)

// fakeContext is a minimal context.Context implementation for testing.
type fakeContext struct{}

func (f fakeContext) Version() string                        { return "test" }
func (f fakeContext) Platform() context.Platform             { return nil }
func (f fakeContext) RootFS() context.RootFS                 { return nil }
func (f fakeContext) Project() context.Project               { return nil }
func (f fakeContext) BuildType() string                      { return "Release" }
func (f fakeContext) Downloads() string                      { return "" }
func (f fakeContext) LibraryFolder() string                  { return "" }
func (f fakeContext) Jobs() int                              { return 1 }
func (f fakeContext) Offline() bool                          { return true }
func (f fakeContext) Verbose() bool                          { return false }
func (f fakeContext) InstalledDir() string                   { return "" }
func (f fakeContext) InstalledDevDir() string                { return "" }
func (f fakeContext) PkgCacheConfig() context.PkgCacheConfig { return nil }
func (f fakeContext) DevCacheConfig() context.DevCacheConfig { return nil }
func (f fakeContext) ProxyHostPort() (host string, port int) { return "", 0 }
func (f fakeContext) CCacheEnabled() bool                    { return false }
func (f fakeContext) GenerateToolchainFile() error           { return nil }
func (f fakeContext) ExprVars() *context.ExprVars            { return nil }
func (f fakeContext) PythonConfig() context.PythonConfig     { return nil }
func (f fakeContext) Features() context.Features             { return nil }

// newInitializedCeler returns a Celer that has been Init'd and has a freshly
// cloned conf repo — the minimum state required for configureCmd's
// checkIfInitialized() to pass. Every cmd.Execute()-style test needs this.
func newInitializedCeler(t *testing.T) *configs.Celer {
	t.Helper()
	dirs.RemoveAllForTest()

	celer := configs.NewCeler()
	if err := celer.Init(); err != nil {
		t.Fatal(err)
	}
	if err := celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true); err != nil {
		t.Fatal(err)
	}
	return celer
}

// runCommand builds the cobra command from `command`, feeds it `args` just
// like a user would on the shell, captures everything the command writes to
// stderr, and returns (stderr, executeErr).
func runCommand(t *testing.T, cmd *cobra.Command, args ...string) (string, error) {
	t.Helper()

	// Redirect os.Stderr to a pipe. Anything the command writes to
	// stderr lands in `reader`. We restore the original os.Stderr before returning.
	stdErr := os.Stderr
	reader, writer, err := os.Pipe()
	if err != nil {
		t.Fatal(err)
	}
	os.Stderr = writer

	// Drain the pipe in a goroutine while the command runs.
	done := make(chan string, 1)
	go func() {
		var buf strings.Builder
		_, _ = io.Copy(&buf, reader)
		done <- buf.String()
	}()

	// Run the command, then close the write end so the goroutine sees
	// EOF and exits. Order matters: close must happen before <-done,
	// otherwise the receive blocks forever.
	cmd.SetArgs(args)
	execErr := cmd.Execute()
	_ = writer.Close()
	os.Stderr = stdErr
	return <-done, execErr
}

// equals reports whether list1 and list2 contain the same elements, ignoring order.
func equals(list1, list2 []string) bool {
	if len(list1) != len(list2) {
		return false
	}
	for _, item := range list1 {
		if !slices.Contains(list2, item) {
			return false
		}
	}
	return true
}

func check(t *testing.T, err error) {
	t.Helper()
	if err != nil {
		t.Fatal(err)
	}
}
