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

func (e Executor) doExecute(output io.Writer) error {
	var cmd *exec.Cmd
	var message string

	if e.msys2Env {
		var args []string
		args = append(args, e.msvcEnvs)
		args = append(args, e.command)
		message = strings.Join(args, " ")

		cmd = exec.Command("bash", "-lc", strings.Join(args, " "))
		cmd.Env = append(os.Environ(),
			"MSYSTEM=MINGW64",
			"CHERE_INVOKING=1",
			"MSYS=winsymlinks:nativestric",
		)
	} else {
		message = e.command + " " + strings.Join(e.args, " ")
		if len(e.args) == 0 {
			cmd = exec.Command("cmd")
			cmd.SysProcAttr = &syscall.SysProcAttr{
				CmdLine:    fmt.Sprintf(`/c %s`, e.command),
				HideWindow: true,
			}
		} else {
			cmd = exec.Command(e.command, e.args...)
		}
		cmd.Env = os.Environ()
	}

	if e.workDir != "" && !fileio.PathExists(e.workDir) {
		return fmt.Errorf("work dir does not exist: %s", e.workDir)
	}

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

	if e.title != "" {
		color.Printf(color.Title, "\n%s\n", e.title)
		color.Printf(color.Hint, "▶ %s\n", message)
	}

	if err := cmd.Run(); err != nil {
		return err
	}

	return nil
}
