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

// func (e Executor) loadMSVCEnv() (string, error) {
// 	vcvars := `C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat`

// 	// Print all environment variables about msvc.
// 	var out bytes.Buffer
// 	cmd := exec.Command("cmd", "/c", "call", vcvars, "x64", "&&", "set")
// 	cmd.Stdout = &out
// 	cmd.Stderr = &out
// 	if err := cmd.Run(); err != nil {
// 		return "", fmt.Errorf("failed to call vcvarsall: %v\noutput: %s", err, out.String())
// 	}

// 	// Parse environment variables from output.
// 	var envs []string

// 	lines := strings.Split(out.String(), "\n")
// 	for _, line := range lines {
// 		line = strings.TrimSpace(line)
// 		if line != "" && strings.Contains(line, "=") {
// 			parts := strings.Split(line, "=")
// 			key := parts[0]
// 			value := parts[1]
// 			if !strings.Contains(key, "PATH=") {
// 				continue
// 			}

// 			if strings.Contains(value, ";") {
// 				parts := strings.Split(value, ";")
// 				for index, part := range parts {
// 					parts[index] = toCygpath(part)
// 				}
// 				envs = append(envs, fmt.Sprintf(`%s="%s"`, key, strings.Join(parts, ":")))
// 			} else {
// 				value = toCygpath(value)
// 				envs = append(envs, fmt.Sprintf(`%s="%s"`, key, value))
// 			}
// 		}
// 	}

// 	return strings.Join(envs, " "), nil
// }

// func toCygpath(path string) string {
// 	if runtime.GOOS == "windows" {
// 		path = filepath.Clean(path)
// 		path = filepath.ToSlash(path)

// 		// Handle disk driver（for example: `C:/` → `/c/`）
// 		if len(path) >= 2 && path[1] == ':' {
// 			drive := strings.ToLower(string(path[0]))
// 			path = "/" + drive + path[2:]
// 		}

// 		return path
// 	}

// 	return path
// }
