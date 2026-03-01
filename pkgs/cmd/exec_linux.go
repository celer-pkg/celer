//go:build darwin || netbsd || freebsd || openbsd || dragonfly || linux

package cmd

import (
	"bytes"
	"celer/pkgs/color"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"time"
)

type Executor struct {
	msys2Env   bool
	title      string
	cmd        string
	args       []string
	msvcEnvs   string
	workDir    string
	logPath    string
	maxRetries int
}

func NewExecutor(title string, cmd string, args ...string) *Executor {
	return &Executor{
		title:      title,
		cmd:        cmd,
		args:       args,
		workDir:    "",
		logPath:    "",
		maxRetries: 1,
	}
}

func (e *Executor) MSYS2Env(msys2Env bool) {
	e.msys2Env = msys2Env
}

func (e *Executor) SetMsvcEnvs(msvcEnvs string) {
	e.msvcEnvs = msvcEnvs
}

func (e *Executor) SetWorkDir(workDir string) *Executor {
	e.workDir = workDir
	return e
}

func (e *Executor) SetLogPath(logPath string) *Executor {
	e.logPath = logPath
	return e
}

func (e *Executor) SetMaxRetries(max int) *Executor {
	if max < 0 {
		panic("max retries should be greater or equals than 1")
	}
	e.maxRetries = max
	return e
}

func (e *Executor) ExecuteOutput() (string, error) {
	var lastErr error
	var buffer bytes.Buffer

	for attempt := 1; attempt <= e.maxRetries; attempt++ {
		err := e.doExecute(&buffer)
		if err == nil {
			return buffer.String(), nil
		}

		lastErr = err
		color.Printf(color.Warning, "-- %s (attempt %d/%d): %v\n", e.title, attempt, e.maxRetries, err)
		if attempt < e.maxRetries {
			time.Sleep(time.Duration(attempt) * time.Second) // Exponential backoff.
		}
	}

	return "", fmt.Errorf("%s failed after %d attempts -> %w", e.title, e.maxRetries, lastErr)
}

func (e Executor) Execute() error {
	if err := e.doExecute(nil); err != nil {
		return err
	}

	return nil
}

func (e Executor) doExecute(buffer *bytes.Buffer) error {
	if e.title != "" {
		color.Printf(color.Title, "\n%s: %s\n", e.title, e.cmd+" "+strings.Join(e.args, " "))
	}

	var cmd *exec.Cmd
	if len(e.args) == 0 {
		cmd = exec.Command("bash", "-c", e.cmd)
	} else {
		cmd = exec.Command(e.cmd, e.args...)
	}

	cmd.Env = os.Environ()
	cmd.Dir = e.workDir
	cmd.Stdin = os.Stdin

	// Create log file if log path specified.
	if e.logPath != "" {
		if err := os.MkdirAll(filepath.Dir(e.logPath), os.ModePerm); err != nil {
			return err
		}
		logFile, err := os.Create(e.logPath)
		if err != nil {
			return err
		}
		defer logFile.Close()

		// Write env variables to log file.
		var buffer bytes.Buffer
		for _, envVar := range cmd.Env {
			buffer.WriteString(envVar + "\n")
		}
		io.WriteString(logFile, fmt.Sprintf("Environment:\n%s\n", buffer.String()))

		// Write command summary as header content of file.
		io.WriteString(logFile, fmt.Sprintf("%s: %s\n\n", e.title, e.cmd)+" "+strings.Join(e.args, " "))

		cmd.Stdout = io.MultiWriter(os.Stdout, logFile)
		cmd.Stderr = io.MultiWriter(os.Stderr, logFile)
	} else if buffer != nil {
		cmd.Stdout = buffer
		cmd.Stderr = buffer
	} else {
		cmd.Stdout = os.Stdout
		cmd.Stderr = os.Stdout
	}

	if err := cmd.Run(); err != nil {
		return err
	}

	return nil
}
