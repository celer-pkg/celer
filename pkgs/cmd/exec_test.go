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
	if executor.winEnvs != "" {
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
		SetWinEnvs("vcvars").
		SetWorkDir(os.TempDir()).
		SetLogPath("test.log")

	if !executor.msys2Env {
		t.Error("msys2Env should be true")
	}
	if executor.winEnvs != "vcvars" {
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

func TestExecutor_WithRetry(t *testing.T) {
	executor := NewExecutor("title", "cmd").WithRetry(3)
	if executor.retryMaxAttempts != 3 {
		t.Errorf("retryMaxAttempts = %d, want 3", executor.retryMaxAttempts)
	}
}

func TestExecutor_WithRetry_Default(t *testing.T) {
	executor := NewExecutor("title", "cmd")
	if executor.retryMaxAttempts != 0 {
		t.Errorf("retryMaxAttempts = %d, want 0", executor.retryMaxAttempts)
	}
}

func TestExecutor_WithRetry_RetriesOnFailure(t *testing.T) {
	executor := NewExecutor("[test retry]", "false").WithRetry(3)
	err := executor.Execute()
	if err == nil {
		t.Fatal("expected error from failing command")
	}
	if !strings.Contains(err.Error(), "failed after 3 attempts") {
		t.Errorf("error should mention 'failed after 3 attempts', got: %v", err)
	}
}

func TestExecutor_WithRetry_NoRetryWhenZero(t *testing.T) {
	executor := NewExecutor("[test]", "false") // no WithRetry
	err := executor.Execute()
	if err == nil {
		t.Fatal("expected error from failing command")
	}
	if strings.Contains(err.Error(), "failed after") {
		t.Errorf("error should not mention retry when WithRetry not set, got: %v", err)
	}
}
