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

	// Add celer.exe to PATH if it's not already there.
	pathEnv := os.Getenv("PATH")
	if !strings.Contains(pathEnv, filepath.Dir(dest)) {
		if err := p.executeCmd("setx", "PATH", "%PATH%;"+filepath.Dir(dest)); err != nil {
			return fmt.Errorf("failed to add celer dir to PATH.\n %w", err)
		}
	}

	fmt.Printf("[integrate] celer.exe --> %s\n", filepath.Dir(dest))
	return nil
}

func (p powershell) installCompletion() error {
	if err := dirs.CleanTmpFilesDir(); err != nil {
		return fmt.Errorf("failed to create clean tmp dir.\n %w", err)
	}

	// Generate completion file.
	filePath := filepath.Join(dirs.TmpFilesDir, "celer_completion.ps1")
	file, err := os.Create(filePath)
	if err != nil {
		return fmt.Errorf("failed to create powershell completion file.\n %w", err)
	}
	defer file.Close()

	if err := p.rootCmd.GenPowerShellCompletion(file); err != nil {
		return fmt.Errorf("failed to generate powershell completion file.\n %w", err)
	}

	return nil
}

func (p powershell) uninstallCompletion() error {
	// Unregister completion ps file.
	modulesDir := filepath.Join(os.Getenv("USERPROFILE"), "Documents", "WindowsPowerShell", "Modules")
	celerDir := filepath.Join(modulesDir, "celer")
	if err := os.RemoveAll(celerDir); err != nil {
		return fmt.Errorf("failed to unregister celer module.\n %w", err)
	}

	fmt.Println("[integrate] celer --> ~/Documents/WindowsPowerShell/Modules")
	return nil
}

func (p powershell) uninstallBinary() error {
	// Unregister completion ps file.
	modulesDir := filepath.Join(os.Getenv("USERPROFILE"), "Documents", "WindowsPowerShell", "Modules")
	celerDir := filepath.Join(modulesDir, "celer")
	if err := os.RemoveAll(celerDir); err != nil {
		return fmt.Errorf("failed to unregister celer module.\n %w", err)
	}

	// Remove celer.exe
	binDir := filepath.Join(os.Getenv("USERPROFILE"), "AppData", "Local", "celer")
	if err := os.RemoveAll(binDir); err != nil {
		return fmt.Errorf("failed to remove celer.exe.\n %w", err)
	}

	return nil
}

func (p powershell) registerRunCommand() error {
	// Install completion file to `~/Documents/WindowsPowerShell/Modules`
	modulesDir := filepath.Join(os.Getenv("USERPROFILE"), "Documents", "WindowsPowerShell", "Modules")
	celerRcFile := filepath.Join(modulesDir, "celer", "celer_completion.ps1")
	profilePath := filepath.Join(filepath.Dir(modulesDir), "profile.ps1")
	if err := os.MkdirAll(filepath.Dir(celerRcFile), os.ModePerm); err != nil {
		return fmt.Errorf("failed to create PowerShell Modules dir.\n %w", err)
	}

	rcFile := filepath.Join(dirs.TmpFilesDir, "celer_completion.ps1")
	if err := fileio.MoveFile(rcFile, celerRcFile); err != nil {
		return fmt.Errorf("failed to move PowerShell completion file.\n %w", err)
	}

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
		content := fmt.Sprintf(". %s", celerRcFile)
		if err := os.WriteFile(profilePath, []byte(content), os.ModePerm); err != nil {
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
