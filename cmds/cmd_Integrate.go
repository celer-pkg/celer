package cmds

import (
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
	uninstall bool
}

func (i integrateCmd) Command() *cobra.Command {
	command := &cobra.Command{
		Use:   "integrate",
		Short: "Integrate to support tab completion.",
		Run: func(cobraCmd *cobra.Command, args []string) {
			homeDir, err := os.UserHomeDir()
			if err != nil {
				configs.PrintError(err, "cannot get home dir.")
				return
			}

			if i.uninstall {
				if err := i.doUninstall(homeDir); err != nil {
					configs.PrintError(err, "celer integration uninstall failed.")
					return
				}
				configs.PrintSuccess("celer integration is uninstalled.")
			} else {
				if err := i.installToSystem(homeDir); err != nil {
					configs.PrintError(err, "celer integrate failed.")
					return
				}
				configs.PrintSuccess("celer is integrated.")
			}
		},
		ValidArgsFunction: i.completion,
	}

	// Register flags.
	command.Flags().BoolVarP(&i.uninstall, "uninstall", "u", false, "uninstall integrated celer.")

	return command
}

func (i integrateCmd) doUninstall(homeDir string) error {
	switch runtime.GOOS {
	case "windows":
		// Remove completion ps file.
		modulesDir := filepath.Join(os.Getenv("USERPROFILE"), "Documents", "WindowsPowerShell", "Modules")
		celerDir := filepath.Join(modulesDir, "celer")
		if err := os.RemoveAll(celerDir); err != nil {
			return fmt.Errorf("cannot remove celer module: %w", err)
		}

		// Remove celer.exe
		binDir := filepath.Join(os.Getenv("USERPROFILE"), "AppData", "Local", "celer")
		if err := os.RemoveAll(binDir); err != nil {
			return fmt.Errorf("cannot remove celer.exe: %w", err)
		}

	case "linux":
		// Uninstall celer binary.
		fmt.Println("[integrate] rm -f ~/.local/bin/celer")
		cmd := exec.Command("rm", "-f", filepath.Join(homeDir, ".local/bin/celer"))
		cmd.Stdout = os.Stdout
		cmd.Stderr = os.Stderr
		cmd.Stdin = os.Stdin
		cmd.Run()

		// Uninstall celer_completion.
		fmt.Println("[integrate] rm -f ~/.local/share/bash-completion/completions/celer")
		cmd = exec.Command("rm", "-f", filepath.Join(homeDir, ".local/share/bash-completion/completions/celer"))
		cmd.Stdout = os.Stdout
		cmd.Stderr = os.Stderr
		cmd.Stdin = os.Stdin
		cmd.Run()
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
	terminal, err := i.terminal()
	if err != nil {
		return fmt.Errorf("cannot guess terminal: %w", err)
	}

	if err := dirs.CleanTmpFilesDir(); err != nil {
		return fmt.Errorf("cannot create clean tmp dir: %w", err)
	}

	var (
		filePath string
		genFunc  func(io.Writer) error
	)

	switch terminal {
	case "bash":
		filePath = filepath.Join(dirs.TmpFilesDir, "celer")
		genFunc = rootCmd.GenBashCompletion

	case "zsh":
		filePath = filepath.Join(dirs.TmpFilesDir, "_celer_completion")
		genFunc = rootCmd.GenZshCompletion

	case "fish":
		filePath = filepath.Join(dirs.TmpFilesDir, "celer_completion")
		genFunc = func(w io.Writer) error {
			return rootCmd.GenFishCompletion(w, true)
		}

	case "powershell":
		filePath = filepath.Join(dirs.TmpFilesDir, "celer_completion.ps1")
		genFunc = rootCmd.GenPowerShellCompletionWithDesc

	default:
		return fmt.Errorf("unsupported terminal: %s", terminal)
	}

	file, err := os.Create(filePath)
	if err != nil {
		return fmt.Errorf("cannot create completion file: %w", err)
	}
	defer file.Close()

	if err := genFunc(file); err != nil {
		return fmt.Errorf("cannot generate completion file: %w", err)
	}

	// Install completion file.
	switch terminal {
	case "bash":
		destination := filepath.Join(homeDir, ".local", "share", "bash-completion", "completions", "celer")
		if err := i.executeCmd("mkdir", "-p", filepath.Dir(destination)); err != nil {
			return err
		}
		if err := i.executeCmd("mv", filePath, destination); err != nil {
			return err
		}
		fmt.Printf("[integrate] completion --> %s\n", destination)

	case "zsh":
		// cp completion file to  ~/.zsh/completions/
		destination := filepath.Join(os.Getenv("USERPROFILE"), "completions", "_celer_completion")
		if err := os.MkdirAll(filepath.Dir(destination), os.ModePerm); err != nil {
			return fmt.Errorf("cannot create zsh folder: %w", err)
		}
		if err := os.Rename(filePath, destination); err != nil {
			return err
		}

		// Fix ~/.zshrc to support completion.
		content := []string{"fpath=(~/.zsh/completions $fpath)", "autoload -Uz compinit && compinit"}
		zshrcPath := filepath.Join(os.Getenv("USERPROFILE"), ".zshrc")
		if fileio.PathExists(zshrcPath) {
			if err := os.WriteFile(zshrcPath, []byte(strings.Join(content, "\n")), os.ModePerm); err != nil {
				return fmt.Errorf("cannot create ~/.zshrc")
			}
		} else {
			bytes, err := os.ReadFile(zshrcPath)
			if err != nil {
				return fmt.Errorf("cannnot read ~/.zshrc")
			}

			lines := strings.Split(string(bytes), "\n")
			if !slices.Contains(lines, content[0]) {
				zshrc, err := os.OpenFile(filePath, os.O_CREATE|os.O_RDWR|os.O_APPEND, os.ModePerm)
				if err != nil {
					return fmt.Errorf("cannot open ~/.zshrc")
				}
				defer zshrc.Close()

				if _, err := zshrc.WriteAt([]byte(strings.Join(content, "\n")), 0); err != nil {
					return fmt.Errorf("cannot write ~/.zshrc: %w", err)
				}
			}
		}

	case "powershell":
		modulesDir := filepath.Join(os.Getenv("USERPROFILE"), "Documents", "WindowsPowerShell", "Modules")
		celerProfile := filepath.Join(modulesDir, "celer", "celer_completion.ps1")
		profilePath := filepath.Join(filepath.Dir(modulesDir), "profile.ps1")
		if err := os.MkdirAll(filepath.Dir(celerProfile), os.ModePerm); err != nil {
			return fmt.Errorf("cannot create PowerShell Modules dir: %w", err)
		}

		if err := fileio.CopyFile(filePath, celerProfile); err != nil {
			return fmt.Errorf("cannot copy PowerShell completion file: %w", err)
		}

		if fileio.PathExists(profilePath) {
			// Add completion script to if not contains.
			profile, err := os.OpenFile(profilePath, os.O_CREATE|os.O_RDWR, os.ModePerm)
			if err != nil {
				return fmt.Errorf("cannot open or create PowerShell profile: %w", err)
			}
			defer profile.Close()

			// Read profile content.
			content, err := os.ReadFile(profilePath)
			if err != nil {
				return fmt.Errorf("cannot read PowerShell profile: %w", err)
			}

			lines := strings.Split(string(content), "\n")

			// Add completion script to profile if not contains.
			if !slices.Contains(lines, ". "+celerProfile) {
				profile.WriteString(". " + celerProfile + "\n")
			}
		} else {
			content := fmt.Sprintf(". %s", celerProfile)
			if err := os.WriteFile(profilePath, []byte(content), os.ModePerm); err != nil {
				return fmt.Errorf("cannot write PowerShell profile: %w", err)
			}
		}
	}

	return nil
}

func (i integrateCmd) installExecutable(homeDir string) error {
	path, err := os.Executable()
	if err != nil {
		return fmt.Errorf("cannot get celer's path: %w", err)
	}

	switch runtime.GOOS {
	case "linux":
		// Install celer into `/usr/local/bin`
		if err := i.executeCmd("cp", path, filepath.Join(homeDir, ".local/bin")); err != nil {
			return fmt.Errorf("failed to cp celer to `/usr/local/bin`: %w", err)
		}

		fmt.Println("[integrate] celer --> /usr/local/bin")

	case "windows":
		// Install celer into `C:/Users/[user]/AppData/Local/celer/celer.exe`
		destionation := filepath.Join(os.Getenv("USERPROFILE"), "AppData", "Local", "celer", "celer.exe")
		if err := os.MkdirAll(filepath.Dir(destionation), os.ModePerm); err != nil {
			return fmt.Errorf("cannot create celer.exe destination dir: %w", err)
		}
		if err := fileio.CopyFile(path, destionation); err != nil {
			return fmt.Errorf("failed to cp celer.exe to `%s`: %w", destionation, err)
		}

		// Add celer.exe to PATH if it's not already there.
		pathEnv := os.Getenv("PATH")
		if !strings.Contains(pathEnv, filepath.Dir(destionation)) {
			if err := i.executeCmd("setx", "PATH", "%PATH%;"+filepath.Dir(destionation)); err != nil {
				return fmt.Errorf("failed to add celer.exe to PATH: %w", err)
			}
		}

		fmt.Printf("[integrate] celer.exe --> %s\n", destionation)
	}

	return nil
}

func (i integrateCmd) terminal() (string, error) {
	if runtime.GOOS == "windows" {
		return "powershell", nil
	}

	envValue := os.Getenv("SHELL")
	if envValue == "" {
		return "", fmt.Errorf("cannot guess current terminal")
	}

	switch {
	case strings.HasSuffix(envValue, "bash"):
		return "bash", nil

	case strings.HasSuffix(envValue, "zsh"):
		return "zsh", nil

	case strings.HasSuffix(envValue, "fish"):
		return "fish", nil
	}

	return "", fmt.Errorf("unsupported terminal: %s", envValue)
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
	for _, flag := range []string{"--uninstall", "-u"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
