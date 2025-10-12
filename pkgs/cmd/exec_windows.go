//go:build windows

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
	"syscall"
)

type Executor struct {
	msys2Env bool
	title    string
	cmd      string
	args     []string
	msvcEnvs string
	workDir  string
	logPath  string
}

func NewExecutor(title string, cmd string, args ...string) *Executor {
	return &Executor{
		title:   title,
		cmd:     cmd,
		args:    args,
		workDir: "",
		logPath: "",
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

func (e *Executor) ExecuteOutput() (string, error) {
	var buffer bytes.Buffer
	if err := e.doExecute(&buffer); err != nil {
		return "", err
	}

	return buffer.String(), nil
}

func (e Executor) Execute() error {
	if err := e.doExecute(nil); err != nil {
		return err
	}

	return nil
}

func (e Executor) doExecute(buffer *bytes.Buffer) error {
	// Create command for windows and unix like.
	if e.title != "" {
		fmt.Print(color.Sprintf(color.Blue, "\n%s: %s\n", e.title, e.cmd+" "+strings.Join(e.args, " ")))
	}

	var cmd *exec.Cmd
	if e.msys2Env {
		var args []string
		args = append(args, e.msvcEnvs)
		args = append(args, e.cmd)

		cmd = exec.Command("bash", "-lc", strings.Join(args, " "))
		cmd.Env = append(os.Environ(),
			"MSYSTEM=MINGW64",              // Set environment as MinGW64.
			"CHERE_INVOKING=1",             // Preserve the current working directory.
			"MSYS=winsymlinks:nativestric", // Allow creating symblink in windows.
		)
	} else {
		if len(e.args) == 0 {
			cmd = exec.Command("cmd")
			cmd.SysProcAttr = &syscall.SysProcAttr{
				CmdLine:    fmt.Sprintf(`/c %s`, e.cmd),
				HideWindow: true,
			}
			cmd.Env = os.Environ()
		} else {
			cmd = exec.Command(e.cmd, e.args...)
		}
		cmd.Env = os.Environ()
	}

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
		io.WriteString(logFile, fmt.Sprintf("%s: %s\n\n", e.title, e.args))

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
