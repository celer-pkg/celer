package buildtools

import (
	"celer/pkgs/cmd"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"path/filepath"
	"runtime"
	"slices"
	"strings"
)

var Python3 *python3

func setupPython3(rootDir string) {
	if Python3 != nil {
		return
	}

	// Init python3 for windows/linux.
	switch runtime.GOOS {
	case "windows":
		Python3 = &python3{
			Path:    filepath.Join(rootDir, "python.exe"),
			rootDir: rootDir,
		}

	case "linux":
		Python3 = &python3{
			Path:    "/usr/bin/python3",
			rootDir: "/usr/bin",
		}

	default:
		panic("unsupported os: " + runtime.GOOS)
	}
}

// pipInstall checks if the python3 is installed.
func pipInstall(libraries *[]string) error {
	// Remove none python3:xxx from list.
	*libraries = slices.DeleteFunc(*libraries, func(element string) bool {
		return !strings.HasPrefix(element, "python3:")
	})

	defer func() {
		// Remove python3 related after setted up.
		*libraries = slices.DeleteFunc(*libraries, func(tool string) bool {
			return strings.HasPrefix(tool, "python3:")
		})
	}()

	// Install extra tools if not exist.
	for _, library := range *libraries {
		// Check if the tool is already installed.
		libraryName := strings.TrimPrefix(library, "python3:")
		installed, err := Python3.checkIfInstalled(libraryName)
		if err != nil {
			return fmt.Errorf("check if %s is installed: %s", libraryName, err)
		}
		if installed {
			continue
		}

		// Use Python3.Path instead of "python3" command to ensure it works on Windows.
		title := fmt.Sprintf("[python3 install tool] %s", libraryName)
		command := fmt.Sprintf("%s -m pip install --user %s", Python3.Path, libraryName)
		executor := cmd.NewExecutor(title, command)
		if err := executor.Execute(); err != nil {
			return fmt.Errorf("failed to install %s: %w", libraryName, err)
		}

		// Verify the installation.
		binPath := filepath.Join(dirs.PythonUserBase, "bin", libraryName)
		if fileio.PathExists(binPath) {
			checkCmd := fmt.Sprintf("%s --version >/dev/null 2>&1", binPath)
			checkExecutor := cmd.NewExecutor("", checkCmd)
			if checkExecutor.Execute() == nil {
				continue
			}
		}

		// Try to import as a module.
		verifyCmd := fmt.Sprintf("%s -c \"import %s\" 2>&1", Python3.Path, libraryName)
		verifyExecutor := cmd.NewExecutor("", verifyCmd)
		if verifyErr := verifyExecutor.Execute(); verifyErr != nil {
			return fmt.Errorf("failed to verify %s after installation (neither command-line tool nor importable module): %w", libraryName, verifyErr)
		}
	}

	return nil
}

type python3 struct {
	Path    string
	rootDir string
}

func (p python3) checkIfInstalled(target string) (bool, error) {
	// Redirect output to null device.
	nullDevice := expr.If(runtime.GOOS == "windows", "nul", "/dev/null")

	// PYTHONUSERBASE is already set in envs.CleanEnv(), so pip will check workspace directory.
	command := fmt.Sprintf("%s -m pip show %s >%s 2>&1 && echo yes || echo no", p.Path, target, nullDevice)
	executor := cmd.NewExecutor("", command)
	output, err := executor.ExecuteOutput()
	if err != nil {
		if strings.TrimSpace(output) == "" {
			return false, err
		}
	}

	if strings.TrimSpace(output) == "yes" {
		return true, nil
	}

	return false, nil
}
