//go:build windows

package cmd

import (
	"fmt"
	"io"
	"os"
	"os/exec"
	"strings"
	"syscall"

	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

// doExecute implements Windows specific command execution.
// It handles both native Windows commands and MSYS2/bash environments.
func (e *executor) doExecute(output io.Writer) error {
	var (
		cmd        *exec.Cmd
		displayCmd string
	)

	// Build command and display string based on environment mode.
	if e.msys2Env {
		cmd = e.buildMSYS2Command(&displayCmd)
	} else {
		cmd = e.buildNativeCommand(&displayCmd)
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

	// Set up stdin.
	cmd.Stdin = os.Stdin

	// Create and configure log file.
	logFile, err := e.createLogFile(cmd)
	if err != nil {
		return fmt.Errorf("failed to setup logging -> %w", err)
	}
	if logFile != nil {
		defer logFile.Close()
	}

	// Route output to appropriate destinations.
	e.configureOutputs(cmd, logFile, output)

	// Execute command and return result.
	if err := cmd.Run(); err != nil {
		return fmt.Errorf("faild to execute command -> %w", err)
	}

	return nil
}

// buildMSYS2Command constructs a command to run in MSYS2/bash environment.
func (e *executor) buildMSYS2Command(displayCmd *string) *exec.Cmd {
	var args []string
	if e.msvcEnvs != "" {
		args = append(args, e.msvcEnvs)
	}
	args = append(args, e.command)
	if len(e.args) > 0 {
		args = append(args, e.args...)
	}
	*displayCmd = strings.Join(args, " ")

	cmd := exec.Command("bash", "-lc", strings.Join(args, " "))

	// Configure MSYS2 environment variables.
	cmd.Env = append(os.Environ(),
		"MSYSTEM=MINGW64",               // Use MinGW64 subsystem
		"CHERE_INVOKING=1",              // Disable directory changing
		"MSYS=winsymlinks:nativestrict", // Use native Windows symlinks
	)

	return cmd
}

// buildNativeCommand constructs a command to run in native Windows environment.
func (e *executor) buildNativeCommand(displayCmd *string) *exec.Cmd {
	*displayCmd = e.command
	if len(e.args) > 0 {
		*displayCmd += " " + strings.Join(e.args, " ")
	}

	var cmd *exec.Cmd

	if len(e.args) == 0 {
		// Command without arguments: wrap in cmd.exe /c for shell features
		cmd = exec.Command("cmd")
		cmd.SysProcAttr = &syscall.SysProcAttr{
			CmdLine:    fmt.Sprintf(`/c %s`, e.command),
			HideWindow: true, // Hide console window for native commands
		}
	} else {
		// Command with arguments: direct execution (safer, no shell parsing)
		cmd = exec.Command(e.command, e.args...)
	}

	cmd.Env = os.Environ()
	return cmd
}
