package completion

import (
	"bufio"
	"bytes"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/spf13/cobra"
)

type bash struct {
	homeDir        string
	rootCmd        *cobra.Command
	registerBinary string
}

func (b bash) Register() error {
	if err := b.installBinary(); err != nil {
		return fmt.Errorf("failed to install bash binary.\n %w", err)
	}
	if err := b.installCompletion(); err != nil {
		return fmt.Errorf("failed to install bash completion.\n %w", err)
	}
	if err := b.registerRunCommand(); err != nil {
		return fmt.Errorf("failed to add run command to bashrc.\n %w", err)
	}

	return nil
}

func (b bash) Unregister() error {
	if err := b.uninstallBinary(); err != nil {
		return fmt.Errorf("failed to uninstall bash binary.\n %w", err)
	}
	if err := b.uninstallCompletion(); err != nil {
		return fmt.Errorf("failed to uninstall bash completion.\n %w", err)
	}
	if err := b.unregisterRunCommand(); err != nil {
		return fmt.Errorf("failed to remove run command from bashrc.\n %w", err)
	}

	return nil
}

func (b bash) installBinary() error {
	executable, err := os.Executable()
	if err != nil {
		return fmt.Errorf("failed to get celer's path.\n %w", err)
	}

	// Copy into `~/.local/bin`
	if err := fileio.CopyFile(executable, filepath.Join(b.homeDir, ".local/bin/celer")); err != nil {
		return fmt.Errorf("failed to copy celer to ~/.local/bin.\n %w", err)
	}

	// Check if already contains the line.
	bashrcPath := filepath.Join(b.homeDir, ".bashrc")
	content, err := os.ReadFile(bashrcPath)
	if err != nil {
		return fmt.Errorf("failed to read ~/.bashrc.\n %w", err)
	}
	if strings.Contains(string(content), b.registerBinary) {
		return nil
	}

	// Append to `export PATH=~/.local/bin:$PATH` to end of .bashrc
	file, err := os.OpenFile(bashrcPath, os.O_APPEND|os.O_CREATE|os.O_WRONLY, os.ModeAppend)
	if err != nil {
		return fmt.Errorf("failed to open ~/.bashrc: %w", err)
	}
	defer file.Close()

	if _, err := file.WriteString("\n" + b.registerBinary); err != nil {
		return fmt.Errorf("failed to write to ~/.bashrc.\n %w", err)
	}

	fmt.Println("[integrate] celer -> ~/.local/bin")
	return nil
}

func (b bash) installCompletion() error {
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

	if err := b.rootCmd.GenBashCompletion(file); err != nil {
		return fmt.Errorf("failed to generate bash completion file.\n %w", err)
	}

	// Install completion file to `~/.local/share/bash-completion/completions`
	destination := filepath.Join(b.homeDir, ".local", "share", "bash-completion", "completions", "celer")
	if err := os.MkdirAll(filepath.Dir(destination), os.ModePerm); err != nil {
		return err
	}
	if err := fileio.MoveFile(filePath, destination); err != nil {
		return err
	}

	fmt.Printf("[integrate] completion -> %s\n", destination)
	return nil
}

func (b bash) uninstallCompletion() error {
	destination := filepath.Join(b.homeDir, ".local", "share", "bash-completion", "completions", "celer")
	if err := os.Remove(destination); err != nil {
		return fmt.Errorf("failed to remove bash completion file.\n %w", err)
	}
	fmt.Printf("[integrate] rm -f %s\n", destination)

	// Remove empty parent folder.
	if err := fileio.RemoveFolderRecursively(filepath.Dir(destination)); err != nil {
		return fmt.Errorf("failed to remove empty parent folder of celer.\n %w", err)
	}

	return nil
}

func (b bash) uninstallBinary() error {
	if err := os.Remove(filepath.Join(b.homeDir, ".local/bin/celer")); err != nil {
		return fmt.Errorf("failed to remove celer binary.\n %w", err)
	}
	fmt.Println("[integrate] rm -f ~/.local/bin/celer")

	return nil
}

func (b bash) registerRunCommand() error {
	bashrcPath := filepath.Join(b.homeDir, ".bashrc")
	if !fileio.PathExists(bashrcPath) {
		return fmt.Errorf("no .bashrc file found in home dir")
	}

	// Check if already contains the line.
	content, err := os.ReadFile(bashrcPath)
	if err != nil {
		return fmt.Errorf("failed to read ~/.bashrc.\n %w", err)
	}
	if strings.Contains(string(content), b.registerBinary) {
		return nil
	}

	// Append to end of .bashrc
	file, err := os.OpenFile(bashrcPath, os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
	if err != nil {
		return fmt.Errorf("failed to open ~/.bashrc: %w", err)
	}
	defer file.Close()

	if _, err := file.WriteString("\n" + b.registerBinary); err != nil {
		return fmt.Errorf("failed to write to ~/.bashrc.\n %w", err)
	}
	return nil
}

func (b bash) unregisterRunCommand() error {
	// Check if .bashrc exists
	bashrcPath := filepath.Join(b.homeDir, ".bashrc")
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
		if line != b.registerBinary {
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

func NewBashCompletion(homeDir string, rootCmd *cobra.Command) bash {
	return bash{
		homeDir:        homeDir,
		rootCmd:        rootCmd,
		registerBinary: "export PATH=$HOME/.local/bin:$PATH # added by celer",
	}
}
