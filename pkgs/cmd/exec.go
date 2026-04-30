package cmd

import (
	"bytes"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/pkgs/fileio"
)

type Executor struct {
	msys2Env     bool
	title        string
	command      string
	args         []string
	msvcEnvs     string
	workDir      string
	logPath      string
	mirrorOutput bool
}

func NewExecutor(title string, command string, args ...string) *Executor {
	return &Executor{
		title:   title,
		command: command,
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

func (e *Executor) SetMirrorOutput(mirrorOutput bool) *Executor {
	e.mirrorOutput = mirrorOutput
	return e
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
	var output fileio.LockedBuffer
	err := e.doExecute(&output)
	return output.String(), err
}

func (e Executor) Execute() error {
	return e.doExecute(nil)
}

func (e Executor) createLogFile(cmd *exec.Cmd) (*os.File, error) {
	if e.logPath == "" {
		return nil, nil
	}

	if err := os.MkdirAll(filepath.Dir(e.logPath), os.ModePerm); err != nil {
		return nil, err
	}
	logFile, err := os.Create(e.logPath)
	if err != nil {
		return nil, err
	}

	var buffer bytes.Buffer
	for _, envVar := range cmd.Env {
		buffer.WriteString(envVar + "\n")
	}
	io.WriteString(logFile, fmt.Sprintf("Environment:\n%s\n", buffer.String()))
	io.WriteString(logFile, fmt.Sprintf("%s: %s\n\n", e.title, e.command+" "+strings.Join(e.args, " ")))
	return logFile, nil
}

func (e Executor) configureOutputs(cmd *exec.Cmd, logFile *os.File, output io.Writer) {
	outWriters := make([]io.Writer, 0, 3)
	errWriters := make([]io.Writer, 0, 3)

	if output == nil {
		outWriters = append(outWriters, os.Stdout)
		errWriters = append(errWriters, os.Stderr)
	}
	if logFile != nil {
		outWriters = append(outWriters, logFile)
		errWriters = append(errWriters, logFile)
	}
	if output != nil {
		outWriters = append(outWriters, output)
		errWriters = append(errWriters, output)
	}
	if output != nil && e.mirrorOutput {
		outWriters = append(outWriters, os.Stdout)
		errWriters = append(errWriters, os.Stderr)
	}

	cmd.Stdout = e.composeWriters(outWriters...)
	cmd.Stderr = e.composeWriters(errWriters...)
}

func (e Executor) composeWriters(writers ...io.Writer) io.Writer {
	if len(writers) == 1 {
		return writers[0]
	}
	return io.MultiWriter(writers...)
}
