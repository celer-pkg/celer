package completion

import (
	"os"
	"path/filepath"
	"strings"
	"testing"

	"celer/pkgs/dirs"

	"github.com/spf13/cobra"
)

func TestPowerShellRegisterAndUnregisterRunCommand_PowerShell(t *testing.T) {
	var fileExists = func(path string) bool {
		_, err := os.Stat(path)
		return err == nil
	}

	// Prepare isolated USERPROFILE and tmp dir.
	userProfile := t.TempDir()
	if err := os.Setenv("USERPROFILE", userProfile); err != nil {
		t.Fatalf("failed to set USERPROFILE: %v", err)
	}

	origTmp := dirs.TmpFilesDir
	tmp := filepath.Join(t.TempDir(), "tmpfiles")
	dirs.TmpFilesDir = tmp
	defer func() { dirs.TmpFilesDir = origTmp }()

	// Ensure tmp dir is clean/created.
	if err := dirs.CleanTmpFilesDir(); err != nil {
		t.Fatalf("CleanTmpFilesDir failed: %v", err)
	}

	rootCmd := &cobra.Command{Use: "celer"}
	p := NewPowerShellCompletion(userProfile, rootCmd)

	// Install completion into tmp dir (generates celer_completion.ps1)
	if err := p.installCompletion(); err != nil {
		t.Fatalf("installCompletion failed: %v", err)
	}

	// Register run command should move tmp file to Modules and append to profile.
	if err := p.registerRunCommand(); err != nil {
		t.Fatalf("registerRunCommand failed: %v", err)
	}

	modulesDir := filepath.Join(userProfile, "Documents", "WindowsPowerShell", "Modules")
	celerRcFile := filepath.Join(modulesDir, "celer", "celer_completion.ps1")
	if !fileExists(celerRcFile) {
		t.Fatalf("expected completion file at %s to exist", celerRcFile)
	}

	profilePath := filepath.Join(filepath.Dir(modulesDir), "profile.ps1")
	data, err := os.ReadFile(profilePath)
	if err != nil {
		t.Fatalf("failed to read profile.ps1: %v", err)
	}
	if !strings.Contains(string(data), p.registerBinary) {
		t.Fatalf("expected profile to contain register line, got: %s", string(data))
	}

	// Calling registerRunCommand again should not duplicate the register line.
	if err := p.registerRunCommand(); err != nil {
		t.Fatalf("second registerRunCommand failed: %v", err)
	}
	data2, err := os.ReadFile(profilePath)
	if err != nil {
		t.Fatalf("failed to read profile.ps1 after second register: %v", err)
	}
	if strings.Count(string(data2), p.registerBinary) != 1 {
		t.Fatalf("expected register line to appear once, got %d", strings.Count(string(data2), p.registerBinary))
	}

	// Now test unregister: should remove the line (and file if empty)
	if err := p.unregisterRunCommand(); err != nil {
		t.Fatalf("unregisterRunCommand failed: %v", err)
	}
	// profile may be removed entirely or no longer contain the line
	if fileExists(profilePath) {
		data3, err := os.ReadFile(profilePath)
		if err != nil {
			t.Fatalf("failed to read profile.ps1 after unregister: %v", err)
		}
		if strings.Contains(string(data3), p.registerBinary) {
			t.Fatalf("expected register line to be removed, still present in: %s", string(data3))
		}
	}
}

func TestPowerShellInstallAndUninstallCompletion_PowerShell(t *testing.T) {
	var fileExists = func(path string) bool {
		_, err := os.Stat(path)
		return err == nil
	}

	userProfile := t.TempDir()
	if err := os.Setenv("USERPROFILE", userProfile); err != nil {
		t.Fatalf("failed to set USERPROFILE: %v", err)
	}

	origTmp := dirs.TmpFilesDir
	tmp := filepath.Join(t.TempDir(), "tmpfiles2")
	dirs.TmpFilesDir = tmp
	defer func() { dirs.TmpFilesDir = origTmp }()

	if err := dirs.CleanTmpFilesDir(); err != nil {
		t.Fatalf("CleanTmpFilesDir failed: %v", err)
	}

	rootCmd := &cobra.Command{Use: "celer"}
	p := NewPowerShellCompletion(userProfile, rootCmd)

	// Create the tmp completion file
	if err := p.installCompletion(); err != nil {
		t.Fatalf("installCompletion failed: %v", err)
	}

	// Move the tmp file into Modules via registerRunCommand
	if err := p.registerRunCommand(); err != nil {
		t.Fatalf("registerRunCommand failed: %v", err)
	}

	modulesDir := filepath.Join(userProfile, "Documents", "WindowsPowerShell", "Modules")
	celerDir := filepath.Join(modulesDir, "celer")
	if !fileExists(filepath.Join(celerDir, "celer_completion.ps1")) {
		t.Fatalf("expected completion file in modules dir")
	}

	// uninstallCompletion should remove the module directory
	if err := p.uninstallCompletion(); err != nil {
		t.Fatalf("uninstallCompletion failed: %v", err)
	}
	if fileExists(celerDir) {
		t.Fatalf("expected module dir to be removed after uninstallCompletion")
	}
}
