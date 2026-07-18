//go:build windows

package cmd

import (
	"strings"
	"testing"
)

func TestBuildMSYS2Command_EnvVars(t *testing.T) {
	var displayCmd string
	executor := NewExecutor("", "make -j 4")
	cmd := executor.buildMSYS2Command(&displayCmd)

	found := map[string]bool{
		"MSYSTEM=MINGW64":               false,
		"CHERE_INVOKING=1":              false,
		"MSYS=winsymlinks:nativestrict": false,
	}
	for _, env := range cmd.Env {
		for key := range found {
			if env == key {
				found[key] = true
			}
		}
	}
	for key, ok := range found {
		if !ok {
			t.Errorf("missing env var: %s", key)
		}
	}
}

func TestBuildMSYS2Command_WithMsvcEnvs(t *testing.T) {
	var displayCmd string
	executor := NewExecutor("", "make -j 4").SetWinEnvs(`call "C:\VC\vcvarsall.bat" x64 > nul &&`)
	cmd := executor.buildMSYS2Command(&displayCmd)

	if !strings.Contains(displayCmd, executor.winEnvs) {
		t.Errorf("displayCmd should contain msvcEnvs, got: %s", displayCmd)
	}
	if !strings.Contains(displayCmd, executor.command) {
		t.Errorf("displayCmd should contain command, got: %s", displayCmd)
	}
	if !strings.HasPrefix(displayCmd, executor.winEnvs) {
		t.Errorf("msvcEnvs should be at the beginning of displayCmd, got: %s", displayCmd)
	}

	if len(cmd.Args) < 3 {
		t.Fatalf("expected at least 3 args for bash -lc, got: %v", cmd.Args)
	}
	if cmd.Args[0] != "bash" {
		t.Errorf("expected first arg to be bash, got: %s", cmd.Args[0])
	}
	if cmd.Args[1] != "-lc" {
		t.Errorf("expected second arg to be -lc, got: %s", cmd.Args[1])
	}
	combined := cmd.Args[2]
	if !strings.Contains(combined, executor.winEnvs) {
		t.Errorf("bash command should contain msvcEnvs, got: %s", combined)
	}
	if !strings.Contains(combined, executor.command) {
		t.Errorf("bash command should contain command, got: %s", combined)
	}
}

func TestBuildMSYS2Command_WithoutMsvcEnvs(t *testing.T) {
	var displayCmd string
	executor := NewExecutor("", "make -j 4")
	cmd := executor.buildMSYS2Command(&displayCmd)

	if displayCmd != executor.command {
		t.Errorf("displayCmd should equal command when msvcEnvs is empty, got: %s", displayCmd)
	}
	if len(cmd.Args) < 3 {
		t.Fatalf("expected at least 3 args, got: %v", cmd.Args)
	}
	if cmd.Args[2] != executor.command {
		t.Errorf("bash command should equal command, got: %s", cmd.Args[2])
	}
}

func TestBuildMSYS2Command_WithArgs(t *testing.T) {
	var displayCmd string
	executor := NewExecutor("", "cmake", "--build", ".", "--config", "Release")
	cmd := executor.buildMSYS2Command(&displayCmd)

	expected := "cmake --build . --config Release"
	if displayCmd != expected {
		t.Errorf("displayCmd = %q, want %q", displayCmd, expected)
	}
	if len(cmd.Args) < 3 {
		t.Fatalf("expected at least 3 args, got: %v", cmd.Args)
	}
	if cmd.Args[2] != expected {
		t.Errorf("bash command = %q, want %q", cmd.Args[2], expected)
	}
}

func TestBuildMSYS2Command_EnvNotOverridden(t *testing.T) {
	var displayCmd string
	executor := NewExecutor("", "make").SetWinEnvs(`call vcvarsall.bat &&`)
	cmd := executor.buildMSYS2Command(&displayCmd)

	msysCount := 0
	for _, env := range cmd.Env {
		if strings.HasPrefix(env, "MSYSTEM=") {
			msysCount++
		}
	}
	if msysCount != 1 {
		t.Errorf("expected exactly 1 MSYSTEM env var, got %d", msysCount)
	}
}

func TestBuildMSYS2Command_MsvcEnvsOrder(t *testing.T) {
	executor := NewExecutor("", "make install", "DESTDIR=/tmp").
		SetWinEnvs(`call "C:\VC\vcvarsall.bat" x64 > nul &&`)

	var displayCmd string
	cmd := executor.buildMSYS2Command(&displayCmd)

	parts := strings.SplitN(displayCmd, " ", 2)
	if !strings.HasPrefix(parts[0], "call") {
		t.Errorf("msvcEnvs should come first in displayCmd, got: %s", displayCmd)
	}

	bashCmd := cmd.Args[2]
	msvcIdx := strings.Index(bashCmd, "call")
	cmdIdx := strings.Index(bashCmd, "make install")
	if msvcIdx > cmdIdx {
		t.Errorf("msvcEnvs should appear before command in bash command, got: %s", bashCmd)
	}
}

func TestBuildNativeCommand_NoArgs(t *testing.T) {
	var displayCmd string
	executor := NewExecutor("", `call "C:\VC\vcvarsall.bat" x64 && cmake --build .`)
	cmd := executor.buildNativeCommand(&displayCmd)

	if displayCmd != executor.command {
		t.Errorf("displayCmd = %q, want %q", displayCmd, executor.command)
	}
	if cmd.Path != "" && !strings.HasSuffix(strings.ToLower(cmd.Path), "cmd.exe") {
		t.Errorf("expected cmd.exe, got: %s", cmd.Path)
	}
	if cmd.SysProcAttr == nil {
		t.Fatal("expected SysProcAttr to be set")
	}
	if cmd.SysProcAttr.CmdLine != `/c call "C:\VC\vcvarsall.bat" x64 && cmake --build .` {
		t.Errorf("CmdLine = %q, unexpected value", cmd.SysProcAttr.CmdLine)
	}
	if !cmd.SysProcAttr.HideWindow {
		t.Error("expected HideWindow to be true")
	}
}

func TestBuildNativeCommand_WithArgs(t *testing.T) {
	var displayCmd string
	executor := NewExecutor("", "cmake", "--build", ".", "--config", "Release")
	cmd := executor.buildNativeCommand(&displayCmd)

	expected := "cmake --build . --config Release"
	if displayCmd != expected {
		t.Errorf("displayCmd = %q, want %q", displayCmd, expected)
	}
	if cmd.SysProcAttr != nil {
		t.Error("expected SysProcAttr to be nil for command with args")
	}
	if len(cmd.Args) < 5 {
		t.Fatalf("expected at least 5 args, got: %v", cmd.Args)
	}
	if cmd.Args[0] != "cmake" {
		t.Errorf("first arg = %q, want %q", cmd.Args[0], "cmake")
	}
}

func TestBuildNativeCommand_EnvSet(t *testing.T) {
	var displayCmd string
	executor := NewExecutor("", "echo", "hello")
	cmd := executor.buildNativeCommand(&displayCmd)

	if len(cmd.Env) == 0 {
		t.Error("expected cmd.Env to be set from os.Environ()")
	}
}
