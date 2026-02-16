package buildtools

import (
	"celer/envs"
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

func setupPython3(rootDir string) error {
	if Python3 != nil {
		return nil
	}

	// Init python3 for windows/linux.
	switch runtime.GOOS {
	case "linux":
		Python3 = &python3{
			Path:    filepath.Join(rootDir, "python3"),
			rootDir: rootDir,
		}
	case "windows":
		Python3 = &python3{
			Path:    filepath.Join(rootDir, "python.exe"),
			rootDir: rootDir,
		}
	default:
		return fmt.Errorf("unsupported os: %s", runtime.GOOS)
	}

	// Use PYTHONUSEBASE as python venv dir.
	pythonVenvDir := dirs.PythonUserBase

	// Ensure virtual environment exists.
	if !fileio.PathExists(pythonVenvDir) {
		command := fmt.Sprintf("%s -m venv %s", Python3.Path, pythonVenvDir)
		executor := cmd.NewExecutor("[create python venv]", command)
		if err := executor.Execute(); err != nil {
			return fmt.Errorf("failed to create python venv: %w", err)
		}
	}

	// Use virtual environment python with platform-specific paths.
	var venvPythonPath, venvPipPath, venvBinDir string
	if runtime.GOOS == "windows" {
		venvPythonPath = filepath.Join(pythonVenvDir, "Scripts", "python.exe")
		venvPipPath = filepath.Join(pythonVenvDir, "Scripts", "pip.exe")
		venvBinDir = filepath.Join(pythonVenvDir, "Scripts")
	} else {
		venvPythonPath = filepath.Join(pythonVenvDir, "bin", "python3")
		venvPipPath = filepath.Join(pythonVenvDir, "bin", "pip")
		venvBinDir = filepath.Join(pythonVenvDir, "bin")
	}

	if fileio.PathExists(venvPythonPath) && fileio.PathExists(venvPipPath) {
		Python3.Path = venvPythonPath
		Python3.rootDir = venvBinDir
	} else {
		var deleteCmd string
		if runtime.GOOS == "windows" {
			deleteCmd = fmt.Sprintf("rmdir /s /q %s", pythonVenvDir)
		} else {
			deleteCmd = fmt.Sprintf("rm -rf %s", pythonVenvDir)
		}
		return fmt.Errorf("python virtual environment is incomplete at %s\n "+
			"Please delete the directory and try again: %s\n",
			pythonVenvDir, deleteCmd)
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
		return fmt.Errorf("failed to setup python3: %w", err)
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
	}

	// Remove python3:xxx from list.
	*libraries = slices.DeleteFunc(*libraries, func(element string) bool {
		return strings.HasPrefix(element, "python3")
	})

	// Always ensure Python bin directory is in PATH.
	envs.AppendPythonBinDir(dirs.PythonUserBase)

	return nil
}

// isPythonPackageInstalled checks if a Python package is already installed.
// This avoids frequent PyPI requests that could lead to IP blocking.
func isPythonPackageInstalled(packageName string) bool {
	libDir := filepath.Join(dirs.PythonUserBase, expr.If(runtime.GOOS == "windows", "Lib", "lib"))
	if !fileio.PathExists(libDir) {
		return false
	}

	var packageDirPattern, distInfoPattern string
	switch runtime.GOOS {
	case "windows":
		// Windows: Lib/site-packages/{packageName} and Lib/site-packages/{packageName}-*.dist-info
		packageDirPattern = filepath.Join(libDir, "site-packages", packageName)
		distInfoPattern = filepath.Join(libDir, "site-packages", packageName+"-*.dist-info")

	case "linux":
		// Linux: lib/python*/site-packages/{packageName} and lib/python*/site-packages/{packageName}-*.dist-info
		packageDirPattern = filepath.Join(libDir, "python*", "site-packages", packageName)
		distInfoPattern = filepath.Join(libDir, "python*", "site-packages", packageName+"-*.dist-info")

	default:
		panic("unsupported os: " + runtime.GOOS)
	}

	matches, err := filepath.Glob(packageDirPattern)
	if err == nil && len(matches) > 0 {
		return true
	}

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
