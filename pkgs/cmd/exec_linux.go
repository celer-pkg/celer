//go:build darwin || netbsd || freebsd || openbsd || dragonfly || linux

package cmd

import (
	"context"
	"fmt"
	"io"
	"os"
	"os/exec"
	"strings"

	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

// doExecute implements Unix/Linux/macOS specific command execution.
// It handles direct command execution with optional shell wrapping.
func (e *executor) doExecute(output io.Writer) error {
	var (
		cmd        *exec.Cmd
		displayCmd string
		ctx        context.Context
		cancel     context.CancelFunc
	)

	// Build command and display string.
	if len(e.args) == 0 {
		cmd = exec.Command("bash", "-c", e.command)
		displayCmd = e.command
	} else {
		cmd = exec.Command(e.command, e.args...)
		displayCmd = e.command + " " + strings.Join(e.args, " ")
	}

	// Apply timeout if set.
	if e.timeout > 0 {
		ctx, cancel = context.WithTimeout(context.Background(), e.timeout)
		cmd = exec.CommandContext(ctx, e.command, e.args...)
		if len(e.args) == 0 {
			cmd = exec.CommandContext(ctx, "bash", "-c", e.command)
		}
		defer cancel()
	}

	// Display execution info.
	if e.title != "" {
		color.Printf(color.Title, "\n%s\n", e.title)
		color.Printf(color.Hint, "▶ %s\n", displayCmd)
	}

	// Verify and set working directory.
	if e.workDir != "" {
		if !fileio.PathExists(e.workDir) {
			return fmt.Errorf("work directory does not exist: %s", e.workDir)
		}
		cmd.Dir = e.workDir
	}

	// Set up environment and stdin.
	cmd.Env = os.Environ()
	cmd.Stdin = os.Stdin

	// Create and configure log file.
	logFile, err := e.createLogFile(cmd)
	if err != nil {
		return fmt.Errorf("failed to setup logging: %w", err)
	}
	if logFile != nil {
		defer logFile.Close()
	}

	// Route output to appropriate destinations.
	e.configureOutputs(cmd, logFile, output)

	// Execute command and return result.
	if err := cmd.Run(); err != nil {
		if ctx != nil && ctx.Err() == context.DeadlineExceeded {
			return fmt.Errorf("command timed out after %s: %s", e.timeout, displayCmd)
		}
		return fmt.Errorf("command execution failed: %w", err)
	}

	return nil
}
