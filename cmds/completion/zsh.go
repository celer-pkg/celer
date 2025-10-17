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

type ZshCompletion struct {
	registerLine string
}

func (z ZshCompletion) Register(homeDir string) error {
	if err := z.installBinary(homeDir); err != nil {
		return fmt.Errorf("failed to install zsh binary.\n %w", err)
	}
	if err := z.installCompletion(nil, homeDir); err != nil {
		return fmt.Errorf("failed to install zsh completion.\n %w", err)
	}
	if err := z.registerRunCommand(homeDir); err != nil {
		return fmt.Errorf("failed to add run command to zshrc.\n %w", err)
	}

	return nil
}

func (z ZshCompletion) Unregister(homeDir string) error {
	if err := z.uninstallBinary(homeDir); err != nil {
		return fmt.Errorf("failed to uninstall zsh binary.\n %w", err)
	}
	if err := z.uninstallCompletion(homeDir); err != nil {
		return fmt.Errorf("failed to uninstall zsh completion.\n %w", err)
	}
	if err := z.unregisterRunCommand(homeDir); err != nil {
		return fmt.Errorf("failed to remove run command from zshrc.\n %w", err)
	}

	return nil
}

func (z ZshCompletion) installBinary(homeDir string) error {
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

func (z ZshCompletion) installCompletion(cmd *cobra.Command, homeDir string) error {
	if runtime.GOOS != "linux" && runtime.GOOS != "darwin" {
		return fmt.Errorf("zsh completion is only supported on linux and macOS")
	}

	if err := dirs.CleanTmpFilesDir(); err != nil {
		return fmt.Errorf("failed to create clean tmp dir.\n %w", err)
	}

	// Generate completion file.
	filePath := filepath.Join(dirs.TmpFilesDir, "celer")
	file, err := os.Create(filePath)
	if err != nil {
		return fmt.Errorf("failed to create zsh completion file.\n %w", err)
	}
	defer file.Close()

	if err := cmd.GenZshCompletion(file); err != nil {
		return fmt.Errorf("failed to generate zsh completion file.\n %w", err)
	}

	// Install completion file to `~/.local/share/zsh/site-functions/_celer`
	destination := filepath.Join(homeDir, ".local", "share", "zsh", "site-functions", "_celer")
	if err := os.MkdirAll(filepath.Dir(destination), os.ModePerm); err != nil {
		return err
	}
	if err := fileio.MoveFile(filePath, destination); err != nil {
		return err
	}

	if err := z.registerRunCommand(homeDir); err != nil {
		return err
	}

	fmt.Printf("[integrate] completion --> %s\n", destination)
	return nil
}

func (z ZshCompletion) uninstallCompletion(homeDir string) error {
	fmt.Println("[integrate] rm -f ~/.local/share/zsh/site-functions/_celer")
	if err := os.Remove(filepath.Join(homeDir, ".local/share/zsh/site-functions/_celer")); err != nil {
		return fmt.Errorf("failed to remove zsh completion file.\n %w", err)
	}

	return nil
}

func (z ZshCompletion) uninstallBinary(homeDir string) error {
	if runtime.GOOS != "linux" && runtime.GOOS != "darwin" {
		return fmt.Errorf("zsh completion is only supported on linux and macOS")
	}

	// Remove celer binary.
	fmt.Println("[integrate] rm -f ~/.local/bin/celer")
	if err := os.Remove(filepath.Join(homeDir, ".local/bin/celer")); err != nil {
		return fmt.Errorf("failed to remove celer binary.\n %w", err)
	}

	return nil
}

func (z ZshCompletion) registerRunCommand(homeDir string) error {
	zshrcPath := filepath.Join(homeDir, ".zshrc")
	if !fileio.PathExists(zshrcPath) {
		return fmt.Errorf("no .zshrc file found in home dir")
	}

	// Check if already contains the line.
	content, err := os.ReadFile(zshrcPath)
	if err != nil {
		return fmt.Errorf("failed to read ~/.zshrc.\n %w", err)
	}
	if strings.Contains(string(content), z.registerLine) {
		return nil
	}

	// Append to end of .bashrc
	file, err := os.OpenFile(zshrcPath, os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
	if err != nil {
		return fmt.Errorf("failed to open ~/.bashrc: %w", err)
	}
	defer file.Close()

	if _, err := file.WriteString("\n" + z.registerLine); err != nil {
		return fmt.Errorf("failed to write to ~/.bashrc.\n %w", err)
	}

	return nil
}

func (z ZshCompletion) unregisterRunCommand(homeDir string) error {
	// Check if .zshrc exists
	zshrcPath := filepath.Join(homeDir, ".zshrc")
	if !fileio.PathExists(zshrcPath) {
		return fmt.Errorf("no .zshrc file found in home dir")
	}

	// Open .zshrc file.
	file, err := os.Open(zshrcPath)
	if err != nil {
		return err
	}
	defer file.Close()

	// Read line by line and filter out the register line.
	var buffer bytes.Buffer
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := scanner.Text()
		if line != z.registerLine {
			buffer.WriteString(line + "\n")
		}
	}
	if err := scanner.Err(); err != nil {
		return err
	}

	// Write back to .zshrc
	if err := os.WriteFile(zshrcPath, buffer.Bytes(), os.ModePerm); err != nil {
		return err
	}

	return nil
}

func NewZshCompletion() ZshCompletion {
	return ZshCompletion{
		registerLine: "fpath=(~/.local/share/zsh/site-functions $fpath) # added by celer",
	}
}
