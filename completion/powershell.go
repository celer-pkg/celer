package completion

import (
	"bufio"
	"bytes"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"slices"
	"strings"

	"github.com/spf13/cobra"
)

type powershell struct {
	homeDir        string
	registerBinary string
	rootCmd        *cobra.Command
}

func (p powershell) Register() error {
	if err := p.installBinary(); err != nil {
		return fmt.Errorf("failed to install powershell binary.\n %w", err)
	}
	if err := p.installCompletion(); err != nil {
		return fmt.Errorf("failed to install powershell completion.\n %w", err)
	}
	if err := p.registerRunCommand(); err != nil {
		return fmt.Errorf("failed to add run command to powershell profile.\n %w", err)
	}

	return nil
}

func (p powershell) Unregister() error {
	if err := p.uninstallBinary(); err != nil {
		return fmt.Errorf("failed to uninstall powershell binary.\n %w", err)
	}
	if err := p.uninstallCompletion(); err != nil {
		return fmt.Errorf("failed to uninstall powershell completion.\n %w", err)
	}
	if err := p.unregisterRunCommand(); err != nil {
		return fmt.Errorf("failed to remove run command from powershell profile.\n %w", err)
	}

	return nil
}

func (p powershell) installBinary() error {
	// Copy into `~/AppData/Local/celer`
	executable, err := os.Executable()
	if err != nil {
		return fmt.Errorf("failed to get celer's path.\n %w", err)
	}

	dest := filepath.Join(os.Getenv("USERPROFILE"), "AppData", "Local", "celer", "celer.exe")
	if err := os.MkdirAll(filepath.Dir(dest), os.ModePerm); err != nil {
		return fmt.Errorf("failed to create celer.exe destination dir.\n %w", err)
	}
	if err := fileio.CopyFile(executable, dest); err != nil {
		return fmt.Errorf("failed to copy celer.exe to `%s`.\n %w", dest, err)
	}
	fmt.Printf("[integrate] celer.exe -> %s\n", filepath.Dir(dest))

	// Add celer.exe to PATH if it's not already there.
	pathEnv := os.Getenv("PATH")
	if !strings.Contains(pathEnv, filepath.Dir(dest)) {
		if err := p.executeCmd("setx", "PATH", "%PATH%;"+filepath.Dir(dest)); err != nil {
			return fmt.Errorf("failed to add celer dir to PATH.\n %w", err)
		}
	}

	return nil
}

func (p powershell) installCompletion() error {
	if err := dirs.CleanTmpFilesDir(); err != nil {
		return fmt.Errorf("failed to create clean tmp dir.\n %w", err)
	}

	// Use temporary file mode to ensure file operation safety.
	tmpDir := dirs.TmpFilesDir
	tmpFile := filepath.Join(tmpDir, "celer_completion.ps1")

	// Create and write temporary completion file.
	if err := func() error {
		file, err := os.Create(tmpFile)
		if err != nil {
			return fmt.Errorf("failed to create powershell completion file.\n %w", err)
		}
		defer file.Close()

		return p.rootCmd.GenPowerShellCompletion(file)
	}(); err != nil {
		return fmt.Errorf("failed to generate powershell completion file.\n %w", err)
	}

	// Wait for the file to be completely released (Windows system may need this).
	// Add a small delay or use file lock check here.
	if err := p.ensureFileReleased(tmpFile); err != nil {
		return err
	}

	// Install completion file to `~/Documents/WindowsPowerShell/Modules`
	modulesDir := filepath.Join(os.Getenv("USERPROFILE"), "Documents", "WindowsPowerShell", "Modules")
	celerRcFile := filepath.Join(modulesDir, "celer", "celer_completion.ps1")
	if err := os.MkdirAll(filepath.Dir(celerRcFile), os.ModePerm); err != nil {
		return fmt.Errorf("failed to create PowerShell Modules dir.\n %w", err)
	}

	if err := fileio.MoveFile(tmpFile, celerRcFile); err != nil {
		return fmt.Errorf("failed to move PowerShell completion file.\n %w", err)
	}

	return nil
}

// Ensure the file is released helper method.
func (p powershell) ensureFileReleased(filePath string) error {
	// Try to open the file multiple times to ensure it's released.
	for range 3 {
		file, err := os.OpenFile(filePath, os.O_RDONLY, 0644)
		if err != nil {
			// If the file cannot be opened, it may not exist or there may be other errors.
			if os.IsNotExist(err) {
				return fmt.Errorf("file does not exist: %s", filePath)
			}
			continue // Other errors, keep retrying
		}
		file.Close()
		return nil // If the file can be opened and closed, it means it's released.
	}
	return fmt.Errorf("file is still locked after multiple attempts: %s", filePath)
}

