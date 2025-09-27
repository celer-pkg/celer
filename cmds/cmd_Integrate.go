package cmds

import (
	"bufio"
	"bytes"
	"celer/configs"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"slices"
	"strings"

	"github.com/spf13/cobra"
)

type integrateCmd struct {
	powershell bool
	bash       bool
	remove     bool
}

func (i integrateCmd) Command(celer *configs.Celer) *cobra.Command {
	command := &cobra.Command{
		Use:   "integrate",
		Short: "Integrate tab completion.",
		Run: func(cobraCmd *cobra.Command, args []string) {
			homeDir, err := os.UserHomeDir()
			if err != nil {
				configs.PrintError(err, "cannot get home dir.")
				return
			}

			if i.remove {
				if err := i.doRemove(homeDir); err != nil {
					configs.PrintError(err, "tab completion remove failed.")
					return
				}
				configs.PrintSuccess("tab completion is removed.")
			} else {
				if err := i.installToSystem(homeDir); err != nil {
					configs.PrintError(err, "tab completion install failed.")
					return
				}
				configs.PrintSuccess("tab completion is integrated.")
			}
		},
		ValidArgsFunction: i.completion,
	}

	// Register flags.
	command.Flags().BoolVar(&i.remove, "remove", false, "remove tab completion.")
	command.Flags().BoolVar(&i.powershell, "powershell", false, "integrate tab completion for powershell.")
	command.Flags().BoolVar(&i.bash, "bash", false, "integrate tab completion for bash.")

	command.MarkFlagsMutuallyExclusive("powershell", "bash")

	return command
}

