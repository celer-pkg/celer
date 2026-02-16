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

type zsh struct {
	homeDir        string
	registerFpath  string
	registerBinary string
	rootCmd        *cobra.Command
}

func (z zsh) Register() error {
	if err := z.installBinary(); err != nil {
		return fmt.Errorf("failed to install zsh binary: %w", err)
	}
	if err := z.installCompletion(); err != nil {
		return fmt.Errorf("failed to install zsh completion: %w", err)
	}
	if err := z.registerRunCommand(); err != nil {
		return fmt.Errorf("failed to add run command to zshrc: %w", err)
	}

	return nil
}

func (z zsh) Unregister() error {
	if err := z.uninstallBinary(); err != nil {
		return fmt.Errorf("failed to uninstall zsh binary: %w", err)
	}
	if err := z.uninstallCompletion(); err != nil {
		return fmt.Errorf("failed to uninstall zsh completion: %w", err)
	}
	if err := z.unregisterRunCommand(); err != nil {
		return fmt.Errorf("failed to remove run command from zshrc: %w", err)
	}

	return nil
}

func (z zsh) installBinary() error {
	executable, err := os.Executable()
	if err != nil {
		return fmt.Errorf("failed to get celer's path: %w", err)
	}

	// Copy into `~/.local/bin`
	if err := fileio.CopyFile(executable, filepath.Join(z.homeDir, ".local/bin/celer")); err != nil {
		return fmt.Errorf("failed to copy celer to ~/.local/bin: %w", err)
	}
	fmt.Println("[integrate] celer --> ~/.local/bin")

	// Check if already contains the line.
	zshrcPath := filepath.Join(z.homeDir, ".zshrc")
	content, err := os.ReadFile(zshrcPath)
	if err != nil {
		return fmt.Errorf("failed to read ~/.zshrc: %w", err)
	}
	if strings.Contains(string(content), z.registerBinary) {
		return nil
	}

	// Append `export PATH=~/.local/bin:$PATH` to top of .zshrc
	file, err := os.ReadFile(zshrcPath)
	if err != nil && !os.IsNotExist(err) {
		return fmt.Errorf("failed to read ~/.zshrc: %w", err)
	}
	newContent := z.registerBinary + "\n" + string(file)
	if err := os.WriteFile(zshrcPath, []byte(newContent), os.ModePerm); err != nil {
		return fmt.Errorf("failed to write to ~/.zshrc: %w", err)
	}

	return nil
}

func (z zsh) installCompletion() error {
	if err := dirs.CleanTmpFilesDir(); err != nil {
		return fmt.Errorf("failed to create clean tmp dir: %w", err)
	}

	// Generate completion file.
	filePath := filepath.Join(dirs.TmpFilesDir, "celer")
	file, err := os.Create(filePath)
	if err != nil {
		return fmt.Errorf("failed to create zsh completion file: %w", err)
	}
	defer file.Close()

	if err := z.rootCmd.GenZshCompletion(file); err != nil {
		return fmt.Errorf("failed to generate zsh completion file: %w", err)
	}

	// Install completion file to `~/.local/share/zsh/site-functions/_celer`
	destination := filepath.Join(z.homeDir, ".local", "share", "zsh", "site-functions", "_celer")
	if err := os.MkdirAll(filepath.Dir(destination), os.ModePerm); err != nil {
		return err
	}
	if err := fileio.MoveFile(filePath, destination); err != nil {
		return err
	}

	if err := z.registerRunCommand(); err != nil {
		return err
	}

	fmt.Printf("[integrate] completion --> %s\n", destination)
	return nil
}

func (z zsh) uninstallCompletion() error {
	fmt.Println("[integrate] rm -f ~/.local/share/zsh/site-functions/_celer")
	completionFile := filepath.Join(z.homeDir, ".local/share/zsh/site-functions/_celer")
	if err := os.Remove(completionFile); err != nil {
		return fmt.Errorf("failed to remove zsh completion file: %w", err)
	}

	if err := fileio.RemoveFolderRecursively(filepath.Dir(completionFile)); err != nil {
		return fmt.Errorf("failed to remove empty parent folder of _zsh: %w", err)
	}

	return nil
}

func (z zsh) uninstallBinary() error {
	// Remove celer binary.
	fmt.Println("[integrate] rm -f ~/.local/bin/celer")
	if err := os.Remove(filepath.Join(z.homeDir, ".local/bin/celer")); err != nil {
		return fmt.Errorf("failed to remove celer binary: %w", err)
	}

	return nil
}

func (z zsh) registerRunCommand() error {
	zshrcPath := filepath.Join(z.homeDir, ".zshrc")
	if !fileio.PathExists(zshrcPath) {
		return fmt.Errorf("no .zshrc file found in home dir")
	}

	// Check if already contains the fpath.
	content, err := os.ReadFile(zshrcPath)
	if err != nil {
		return fmt.Errorf("failed to read ~/.zshrc: %w", err)
	}
	if strings.Contains(string(content), z.registerFpath) {
		return nil
	}

	// Add fpath to the bottom of `export PATH=$HOME/.local/bin:$PATH`
	file, err := os.Open(zshrcPath)
	if err != nil {
		return err
	}
	defer file.Close()

	var buffer bytes.Buffer
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := scanner.Text()
		if line == z.registerBinary {
			buffer.WriteString(line + "\n" + z.registerFpath + "\n\n")
		} else {
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

func (z zsh) unregisterRunCommand() error {
	// Check if .zshrc exists
	zshrcPath := filepath.Join(z.homeDir, ".zshrc")
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
		if line != z.registerFpath {
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

func NewZshCompletion(homeDir string, rootCmd *cobra.Command) zsh {
	return zsh{
		homeDir:        homeDir,
		registerFpath:  "fpath=($HOME/.local/share/zsh/site-functions $fpath) # added by celer",
		registerBinary: "export PATH=$HOME/.local/bin:$PATH # added by celer",
		rootCmd:        rootCmd,
	}
}
