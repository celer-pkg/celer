package cmd

import (
	"bytes"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"time"

	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

// executor manages command execution with logging, environment configuration, and output routing.
type executor struct {
	msys2Env         bool     // Whether to execute in MSYS2 environment (Windows only)
	title            string   // Execution title for display output
	command          string   // Command to execute
	args             []string // Command arguments
	msvcEnvs         string   // MSVC environment setup string (Windows only)
	workDir          string   // Working directory for command execution
	logPath          string   // File path for execution logs
	retryMaxAttempts int      // 0 = no retry (default)
}

// NewExecutor creates a new Executor with the given title, command, and arguments.
func NewExecutor(title string, command string, args ...string) *executor {
	return &executor{
		title:            title,
		command:          command,
		args:             args,
		msys2Env:         false, // Default: no MSYS2
		msvcEnvs:         "",    // Default: no MSVC setup
		workDir:          "",    // Default: inherit parent working directory
		logPath:          "",    // Default: no logging
		retryMaxAttempts: 0,     // Default: no retry
	}
}

// MSYS2Env enables/disables MSYS2 environment mode (Windows only).
func (e *executor) MSYS2Env(msys2Env bool) *executor {
	e.msys2Env = msys2Env
	return e
}

// SetMsvcEnvs sets MSVC environment configuration string (Windows only).
func (e *executor) SetMsvcEnvs(msvcEnvs string) *executor {
	e.msvcEnvs = msvcEnvs
	return e
}

// SetWorkDir sets the working directory for command execution.
func (e *executor) SetWorkDir(workDir string) *executor {
	e.workDir = workDir
	return e
}

// SetLogPath sets the file path for execution logs (environment variables, command line, output).
func (e *executor) SetLogPath(logPath string) *executor {
	e.logPath = logPath
	return e
}

// WithRetry enables automatic retry on command failure.
// maxAttempts controls how many times the command will be executed total (1 initial + N retries).
// A retry sleep of attempt * time.Second is applied between attempts.
func (e *executor) WithRetry(maxAttempts int) *executor {
	e.retryMaxAttempts = maxAttempts
	return e
}

// ExecuteOutput executes the command quietly and returns its combined stdout/stderr as a string.
func (e *executor) ExecuteOutput() (string, error) {
	var output fileio.LockedBuffer
	err := e.executeWithRetry(&output)
	return strings.TrimSpace(output.String()), err
}

// ExecuteOutputLive executes the command with live terminal output and returns its combined stdout/stderr as a string.
func (e *executor) ExecuteOutputLive() (string, error) {
	var output fileio.LockedBuffer
	err := e.executeWithRetry(outputCapture{output: &output})
	return strings.TrimSpace(output.String()), err
}

// Execute runs the command with live terminal output and no returned output.
func (e *executor) Execute() error {
	return e.executeWithRetry(nil)
}

// executeWithRetry calls doExecute, retrying up to retryMaxAttempts times on failure.
func (e *executor) executeWithRetry(output io.Writer) error {
	if e.retryMaxAttempts <= 1 {
		return e.doExecute(output)
	}

	var lastErr error
	for attempt := 1; attempt <= e.retryMaxAttempts; attempt++ {
		if attempt > 1 {
			// Reset buffer, to make sure the output buffer is clean after new retry.
			if r, ok := output.(resetable); ok {
				r.Reset()
			}
		}
		lastErr = e.doExecute(output)
		if lastErr == nil {
			return nil
		}

		color.Printf(color.Warning, "Attempted (%d/%d): %v\n", attempt, e.retryMaxAttempts, lastErr)
		if attempt < e.retryMaxAttempts {
			time.Sleep(time.Duration(attempt) * time.Second)
		}
	}

	// Show mesage without "[]".
	e.title = strings.Trim(e.title, "[]")
	return fmt.Errorf("%s failed after %d attempts -> %w", e.title, e.retryMaxAttempts, lastErr)
}

// createLogFile creates and initializes a log file with environment variables and command info.
func (e *executor) createLogFile(cmd *exec.Cmd) (*os.File, error) {
	if e.logPath == "" {
		return nil, nil
	}

	// Create directory hierarchy if needed.
	if err := os.MkdirAll(filepath.Dir(e.logPath), os.ModePerm); err != nil {
		return nil, fmt.Errorf("failed to create log directory -> %w", err)
	}
	logFile, err := os.Create(e.logPath)
	if err != nil {
		return nil, fmt.Errorf("failed to create log file -> %w", err)
	}

	// Write environment variables.
	var buffer bytes.Buffer
	for _, envVar := range cmd.Env {
		fmt.Fprintf(&buffer, "%s\n", envVar)
	}

	// Write headers to log file.
	if _, err := io.WriteString(logFile, fmt.Sprintf("Environment:\n%s\n", buffer.String())); err != nil {
		logFile.Close()
		return nil, fmt.Errorf("failed to write environment to log -> %w", err)
	}

	commandLine := e.command
	if len(e.args) > 0 {
		commandLine += " " + strings.Join(e.args, " ")
	}

	if _, err := io.WriteString(logFile, fmt.Sprintf("%s: %s\n\n", e.title, commandLine)); err != nil {
		logFile.Close()
		return nil, fmt.Errorf("failed to write command to log -> %w", err)
	}

	return logFile, nil
}

// configureOutputs routes command output to appropriate destinations:
// - To stdout/stderr if output is nil (Execute)
// - To custom output only if provided (ExecuteOutput)
// - To stdout/stderr and custom output if outputCapture is provided (ExecuteOutputLive)
// - To log file if configured
func (e *executor) configureOutputs(cmd *exec.Cmd, logFile *os.File, output io.Writer) {
	outWriters := make([]io.Writer, 0, 3)
	errWriters := make([]io.Writer, 0, 3)

	if output != nil {
		// Custom output provided: route to it.
		// outputCapture also keeps live terminal output while capturing combined output.
		if _, ok := output.(outputCapture); ok {
			outWriters = append(outWriters, os.Stdout)
			errWriters = append(errWriters, os.Stderr)
		}
		outWriters = append(outWriters, output)
		errWriters = append(errWriters, output)
	} else {
		// No custom output: use stdout/stderr
		outWriters = append(outWriters, os.Stdout)
		errWriters = append(errWriters, os.Stderr)
	}

	// Always include log file if configured (in addition to primary output)
	if logFile != nil {
		outWriters = append(outWriters, logFile)
		errWriters = append(errWriters, logFile)
	}

	cmd.Stdout = e.composeWriters(outWriters...)
	cmd.Stderr = e.composeWriters(errWriters...)
}

// composeWriters combines multiple Writers into one using io.MultiWriter.
// Returns the single writer directly if only one is provided.
func (e *executor) composeWriters(writers ...io.Writer) io.Writer {
	if len(writers) == 1 {
		return writers[0]
	}
	return io.MultiWriter(writers...)
}

// outputCapture this is a output that keeps live
// terminal output while capturing combined output.
type outputCapture struct {
	output io.Writer
}

func (t outputCapture) Write(p []byte) (int, error) {
	return t.output.Write(p)
}

// Reset delegates to the wrapped output if it supports resetting,
// so retry loops can clear captured data between attempts.
func (t outputCapture) Reset() {
	if r, ok := t.output.(resetable); ok {
		r.Reset()
	}
}

type resetable interface {
	Reset()
}
