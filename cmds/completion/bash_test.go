package completion

import (
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"testing"

	"celer/pkgs/dirs"

	"github.com/spf13/cobra"
)

func TestInstallAndUninstall_Bash(t *testing.T) {
	if runtime.GOOS == "windows" {
		t.Skip("bash is not supported on Windows")
	}

	var fileExists = func(path string) bool {
		_, err := os.Stat(path)
		return err == nil
	}

	home := t.TempDir()

	// Set dirs.TmpFilesDir to a temp location specific for the test
	origTmp := dirs.TmpFilesDir
	tmpRoot := filepath.Join(t.TempDir(), "tmpfiles")
	dirs.TmpFilesDir = tmpRoot
	defer func() { dirs.TmpFilesDir = origTmp }()

	rootCmd := &cobra.Command{
		Use: "celer",
	}
	b := NewBashCompletion(home, rootCmd)

	// Ensure clean tmp dir exists
	if err := dirs.CleanTmpFilesDir(); err != nil {
		t.Fatalf("CleanTmpFilesDir failed: %v", err)
	}

	// Install completion (should generate file in tmp then move it to destination)
	if err := b.installCompletion(); err != nil {
		t.Fatalf("installCompletion failed: %v", err)
	}

	dest := filepath.Join(home, ".local", "share", "bash-completion", "completions", "celer")
	if !fileExists(dest) {
		t.Fatalf("expected completion file at %s to exist", dest)
	}

	// Uninstall completion (should remove file and attempt to remove empty parent)
	if err := b.uninstallCompletion(); err != nil {
		t.Fatalf("uninstallCompletion failed: %v", err)
	}
	if fileExists(dest) {
		t.Fatalf("expected completion file to be removed at %s", dest)
	}

	// parent dir might be removed; ensure it does not contain the file
	parent := filepath.Dir(dest)
	if fileExists(parent) {
		// If parent still exists ensure it's not containing our file
		if fileExists(filepath.Join(parent, "celer")) {
			t.Fatalf("completion file still present after uninstall")
		}
	}
}

func TestRegisterAndUnregisterRunCommand_Bash(t *testing.T) {
	home := t.TempDir()
	b := NewBashCompletion(home, &cobra.Command{Use: "celer"})

	// prepare a .bashrc without the register line
	bashrc := filepath.Join(home, ".bashrc")
	if err := os.WriteFile(bashrc, []byte("export LANG=C\n"), 0644); err != nil {
		t.Fatalf("failed to write .bashrc: %v", err)
	}

	// Register should append the registerBinary line
	if err := b.registerRunCommand(); err != nil {
		t.Fatalf("registerRunCommand failed: %v", err)
	}
	data, err := os.ReadFile(bashrc)
	if err != nil {
		t.Fatalf("failed to read .bashrc: %v", err)
	}
	if !strings.Contains(string(data), b.registerBinary) {
		t.Fatalf("expected .bashrc to contain register line, got: %s", string(data))
	}

	// Calling register again should not duplicate the line
	if err := b.registerRunCommand(); err != nil {
		t.Fatalf("second registerRunCommand failed: %v", err)
	}
	data, _ = os.ReadFile(bashrc)
	if count := strings.Count(string(data), b.registerBinary); count != 1 {
		t.Fatalf("expected register line to appear once, got %d", count)
	}

	// Now test unregister: create a .bashrc that has the register line plus other lines
	if err := os.WriteFile(bashrc, []byte("line1\n"+b.registerBinary+"\nline2\n"), 0644); err != nil {
		t.Fatalf("failed to write .bashrc for unregister test: %v", err)
	}
	if err := b.unregisterRunCommand(); err != nil {
		t.Fatalf("unregisterRunCommand failed: %v", err)
	}
	data, _ = os.ReadFile(bashrc)
	if strings.Contains(string(data), b.registerBinary) {
		t.Fatalf("expected register line to be removed, still present in: %s", string(data))
	}
	if !strings.Contains(string(data), "line1") || !strings.Contains(string(data), "line2") {
		t.Fatalf("expected other lines to be preserved, got: %s", string(data))
	}
}