func (i integrateCmd) doRemove(homeDir string) error {
	switch {
	case i.powershell:
		if runtime.GOOS != "windows" {
			return fmt.Errorf("powershell completion is only supported on windows")
		}

		// Remove completion ps file.
		modulesDir := filepath.Join(os.Getenv("USERPROFILE"), "Documents", "WindowsPowerShell", "Modules")
		celerDir := filepath.Join(modulesDir, "celer")
		if err := os.RemoveAll(celerDir); err != nil {
			return fmt.Errorf("remove celer module error: %w", err)
		}

		// Remove celer.exe
		binDir := filepath.Join(os.Getenv("USERPROFILE"), "AppData", "Local", "celer")
		if err := os.RemoveAll(binDir); err != nil {
			return fmt.Errorf("remove celer.exe error: %w", err)
		}

		// Remove celer_completion.ps1 from profile.ps1.
		celerProfile := filepath.Join(modulesDir, "celer", "celer_completion.ps1")
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
				if !strings.HasPrefix(line, ". "+celerProfile) {
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

	case i.bash:
		if runtime.GOOS != "linux" {
			return fmt.Errorf("bash completion is only supported on linux")
		}

		// Remove celer binary.
		fmt.Println("[integrate] rm -f ~/.local/bin/celer")
		cmd := exec.Command("rm", "-f", filepath.Join(homeDir, ".local/bin/celer"))
		cmd.Stdout = os.Stdout
		cmd.Stderr = os.Stderr
		cmd.Stdin = os.Stdin
		cmd.Run()

		// Remove celer_completion.
		fmt.Println("[integrate] rm -f ~/.local/share/bash-completion/completions/celer")
		cmd = exec.Command("rm", "-f", filepath.Join(homeDir, ".local/share/bash-completion/completions/celer"))
		cmd.Stdout = os.Stdout
		cmd.Stderr = os.Stderr
		cmd.Stdin = os.Stdin
		cmd.Run()

	default:
		return fmt.Errorf("no --bash or --powershell specified to integrate")
	}

	return nil
}

func (i integrateCmd) installToSystem(homeDir string) error {
	if err := i.installExecutable(homeDir); err != nil {
		return err
	}

	if err := i.installCompletion(homeDir); err != nil {
		return err
	}

	return nil
}

func (i integrateCmd) installCompletion(homeDir string) error {
	filePath, err := i.generateCompletionFile()
	if err != nil {
		return err
	}

	// Install completion file.
	switch {
	case i.bash:
		// Install completion file to `~/.local/share/bash-completion/completions`
		destination := filepath.Join(homeDir, ".local", "share", "bash-completion", "completions", "celer")
		if err := os.MkdirAll(filepath.Dir(destination), os.ModePerm); err != nil {
			return err
		}
		if err := fileio.MoveFile(filePath, destination); err != nil {
			return err
		}
		fmt.Printf("[integrate] completion --> %s\n", destination)

	case i.powershell:
		// Install completion file to `~/Documents/WindowsPowerShell/Modules`
		modulesDir := filepath.Join(os.Getenv("USERPROFILE"), "Documents", "WindowsPowerShell", "Modules")
		celerProfile := filepath.Join(modulesDir, "celer", "celer_completion.ps1")
		profilePath := filepath.Join(filepath.Dir(modulesDir), "profile.ps1")
		if err := os.MkdirAll(filepath.Dir(celerProfile), os.ModePerm); err != nil {
			return fmt.Errorf("create PowerShell Modules dir error: %w", err)
		}

		if err := fileio.MoveFile(filePath, celerProfile); err != nil {
			return fmt.Errorf("move PowerShell completion file error: %w", err)
		}

		// Append completion file path to profile.
		if fileio.PathExists(profilePath) {
			// Add completion script to if not contains.
			profile, err := os.OpenFile(profilePath, os.O_CREATE|os.O_RDWR, os.ModePerm)
			if err != nil {
				return fmt.Errorf("open or create PowerShell profile error: %w", err)
			}
			defer profile.Close()

			// Read profile content.
			content, err := os.ReadFile(profilePath)
			if err != nil {
				return fmt.Errorf("read PowerShell profile error: %w", err)
			}

			lines := strings.Split(string(content), "\n")

			// Add completion script to profile if not contains.
			if !slices.Contains(lines, ". "+celerProfile) {
				profile.WriteString(". " + celerProfile + "\n")
			}
		} else {
			content := fmt.Sprintf(". %s", celerProfile)
			if err := os.WriteFile(profilePath, []byte(content), os.ModePerm); err != nil {
				return fmt.Errorf("write PowerShell profile error: %w", err)
			}
		}
	}

	return nil
}

func (i integrateCmd) generateCompletionFile() (string, error) {
	if err := dirs.CleanTmpFilesDir(); err != nil {
		return "", fmt.Errorf("create clean tmp dir error: %w", err)
	}

	var (
		filePath string
		genFunc  func(io.Writer) error
	)

	// Prepare completion file path and completion generation func.
	switch {
	case i.bash:
		if runtime.GOOS != "linux" {
			return "", fmt.Errorf("bash completion is only supported on linux")
		}

		filePath = filepath.Join(dirs.TmpFilesDir, "celer")
		genFunc = rootCmd.GenBashCompletion

	case i.powershell:
		if runtime.GOOS != "windows" {
			return "", fmt.Errorf("powershell completion is only supported on windows")
		}

		filePath = filepath.Join(dirs.TmpFilesDir, "celer_completion.ps1")
		genFunc = rootCmd.GenPowerShellCompletionWithDesc

	default:
		return "", fmt.Errorf("no --bash or --powershell specified to integrate")
	}

	// Generate completion file.
	file, err := os.Create(filePath)
	if err != nil {
		return "", fmt.Errorf("create completion file error: %w", err)
	}
	defer file.Close()

	if err := genFunc(file); err != nil {
		return "", fmt.Errorf("generate completion file error: %w", err)
	}

	return filePath, nil
}

func (i integrateCmd) installExecutable(homeDir string) error {
	path, err := os.Executable()
	if err != nil {
		return fmt.Errorf("get celer's path error: %w", err)
	}

	switch runtime.GOOS {
	case "linux":
		// Copy into `~/.local/bin`
		if err := i.executeCmd("cp", path, filepath.Join(homeDir, ".local/bin")); err != nil {
			return fmt.Errorf("cp celer to `/usr/local/bin` error: %w", err)
		}

		fmt.Println("[integrate] celer --> ~/.local/bin")

	case "windows":
		// Copy into `~/AppData/Local/celer`
		destionation := filepath.Join(os.Getenv("USERPROFILE"), "AppData", "Local", "celer", "celer.exe")
		if err := os.MkdirAll(filepath.Dir(destionation), os.ModePerm); err != nil {
			return fmt.Errorf("create celer.exe destination dir error: %w", err)
		}
		if err := fileio.CopyFile(path, destionation); err != nil {
			return fmt.Errorf("cp celer.exe to `%s` error: %w", destionation, err)
		}

		// Add celer.exe to PATH if it's not already there.
		pathEnv := os.Getenv("PATH")
		if !strings.Contains(pathEnv, filepath.Dir(destionation)) {
			if err := i.executeCmd("setx", "PATH", "%PATH%;"+filepath.Dir(destionation)); err != nil {
				return fmt.Errorf("add celer dir to PATH error: %w", err)
			}
		}

		fmt.Printf("[integrate] celer.exe --> %s\n", filepath.Dir(destionation))
	}

	return nil
}

func (i integrateCmd) executeCmd(name string, args ...string) error {
	cmd := exec.Command(name, args...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	cmd.Stdin = os.Stdin
	return cmd.Run()
}

func (i integrateCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string
	for _, flag := range []string{"--powershell", "--bash", "--remove"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
