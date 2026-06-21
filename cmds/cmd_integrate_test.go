package cmds

import (
	"os"
	"path/filepath"
	"runtime"
	"testing"

	"github.com/celer-pkg/celer/completion"
	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"

	"github.com/spf13/cobra"
)

func TestIntegrateCmd_CommandStructure(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Test command creation.
	integrate := &integrateCmd{}
	celer := &configs.Celer{}

	cmd := integrate.Command(celer)

	if cmd.Use != "integrate" {
		t.Errorf("Expected Use to be 'integrate', got %s", cmd.Use)
	}

	if cmd.Short == "" {
		t.Error("Short description should not be empty")
	}

	if cmd.Long == "" {
		t.Error("Long description should not be empty")
	}

	// Test flags
	flag := cmd.Flags().Lookup("remove")
	if flag == nil {
		t.Error("--remove flag should be defined")
		return
	}

	if flag.DefValue != "false" {
		t.Errorf("Expected --remove default to be false, got %s", flag.DefValue)
	}
}

func TestIntegrateCmd_Completion(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	integrate := &integrateCmd{}
	cmd := &cobra.Command{}

	tests := []struct {
		name       string
		toComplete string
		expected   []string
	}{
		{
			name:       "empty_input",
			toComplete: "",
			expected:   []string{"--remove"},
		},
		{
			name:       "partial_flag",
			toComplete: "--rem",
			expected:   []string{"--remove"},
		},
		{
			name:       "full_flag",
			toComplete: "--remove",
			expected:   []string{"--remove"},
		},
		{
			name:       "no_match",
			toComplete: "--invalid",
			expected:   []string{},
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			suggestions, directive := integrate.completion(cmd, []string{}, test.toComplete)

			if len(suggestions) != len(test.expected) {
				t.Errorf("Expected %d suggestions, got %d", len(test.expected), len(suggestions))
			}

			for i, expected := range test.expected {
				if i >= len(suggestions) || suggestions[i] != expected {
					t.Errorf("Expected suggestion %s, got %s", expected, func() string {
						if i < len(suggestions) {
							return suggestions[i]
						}
						return "<missing>"
					}())
				}
			}

			if directive != cobra.ShellCompDirectiveNoFileComp {
				t.Errorf("Expected NoFileComp directive, got %d", directive)
			}
		})
	}
}

func runIntegrate(t *testing.T, args ...string) (homeDir string, stderr string, err error) {
	t.Helper()

	homeDir = t.TempDir()
	if runtime.GOOS == "windows" {
		t.Setenv("USERPROFILE", homeDir)
	} else {
		t.Setenv("HOME", homeDir)
		t.Setenv("SHELL", "/bin/bash")
		// Pre-create an empty .bashrc; bash.installBinary reads it before
		// appending the PATH export line and a missing rc is a hard error.
		if err := os.WriteFile(filepath.Join(homeDir, ".bashrc"), nil, 0o644); err != nil {
			t.Fatal(err)
		}
	}

	cmd := &integrateCmd{}
	stderr, err = runCommand(t, cmd.Command(configs.NewCeler()), args...)
	return homeDir, stderr, err
}

func integrateArtifacts(homeDir string) []string {
	if runtime.GOOS == "windows" {
		return []string{
			filepath.Join(homeDir, "AppData", "Local", "celer", "celer.exe"),
			filepath.Join(homeDir, "Documents", "WindowsPowerShell", "Modules", "celer", "celer_completion.ps1"),
		}
	}
	return []string{
		filepath.Join(homeDir, ".local", "bin", "celer"),
		filepath.Join(homeDir, ".local", "share", "bash-completion", "completions", "celer"),
	}
}

func TestIntegrateCmd_RunE_Register_E2E(t *testing.T) {
	if completion.CurrentShell() == completion.NotSupported && runtime.GOOS != "windows" {
		if _, err := os.Stat("/bin/bash"); err != nil {
			t.Skip("no usable bash on this host; integrate test requires bash/zsh/powershell")
		}
	}

	homeDir, stderr, err := runIntegrate(t)
	if err != nil {
		t.Fatalf("integrate should succeed: %v\nstderr:\n%s", err, stderr)
	}

	for _, p := range integrateArtifacts(homeDir) {
		if !fileio.PathExists(p) {
			t.Errorf("integrate should create %s", p)
		}
	}
}

func TestIntegrateCmd_RunE_Remove_E2E(t *testing.T) {
	if runtime.GOOS != "windows" {
		if _, err := os.Stat("/bin/bash"); err != nil {
			t.Skip("no usable bash on this host")
		}
	}

	// Register first (sets up the same temp home dir that --remove will tear down).
	homeDir := t.TempDir()
	if runtime.GOOS == "windows" {
		t.Setenv("USERPROFILE", homeDir)
	} else {
		t.Setenv("HOME", homeDir)
		t.Setenv("SHELL", "/bin/bash")
		if err := os.WriteFile(filepath.Join(homeDir, ".bashrc"), nil, 0o644); err != nil {
			t.Fatal(err)
		}
	}

	register := &integrateCmd{}
	if _, err := runCommand(t, register.Command(configs.NewCeler())); err != nil {
		t.Fatalf("register should succeed: %v", err)
	}

	remove := &integrateCmd{}
	if _, err := runCommand(t, remove.Command(configs.NewCeler()), "--remove"); err != nil {
		t.Fatalf("--remove should succeed: %v", err)
	}

	// After --remove, the binary and completion file should be gone.
	for _, path := range integrateArtifacts(homeDir) {
		if fileio.PathExists(path) {
			t.Errorf("integrate --remove should delete %s", path)
		}
	}
}

func TestIntegrateCmd_DetectShell_PanicSafe(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	oldCurrentShellFn := currentShellFn
	t.Cleanup(func() {
		currentShellFn = oldCurrentShellFn
	})

	currentShellFn = func() completion.ShellType {
		panic("unexpected panic")
	}

	integrate := &integrateCmd{}
	shell := integrate.detectShell()
	if shell != completion.NotSupported {
		t.Fatalf("expected NotSupported when shell detection panics, got: %v", shell)
	}

	if err := integrate.validateEnvironment(shell); err == nil {
		t.Fatal("validateEnvironment should return error for NotSupported shell")
	}
}
