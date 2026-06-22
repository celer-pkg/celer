//go:build darwin || netbsd || freebsd || openbsd || dragonfly || linux

package cmd

import (
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
	)

	// Build command and display string.
	if len(e.args) == 0 {
		cmd = exec.Command("bash", "-c", e.command)
		displayCmd = e.command
	} else {
		cmd = exec.Command(e.command, e.args...)
		displayCmd = e.command + " " + strings.Join(e.args, " ")
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
		return fmt.Errorf("faild to execute command: %w", err)
	}

	return nil
}
