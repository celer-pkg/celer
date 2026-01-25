package buildtools

import (
	"celer/envs"
	"celer/pkgs/cmd"
	"celer/pkgs/dirs"
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

// pip3Install checks if the python3 is installed.
func pip3Install(python3Tool *BuildTool, libraries *[]string) error {
	// Setup python3 if not set.
	if python3Tool != nil {
		setupPython3(python3Tool.rootDir)
	} else if runtime.GOOS == "linux" {
		setupPython3("/usr/bin")
	}

	// Install extra tools. pip will skip if already installed.
	for _, library := range *libraries {
		if !strings.HasPrefix(library, "python3:") {
			continue
		}

		// Use Python3.Path instead of "python3" command to ensure it works on Windows.
		libraryName := strings.TrimPrefix(library, "python3:")
		title := fmt.Sprintf("[python3 install tool] %s", libraryName)
		command := fmt.Sprintf("%s -m pip install --user %s", Python3.Path, libraryName)
		executor := cmd.NewExecutor(title, command)
		if err := executor.Execute(); err != nil {
			return fmt.Errorf("failed to install %s: %w", libraryName, err)
		}

		// Make sure the python3 executable can be found in PATH.
		envs.AppendPythonBinDir(dirs.PythonUserBase)
	}

	// Remove python3:xxx from list.
	*libraries = slices.DeleteFunc(*libraries, func(element string) bool {
		return strings.HasPrefix(element, "python3")
	})

	return nil
}

type python3 struct {
	Path    string
	rootDir string
}
