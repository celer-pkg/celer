package cmd

import (
	"os"
	"os/exec"
	"strings"
	"testing"

	"github.com/celer-pkg/celer/pkgs/fileio"
)

func TestNewExecutor_Defaults(t *testing.T) {
	executor := NewExecutor("test title", "echo", "hello")

	if executor.title != "test title" {
		t.Errorf("title = %q, want %q", executor.title, "test title")
	}
	if executor.command != "echo" {
		t.Errorf("command = %q, want %q", executor.command, "echo")
	}
	if len(executor.args) != 1 || executor.args[0] != "hello" {
		t.Errorf("args = %v, want [hello]", executor.args)
	}
	if executor.msys2Env {
		t.Error("msys2Env should default to false")
	}
	if executor.msvcEnvs != "" {
		t.Error("msvcEnvs should default to empty")
	}
	if executor.workDir != "" {
		t.Error("workDir should default to empty")
	}
	if executor.logPath != "" {
		t.Error("logPath should default to empty")
	}
}

func TestExecutor_BuilderChain(t *testing.T) {
	executor := NewExecutor("title", "cmd").
		MSYS2Env(true).
		SetMsvcEnvs("vcvars").
		SetWorkDir(os.TempDir()).
		SetLogPath("test.log")

	if !executor.msys2Env {
		t.Error("msys2Env should be true")
	}
	if executor.msvcEnvs != "vcvars" {
		t.Error("msvcEnvs should be 'vcvars'")
	}
	if executor.workDir != os.TempDir() {
		t.Error("workDir should be set")
	}
	if executor.logPath != "test.log" {
		t.Error("logPath should be 'test.log'")
	}
}

func TestExecutor_CreateLogFile(t *testing.T) {
	tmpDir := t.TempDir()
	logPath := tmpDir + "/test.log"

	executor := NewExecutor("test", "echo").SetLogPath(logPath)
	execCmd := exec.Command("echo")
	execCmd.Env = os.Environ()

	logFile, err := executor.createLogFile(execCmd)
	if err != nil {
		t.Fatalf("unexpected error: %v", err)
	}
	if logFile == nil {
		t.Fatal("expected log file to be created")
	}
	logFile.Close()

	if _, err := os.Stat(logPath); os.IsNotExist(err) {
		t.Fatal("log file should exist")
	}
}

func TestExecutor_CreateLogFileNoPath(t *testing.T) {
	executor := NewExecutor("test", "echo")
	execCmd := exec.Command("echo")
	execCmd.Env = os.Environ()

	logFile, err := executor.createLogFile(execCmd)
	if err != nil {
		t.Fatalf("unexpected error: %v", err)
	}
	if logFile != nil {
		t.Fatal("expected nil log file when logPath is empty")
	}
}

func TestExecutor_CreateLogFileWithEnv(t *testing.T) {
	tmpDir := t.TempDir()
	logPath := tmpDir + "/test.log"

	executor := NewExecutor("test", "echo").SetLogPath(logPath)
	execCmd := exec.Command("echo")
	execCmd.Env = os.Environ()

	logFile, err := executor.createLogFile(execCmd)
	if err != nil {
		t.Fatalf("unexpected error: %v", err)
	}
	logFile.Close()

	data, err := os.ReadFile(logPath)
	if err != nil {
		t.Fatalf("failed to read log file: %v", err)
	}
	content := string(data)
	if !strings.Contains(content, "Environment:") {
		t.Error("log file should contain 'Environment:' header")
	}
	if !strings.Contains(content, "test:") {
		t.Error("log file should contain title header")
	}
}

func TestExecutor_ConfigureOutputs_NilOutput(t *testing.T) {
	executor := NewExecutor("test", "echo")
	execCmd := exec.Command("echo")

	executor.configureOutputs(execCmd, nil, nil)

	if execCmd.Stdout == nil {
		t.Error("stdout should not be nil")
	}
	if execCmd.Stderr == nil {
		t.Error("stderr should not be nil")
	}
}

func TestExecutor_ConfigureOutputs_WithOutput(t *testing.T) {
	executor := NewExecutor("test", "echo")
	execCmd := exec.Command("echo")
	var buf fileio.LockedBuffer

	executor.configureOutputs(execCmd, nil, &buf)

	if execCmd.Stdout == nil {
		t.Error("stdout should not be nil")
	}
	if execCmd.Stderr == nil {
		t.Error("stderr should not be nil")
	}
}

func TestExecutor_ConfigureOutputs_WithLogFile(t *testing.T) {
	tmpDir := t.TempDir()
	logPath := tmpDir + "/test.log"

	executor := NewExecutor("test", "echo").SetLogPath(logPath)
	execCmd := exec.Command("echo")
	execCmd.Env = os.Environ()

	logFile, err := executor.createLogFile(execCmd)
	if err != nil {
		t.Fatalf("unexpected error: %v", err)
	}
	defer logFile.Close()

	executor.configureOutputs(execCmd, logFile, nil)

	if execCmd.Stdout == nil {
		t.Error("stdout should not be nil")
	}
	if execCmd.Stderr == nil {
		t.Error("stderr should not be nil")
	}
}

func TestExecutor_ComposeWriters_Single(t *testing.T) {
	executor := NewExecutor("", "")
	w := &strings.Builder{}
	result := executor.composeWriters(w)
	if result != w {
		t.Error("single writer should be returned directly")
	}
}

func TestExecutor_ComposeWriters_Multiple(t *testing.T) {
	executor := NewExecutor("", "")
	w1 := &strings.Builder{}
	w2 := &strings.Builder{}
	result := executor.composeWriters(w1, w2)
	if result == nil {
		t.Error("expected non-nil multi-writer")
	}
}