func (p powershell) uninstallCompletion() error {
	// Unregister completion ps file.
	modulesDir := filepath.Join(os.Getenv("USERPROFILE"), "Documents", "WindowsPowerShell", "Modules")
	celerDir := filepath.Join(modulesDir, "celer")
	if err := os.RemoveAll(celerDir); err != nil {
		return fmt.Errorf("failed to unregister celer module.\n %w", err)
	}

	fmt.Println("[integrate] rm -rf ~/Documents/WindowsPowerShell/Modules")
	return nil
}

func (p powershell) uninstallBinary() error {
	// Unregister completion ps file.
	modulesDir := filepath.Join(os.Getenv("USERPROFILE"), "Documents", "WindowsPowerShell", "Modules")
	celerDir := filepath.Join(modulesDir, "celer")
	if err := os.RemoveAll(celerDir); err != nil {
		return fmt.Errorf("failed to unregister celer module.\n %w", err)
	}
	fmt.Printf("[integrate] rm -rf %s\n", celerDir)

	// Remove celer.exe
	binDir := filepath.Join(os.Getenv("USERPROFILE"), "AppData", "Local", "celer")
	if err := os.RemoveAll(binDir); err != nil {
		return fmt.Errorf("failed to remove celer.exe.\n %w", err)
	}
	fmt.Printf("[integrate] rm -rf %s\n", binDir)

	// Remove empty parent folder.
	if err := fileio.RemoveFolderRecursively(modulesDir); err != nil {
		return err
	}

	return nil
}

func (p powershell) registerRunCommand() error {
	// Install completion file to `~/Documents/WindowsPowerShell/Modules`
	modulesDir := filepath.Join(os.Getenv("USERPROFILE"), "Documents", "WindowsPowerShell", "Modules")
	profilePath := filepath.Join(filepath.Dir(modulesDir), "profile.ps1")

	// Append completion file path to profile.
	if fileio.PathExists(profilePath) {
		// Add completion script to if not contains.
		profile, err := os.OpenFile(profilePath, os.O_CREATE|os.O_RDWR, os.ModePerm)
		if err != nil {
			return fmt.Errorf("failed to open or create PowerShell profile.\n %w", err)
		}
		defer profile.Close()

		// Read profile content.
		content, err := os.ReadFile(profilePath)
		if err != nil {
			return fmt.Errorf("failed to read PowerShell profile.\n %w", err)
		}

		lines := strings.Split(string(content), "\n")

		// Add completion script to profile if not contains.
		if !slices.Contains(lines, p.registerBinary) {
			profile.WriteString(p.registerBinary + "\n")
		}
	} else {
		if err := os.WriteFile(profilePath, []byte(p.registerBinary), os.ModePerm); err != nil {
			return fmt.Errorf("failed to write PowerShell profile.\n %w", err)
		}
	}

	return nil
}

func (p powershell) unregisterRunCommand() error {
	// Remove celer_completion.ps1 from profile.ps1.
	modulesDir := filepath.Join(os.Getenv("USERPROFILE"), "Documents", "WindowsPowerShell", "Modules")
	profilePath := filepath.Join(filepath.Dir(modulesDir), "profile.ps1")
	if fileio.PathExists(profilePath) {
		file, err := os.Open(profilePath)
		if err != nil {
			return err
		}

		var buffer bytes.Buffer
		scanner := bufio.NewScanner(file)
		for scanner.Scan() {
			line := scanner.Text()
			if !strings.HasPrefix(line, p.registerBinary) {
				buffer.WriteString(line + "\n")
			}
		}
		if err := scanner.Err(); err != nil {
			file.Close()
			return err
		}
		file.Close()

		if buffer.Len() == 0 {
			if err := os.Remove(profilePath); err != nil {
				return err
			}
		} else {
			if err := os.WriteFile(profilePath, buffer.Bytes(), os.ModePerm); err != nil {
				return err
			}
		}
	}
	return nil
}

func (p powershell) executeCmd(name string, args ...string) error {
	cmd := exec.Command(name, args...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	cmd.Stdin = os.Stdin
	return cmd.Run()
}

func NewPowerShellCompletion(homeDir string, rootCmd *cobra.Command) powershell {
	return powershell{
		homeDir:        homeDir,
		rootCmd:        rootCmd,
		registerBinary: ". $HOME\\Documents\\WindowsPowerShell\\Modules\\celer\\celer_completion.ps1 # added by celer",
	}
}
