package completion

import (
	"bufio"
	"bytes"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/spf13/cobra"
)

type BashCompletion struct {
	registerLine string
}

func (b BashCompletion) Register(homeDir string) error {
	if err := b.installBinary(homeDir); err != nil {
		return fmt.Errorf("failed to install bash binary.\n %w", err)
	}
	if err := b.installCompletion(nil, homeDir); err != nil {
		return fmt.Errorf("failed to install bash completion.\n %w", err)
	}
	if err := b.registerRunCommand(homeDir); err != nil {
		return fmt.Errorf("failed to add run command to bashrc.\n %w", err)
	}

	return nil
}

func (b BashCompletion) Unregister(homeDir string) error {
	if err := b.uninstallBinary(homeDir); err != nil {
		return fmt.Errorf("failed to uninstall bash binary.\n %w", err)
	}
	if err := b.uninstallCompletion(homeDir); err != nil {
		return fmt.Errorf("failed to uninstall bash completion.\n %w", err)
	}
	if err := b.unregisterRunCommand(homeDir); err != nil {
		return fmt.Errorf("failed to remove run command from bashrc.\n %w", err)
	}

	return nil
}

func (b BashCompletion) installBinary(homeDir string) error {
	executable, err := os.Executable()
	if err != nil {
		return fmt.Errorf("failed to get celer's path.\n %w", err)
	}

	switch runtime.GOOS {
	case "linux", "darwin":
		// Copy into `~/.local/bin`
		if fileio.CopyFile(executable, filepath.Join(homeDir, ".local/bin")); err != nil {
			return fmt.Errorf("failed to copy celer to ~/.local/bin.\n %w", err)
		}

		fmt.Println("[integrate] celer --> ~/.local/bin")
	}
	return nil
}

func (b BashCompletion) installCompletion(cmd *cobra.Command, homeDir string) error {
	if runtime.GOOS != "linux" {
		return fmt.Errorf("bash completion is only supported on linux")
	}

	if err := dirs.CleanTmpFilesDir(); err != nil {
		return fmt.Errorf("failed to create clean tmp dir.\n %w", err)
	}

	// Generate completion file.
	filePath := filepath.Join(dirs.TmpFilesDir, "celer")
	file, err := os.Create(filePath)
	if err != nil {
		return fmt.Errorf("failed to create bash completion file.\n %w", err)
	}
	defer file.Close()

	if err := cmd.GenBashCompletion(file); err != nil {
		return fmt.Errorf("failed to generate bash completion file.\n %w", err)
	}

	// Install completion file to `~/.local/share/bash-completion/completions`
	destination := filepath.Join(homeDir, ".local", "share", "bash-completion", "completions", "celer")
	if err := os.MkdirAll(filepath.Dir(destination), os.ModePerm); err != nil {
		return err
	}
	if err := fileio.MoveFile(filePath, destination); err != nil {
		return err
	}

	fmt.Printf("[integrate] completion --> %s\n", destination)
	return nil
}

func (B BashCompletion) uninstallCompletion(homeDir string) error {
	if runtime.GOOS != "linux" {
		return fmt.Errorf("bash completion is only supported on linux")
	}

	// Remove completion file.
	destination := filepath.Join(homeDir, ".local", "share", "bash-completion", "completions", "celer")
	fmt.Printf("[integrate] rm -f %s\n", destination)
	if err := os.Remove(destination); err != nil {
		return fmt.Errorf("failed to remove bash completion file.\n %w", err)
	}

	return nil
}

func (b BashCompletion) uninstallBinary(homeDir string) error {
	if runtime.GOOS != "linux" {
		return fmt.Errorf("bash completion is only supported on linux")
	}

	// Remove celer binary.
	fmt.Println("[integrate] rm -f ~/.local/bin/celer")
	if err := os.Remove(filepath.Join(homeDir, ".local/bin/celer")); err != nil {
		return fmt.Errorf("failed to remove celer binary.\n %w", err)
	}

	return nil
}

func (b BashCompletion) registerRunCommand(homeDir string) error {
	bashrcPath := filepath.Join(homeDir, ".bashrc")
	if !fileio.PathExists(bashrcPath) {
		return fmt.Errorf("no .bashrc file found in home dir")
	}

	// Check if already contains the line.
	content, err := os.ReadFile(bashrcPath)
	if err != nil {
		return fmt.Errorf("failed to read ~/.bashrc.\n %w", err)
	}
	if strings.Contains(string(content), b.registerLine) {
		return nil
	}

	// Append to end of .bashrc
	file, err := os.OpenFile(bashrcPath, os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
	if err != nil {
		return fmt.Errorf("failed to open ~/.bashrc: %w", err)
	}
	defer file.Close()

	if _, err := file.WriteString("\n" + b.registerLine); err != nil {
		return fmt.Errorf("failed to write to ~/.bashrc.\n %w", err)
	}
	return nil
}

func (b BashCompletion) unregisterRunCommand(homeDir string) error {
	// Check if .bashrc exists
	bashrcPath := filepath.Join(homeDir, ".bashrc")
	if !fileio.PathExists(bashrcPath) {
		return fmt.Errorf("no .bashrc file found in home dir")
	}

	// Open .bashrc file.
	file, err := os.Open(bashrcPath)
	if err != nil {
		return err
	}
	defer file.Close()

	// Read line by line and filter out the register line.
	var buffer bytes.Buffer
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := scanner.Text()
		if line != b.registerLine {
			buffer.WriteString(line + "\n")
		}
	}
	if err := scanner.Err(); err != nil {
		return err
	}

	// Write back to .bashrc
	if err := os.WriteFile(bashrcPath, buffer.Bytes(), os.ModePerm); err != nil {
		return err
	}

	return nil
}

func NewBashCompletion() BashCompletion {
	return BashCompletion{
		registerLine: "export PATH=~/.local/bin:$PATH # added by celer",
	}
}
