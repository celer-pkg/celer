package cmds

import (
	"io"
	"os"
	"strings"
	"testing"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/spf13/cobra"
)

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
