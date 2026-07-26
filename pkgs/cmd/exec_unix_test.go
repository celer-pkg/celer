//go:build darwin || netbsd || freebsd || openbsd || dragonfly || linux

package cmd

import (
	"os"
	"testing"
)

func TestExecutor_ExecuteOutput(t *testing.T) {
	executor := NewExecutor("echo test", "echo", "hello world")
	output, err := executor.ExecuteOutput()
	if err != nil {
		t.Fatalf("unexpected error: %v", err)
	}
	if output != "hello world\n" {
		t.Errorf("output = %q, want %q", output, "hello world\n")
	}
}

func TestExecutor_Execute(t *testing.T) {
	executor := NewExecutor("echo test", "echo", "hello")
	if err := executor.Execute(); err != nil {
		t.Fatalf("unexpected error: %v", err)
	}
}

func TestExecutor_ExecuteWithWorkDir(t *testing.T) {
	executor := NewExecutor("ls test", "ls").SetWorkDir(os.TempDir())
	if err := executor.Execute(); err != nil {
		t.Fatalf("unexpected error: %v", err)
	}
}

func TestExecutor_ExecuteInvalidWorkDir(t *testing.T) {
	executor := NewExecutor("ls test", "ls").SetWorkDir("/nonexistent/directory/path")
	err := executor.Execute()
	if err == nil {
		t.Fatal("expected error for nonexistent work dir")
	}
}

func TestExecutor_ExecuteNoArgs(t *testing.T) {
	executor := NewExecutor("echo test", "echo hello")
	output, err := executor.ExecuteOutput()
	if err != nil {
		t.Fatalf("unexpected error: %v", err)
	}
	if output != "hello\n" {
		t.Errorf("output = %q, want %q", output, "hello\n")
	}
}

func TestExecutor_ExecuteFailingCommand(t *testing.T) {
	executor := NewExecutor("false test", "false")
	err := executor.Execute()
	if err == nil {
		t.Fatal("expected error for failing command")
	}
}
