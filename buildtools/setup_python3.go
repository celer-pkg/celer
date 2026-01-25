package buildtools

import (
	"celer/pkgs/cmd"
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

	// Install extra tools. pip will skip if already installed.
	for _, library := range *libraries {
		libraryName := strings.TrimPrefix(library, "python3:")

		// Use Python3.Path instead of "python3" command to ensure it works on Windows.
		title := fmt.Sprintf("[python3 install tool] %s", libraryName)
		command := fmt.Sprintf("%s -m pip install --user %s", Python3.Path, libraryName)
		executor := cmd.NewExecutor(title, command)
		if err := executor.Execute(); err != nil {
			return fmt.Errorf("failed to install %s: %w", libraryName, err)
		}
	}

	return nil
}

type python3 struct {
	Path    string
	rootDir string
}
