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

func TestZshRegisterAndUnregisterRunCommand_ZSH(t *testing.T) {
	home := t.TempDir()
	rootCmd := &cobra.Command{Use: "celer"}
	z := NewZshCompletion(home, rootCmd)

	// prepare a .zshrc that already contains the registerBinary line
	zshrc := filepath.Join(home, ".zshrc")
	initial := z.registerBinary + "\nexport LANG=C\n"
	if err := os.WriteFile(zshrc, []byte(initial), 0644); err != nil {
		t.Fatalf("failed to write .zshrc: %v", err)
	}

	// First register should add registerFpath exactly once
	if err := z.registerRunCommand(); err != nil {
		t.Fatalf("registerRunCommand failed: %v", err)
	}
	data, err := os.ReadFile(zshrc)
	if err != nil {
		t.Fatalf("failed to read .zshrc: %v", err)
	}
	if !strings.Contains(string(data), z.registerFpath) {
		t.Fatalf("expected .zshrc to contain registerFpath, got: %s", string(data))
	}

	// Calling register again should be idempotent (no duplicate registerFpath)
	if err := z.registerRunCommand(); err != nil {
		t.Fatalf("second registerRunCommand failed: %v", err)
	}
	data2, _ := os.ReadFile(zshrc)
	if strings.Count(string(data2), z.registerFpath) != 1 {
		t.Fatalf("expected registerFpath to appear once, got %d", strings.Count(string(data2), z.registerFpath))
	}

	// prepare .zshrc with registerFpath and other lines, then unregister
	if err := os.WriteFile(zshrc, []byte("line1\n"+z.registerFpath+"\nline2\n"), 0644); err != nil {
		t.Fatalf("failed to write .zshrc for unregister test: %v", err)
	}
	if err := z.unregisterRunCommand(); err != nil {
		t.Fatalf("unregisterRunCommand failed: %v", err)
	}
	data3, _ := os.ReadFile(zshrc)
	if strings.Contains(string(data3), z.registerFpath) {
		t.Fatalf("expected registerFpath to be removed, still present in: %s", string(data3))
	}
	if !strings.Contains(string(data3), "line1") || !strings.Contains(string(data3), "line2") {
		t.Fatalf("expected other lines to be preserved, got: %s", string(data3))
	}
}

func TestZshInstallAndUninstallCompletion_ZSH(t *testing.T) {
	if runtime.GOOS == "windows" {
		t.Skip("zsh is not supported in Windows")
	}

	var fileExists = func(path string) bool {
		_, err := os.Stat(path)
		return err == nil
	}

	home := t.TempDir()

	// override tmp dir used by completion code
	origTmp := dirs.TmpFilesDir
	tmpRoot := filepath.Join(t.TempDir(), "tmpfiles")
	dirs.TmpFilesDir = tmpRoot
	defer func() { dirs.TmpFilesDir = origTmp }()

	rootCmd := &cobra.Command{Use: "celer"}
	z := NewZshCompletion(home, rootCmd)

	// Ensure .zshrc exists and contains the registerBinary (registerRunCommand expects it)
	zshrc := filepath.Join(home, ".zshrc")
	if err := os.WriteFile(zshrc, []byte(z.registerBinary+"\n"), 0644); err != nil {
		t.Fatalf("failed to write .zshrc: %v", err)
	}

	// Install completion (generates tmp file, moves to destination, and calls registerRunCommand)
	if err := z.installCompletion(); err != nil {
		t.Fatalf("installCompletion failed: %v", err)
	}

	dest := filepath.Join(home, ".local", "share", "zsh", "site-functions", "_celer")
	if !fileExists(dest) {
		t.Fatalf("expected completion file at %s to exist", dest)
	}
	// .zshrc should now contain the registerFpath
	data, err := os.ReadFile(zshrc)
	if err != nil {
		t.Fatalf("failed to read .zshrc: %v", err)
	}
	if !strings.Contains(string(data), z.registerFpath) {
		t.Fatalf("expected .zshrc to contain registerFpath after install, got: %s", string(data))
	}

	// Uninstall completion should remove the completion file
	if err := z.uninstallCompletion(); err != nil {
		t.Fatalf("uninstallCompletion failed: %v", err)
	}
	if fileExists(dest) {
		t.Fatalf("expected completion file to be removed at %s", dest)
	}
}
