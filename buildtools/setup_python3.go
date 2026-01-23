package buildtools

import (
	"celer/pkgs/cmd"
	"celer/pkgs/expr"
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
		library = strings.TrimPrefix(library, "python3:")

		// Check if the tool is already installed.
		installed, err := Python3.checkIfInstalled(library)
		if err != nil {
			return fmt.Errorf("check if %s is installed: %s", library, err)
		}
		if installed {
			continue
		}

		// Use Python3.Path instead of "python3" command to ensure it works on Windows.
		title := "[python3 install tool]"
		command := fmt.Sprintf("%s -m pip install %s", Python3.Path, library)
		executor := cmd.NewExecutor(title, command)
		if err := executor.Execute(); err != nil {
			return err
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
	command := fmt.Sprintf("pip show %s >%s 2>&1 && echo yes || echo no", target, nullDevice)
	executor := cmd.NewExecutor("", command)
	output, err := executor.ExecuteOutput()
	if err != nil {
		return false, err
	}

	if strings.TrimSpace(output) == "yes" {
		return true, nil
	}

	return false, nil
}
