//go:build darwin || netbsd || freebsd || openbsd || dragonfly || linux

package cmd

import (
	"celer/pkgs/color"
	"io"
	"os"
	"os/exec"
	"strings"
)

func (e Executor) doExecute(output io.Writer) error {
	if e.title != "" {
		color.Printf(color.Title, "\n%s\n", e.title)
		color.Printf(color.Hint, "▶ %s\n", e.command+" "+strings.Join(e.args, " "))
	}

	var cmd *exec.Cmd
	if len(e.args) == 0 {
		cmd = exec.Command("bash", "-c", e.command)
	} else {
		cmd = exec.Command(e.command, e.args...)
	}

	cmd.Env = os.Environ()
	cmd.Dir = e.workDir
	cmd.Stdin = os.Stdin

	logFile, err := e.createLogFile(cmd)
	if err != nil {
		return err
	}
	if logFile != nil {
		defer logFile.Close()
	}

	e.configureOutputs(cmd, logFile, output)

	if err := cmd.Run(); err != nil {
		return err
	}

	return nil
}
