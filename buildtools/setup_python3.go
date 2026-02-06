package buildtools

import (
	"celer/envs"
	"celer/pkgs/cmd"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"fmt"
	"path/filepath"
	"runtime"
	"slices"
	"strings"
)

var Python3 *python3

func setupPython3(rootDir string) error {
	if Python3 != nil {
		return nil
	}

	// Init python3 for windows/linux.
	switch runtime.GOOS {
	case "windows":
		Python3 = &python3{
			Path:    filepath.Join(rootDir, "python.exe"),
			rootDir: rootDir,
		}

	case "linux":
		// Use PYTHONUSEBASE as python venv dir.
		pythonVenvDir := dirs.PythonUserBase

		// Ensure virtual environment exists.
		if !fileio.PathExists(pythonVenvDir) {
			title := "[create python venv]"
			command := fmt.Sprintf("%s/python3 -m venv %s", rootDir, pythonVenvDir)
			executor := cmd.NewExecutor(title, command)
			if err := executor.Execute(); err != nil {
				return fmt.Errorf("failed to create python venv.\n %w", err)
			}
		}

		// Use virtual environment python
		venvPythonPath := filepath.Join(pythonVenvDir, "bin", "python3")
		venvPipPath := filepath.Join(pythonVenvDir, "bin", "pip")
		if fileio.PathExists(venvPythonPath) && fileio.PathExists(venvPipPath) {
			Python3 = &python3{
				Path:    venvPythonPath,
				rootDir: filepath.Join(pythonVenvDir, "bin"),
			}
		} else {
			return fmt.Errorf("python virtual environment is incomplete at %s\n"+
				"Please delete the directory and try again: rm -rf %s\n",
				pythonVenvDir, pythonVenvDir)
		}

	default:
		return fmt.Errorf("unsupported os: %s", runtime.GOOS)
	}

	return nil
}

// pip3Install checks if the python3 is installed.
func pip3Install(python3Tool *BuildTool, libraries *[]string) error {
	// Setup python3 if not set.
	var err error
	if python3Tool != nil {
		err = setupPython3(python3Tool.rootDir)
	} else if runtime.GOOS == "linux" {
		err = setupPython3("/usr/bin")
	} else {
		return fmt.Errorf("unsupported os: %s", runtime.GOOS)
	}
	if err != nil {
		return fmt.Errorf("failed to setup python3.\n %w", err)
	}

	// Install extra tools. Check if package is already installed in PYTHONUSERBASE to avoid frequent PyPI requests.
	// PYTHONUSERBASE is already set globally, so pip will install to workspace directory.
	for _, library := range *libraries {
		if !strings.HasPrefix(library, "python3:") {
			continue
		}

		// Use Python3.Path instead of "python3" command to ensure it works on Windows.
		libraryName := strings.TrimPrefix(library, "python3:")

		// Check if package is already installed in PYTHONUSERBASE to avoid frequent PyPI requests.
		if isPythonPackageInstalled(libraryName) {
			continue
		}

		title := fmt.Sprintf("[python3 install tool] %s", libraryName)
		command := fmt.Sprintf("%s -m pip install --ignore-installed %s", Python3.Path, libraryName)
		executor := cmd.NewExecutor(title, command)
		if err := executor.Execute(); err != nil {
			return fmt.Errorf("failed to install %s: %w", libraryName, err)
		}

		// Make sure the python3 executable can be found in PATH.
		envs.AppendPythonBinDir(filepath.Join(dirs.PythonUserBase, "bin"))
	}

	// Remove python3:xxx from list.
	*libraries = slices.DeleteFunc(*libraries, func(element string) bool {
		return strings.HasPrefix(element, "python3")
	})

	return nil
}

// isPythonPackageInstalled checks if a Python package is already installed.
// This avoids frequent PyPI requests that could lead to IP blocking.
func isPythonPackageInstalled(packageName string) bool {
	// Check in the virtual environment at workspace/.venv
	libDir := filepath.Join(dirs.PythonUserBase, "lib")
	if !fileio.PathExists(libDir) {
		return false
	}

	// Check for package directory: lib/python*/site-packages/{packageName}
	packageDirPattern := filepath.Join(libDir, "python*", "site-packages", packageName)
	matches, err := filepath.Glob(packageDirPattern)
	if err == nil && len(matches) > 0 {
		return true
	}

	// Check for .dist-info directory: lib/python*/site-packages/{packageName}-*.dist-info
	distInfoPattern := filepath.Join(libDir, "python*", "site-packages", packageName+"-*.dist-info")
	matches, err = filepath.Glob(distInfoPattern)
	if err == nil && len(matches) > 0 {
		return true
	}

	return false
}

type python3 struct {
	Path    string
	rootDir string
}
