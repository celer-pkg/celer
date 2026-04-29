package buildtools

import (
	"fmt"
	"path/filepath"
	"runtime"
	"slices"
	"strings"

	"github.com/celer-pkg/celer/buildtools/python"
	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/envs"
	"github.com/celer-pkg/celer/pkgs/cmd"
	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/expr"
	"github.com/celer-pkg/celer/pkgs/fileio"

	"github.com/BurntSushi/toml"
)

var PythonTool *pythonTool

func pip3Install(ctx context.Context, libraries *[]string) error {
	var (
		pythonVersion = ctx.Project().GetPythonVersion()
		venvDir       = getVersionedVenvPath(pythonVersion)
	)

	// Setup python3 using conda.
	if err := setupPython(ctx, pythonVersion); err != nil {
		return fmt.Errorf("failed to setup python3 -> %w", err)
	}

	// Install extra tools. Check if package is already installed in PYTHONUSERBASE to avoid frequent PyPI requests.
	// PYTHONUSERBASE is already set globally, so pip will install to workspace directory.
	for _, library := range *libraries {
		if !strings.HasPrefix(library, "python3:") {
			continue
		}

		// Format python3 library name version.
		nameVersion := strings.TrimPrefix(library, "python3:")
		nameVersion = strings.ReplaceAll(nameVersion, "@", "==")

		// Check if package is already installed in PYTHONUSERBASE to avoid frequent PyPI requests.
		if isPackageInstalled(nameVersion, venvDir) {
			continue
		}

		// Install python3 library with path path.
		title := fmt.Sprintf("[python3 install tool %s]", nameVersion)
		command := fmt.Sprintf("%s -m pip install --ignore-installed %s", PythonTool.Path, nameVersion)
		executor := cmd.NewExecutor(title, command)
		if err := executor.Execute(); err != nil {
			return fmt.Errorf("failed to install %s -> %w", nameVersion, err)
		}
	}

	// Remove python3:xxx from list.
	*libraries = slices.DeleteFunc(*libraries, func(element string) bool {
		return strings.HasPrefix(element, "python3")
	})

	// Always ensure Python bin directory is in PATH.
	envs.AppendPythonBinDir(venvDir)

	return nil
}

// setupPython sets up Python with a specific version.
// Strategy: Try system Python first, fallback to conda if version mismatches.
func setupPython(ctx context.Context, pythonVersion string) error {
	// Quick return only if version hasn't changed AND venv still exists on disk.
	envDir := getVersionedVenvPath(pythonVersion)
	if PythonTool != nil && PythonTool.version == pythonVersion && fileio.PathExists(envDir) {
		return nil
	}

	// Try to use system Python first if versions match.
	useSystemPython := false
	systemPythonVer := GetDefaultPythonVersion()
	versionMatches := normalizeVersion(systemPythonVer) == normalizeVersion(pythonVersion)
	if systemPythonVer != "" && versionMatches {
		useSystemPython = true
		color.Printf(color.Hint, "- detected system python %s (matches required %s)",
			normalizeVersion(systemPythonVer), normalizeVersion(pythonVersion))
	}

	var currentPython python.Python
	if useSystemPython { // Use system Python if version matches.
		currentPython = &python.SystemPython{}
	} else { // Fallback to conda if system Python doesn't match or doesn't exist.
		archiveName, err := getCondaArchiveFromConfig()
		if err != nil {
			return fmt.Errorf("failed to get conda archive from config -> %w", err)
		}
		currentPython = python.NewCondaPython(ctx, archiveName, pythonVersion)
	}

	if err := currentPython.Setup(); err != nil {
		return fmt.Errorf("failed to setup conda for Python %s -> %w", pythonVersion, err)
	}

	// Create version specific python environment if not exist.
	pythonExec, err := currentPython.GetExecutable()
	if err != nil {
		return fmt.Errorf("failed to detect python executable for version %s -> %w", pythonVersion, err)
	}

	// Ensure virtual environment exists.
	if !fileio.PathExists(envDir) {
		command := fmt.Sprintf("%s -m venv %s", pythonExec, envDir)
		executor := cmd.NewExecutor("[create python venv]", command)
		if err := executor.Execute(); err != nil {
			return fmt.Errorf("failed to create python venv -> %w", err)
		}
	}

	// Use virtual environment python with platform-specific paths.
	var venvPythonPath, venvPipPath, venvBinDir string
	if runtime.GOOS == "windows" {
		venvPythonPath = filepath.Join(envDir, "Scripts", "python.exe")
		venvPipPath = filepath.Join(envDir, "Scripts", "pip.exe")
		venvBinDir = filepath.Join(envDir, "Scripts")
	} else {
		venvPythonPath = filepath.Join(envDir, "bin", "python3")
		venvPipPath = filepath.Join(envDir, "bin", "pip")
		venvBinDir = filepath.Join(envDir, "bin")
	}

	// Make sure the virtual environment was created successfully and contains the expected executables.
	if !fileio.PathExists(venvPythonPath) || !fileio.PathExists(venvPipPath) {
		var deleteCmd string
		if runtime.GOOS == "windows" {
			deleteCmd = fmt.Sprintf("rmdir /s /q %s", envDir)
		} else {
			deleteCmd = fmt.Sprintf("rm -rf %s", envDir)
		}
		return fmt.Errorf("python virtual environment is incomplete at %s\n "+
			"Please delete the directory and try again: %s\n", envDir, deleteCmd)
	}

	// Save python info as global variable.
	PythonTool = &pythonTool{
		Path:    venvPythonPath,
		rootDir: venvBinDir,
		version: pythonVersion,
	}
	return nil
}

func getCondaArchiveFromConfig() (string, error) {
	// Determine current architecture
	arch := runtime.GOARCH
	switch arch {
	case "amd64", "x86_64":
		arch = "x86_64"
	case "arm64":
		arch = "aarch64"
	}

	// Read the static TOML file for the current platform
	staticFile := fmt.Sprintf("static/%s-%s.toml", arch, runtime.GOOS)
	bytes, err := static.ReadFile(staticFile)
	if err != nil {
		return "", fmt.Errorf("failed to read TOML config %s -> %w", staticFile, err)
	}

	var buildTools BuildTools
	if err := toml.Unmarshal(bytes, &buildTools); err != nil {
		return "", fmt.Errorf("failed to parse TOML config %s -> %w", staticFile, err)
	}

	// Find the conda tool entry.
	condaTool := buildTools.findTool(nil, "conda")
	if condaTool == nil {
		return "", fmt.Errorf("conda tool not found in %s", staticFile)
	}

	// Use the Archive field if specified, otherwise use the filename from URL.
	if condaTool.Archive != "" {
		return condaTool.Archive, nil
	}

	// Fallback: extract filename from URL.
	return filepath.Base(condaTool.Url), nil
}

func getVersionedVenvPath(pythonVersion string) string {
	// Normalize version to minor version format for directory name (e.g., 3.10.5 -> 3.10)
	minorVersion := pythonVersion
	if strings.Count(pythonVersion, ".") > 1 {
		parts := strings.Split(pythonVersion, ".")
		minorVersion = parts[0] + "." + parts[1]
	}
	return filepath.Join(dirs.WorkspaceDir, "installed", fmt.Sprintf("venv-%s", minorVersion))
}

// GetDefaultPythonVersion returns the default Python version for the current platform.
// - Windows: reads from buildtools/static TOML python tool definition (via GetDefaultPythonVersion from build_tools.go)
// - Linux/macOS: returns detected system python3 version.
func GetDefaultPythonVersion() string {
	// For Windows, delegate to build_tools implementation
	if runtime.GOOS == "windows" {
		return getWindowsDefaultPythonVersion()
	} else { // For Linux/macOS, detect system python3 version
		return python.GetSystemPythonVersion()
	}
}

// isPackageInstalled checks if a Python package is already installed.
// This avoids frequent PyPI requests that could lead to IP blocking.
func isPackageInstalled(packageName string, venvDir string) bool {
	libDir := filepath.Join(venvDir, expr.If(runtime.GOOS == "windows", "Lib", "lib"))
	if !fileio.PathExists(libDir) {
		return false
	}

	var packageDirPattern, distInfoPattern string
	switch runtime.GOOS {
	case "windows":
		// Windows: Lib/site-packages/{packageName} and Lib/site-packages/{packageName}-*.dist-info.
		packageDirPattern = filepath.Join(libDir, "site-packages", packageName)
		distInfoPattern = filepath.Join(libDir, "site-packages", packageName+"-*.dist-info")

	case "linux", "darwin":
		// Linux/Darwin: lib/python*/site-packages/{packageName} and lib/python*/site-packages/{packageName}-*.dist-info.
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

// getWindowsDefaultPythonVersion reads Python version from static TOML.
func getWindowsDefaultPythonVersion() string {
	// Determine current architecture.
	arch := runtime.GOARCH
	switch arch {
	case "amd64", "x86_64":
		arch = "x86_64"
	case "arm64":
		arch = "aarch64"
	}

	staticFile := fmt.Sprintf("static/%s-%s.toml", arch, runtime.GOOS)
	bytes, err := static.ReadFile(staticFile)
	if err != nil {
		panic(fmt.Sprintf("failed to read %s", staticFile))
	}

	var buildTools BuildTools
	if err := toml.Unmarshal(bytes, &buildTools); err != nil {
		panic(fmt.Sprintf("failed to decode %s: %s", staticFile, err))
	}

	pythonTool := buildTools.findTool(nil, "python3")
	if pythonTool != nil && pythonTool.Version != "" {
		return pythonTool.Version
	}

	panic("failed to read windows default python version")
}

func shouldUseConda(ctx context.Context) bool {
	projectVersion := ctx.Project().GetPythonVersion()
	systemDefault := GetDefaultPythonVersion()
	return normalizeVersion(projectVersion) != normalizeVersion(systemDefault)
}

func normalizeVersion(fullVersion string) string {
	parts := strings.Split(fullVersion, ".")
	if len(parts) >= 2 {
		return parts[0] + "." + parts[1]
	}
	return fullVersion
}

type pythonTool struct {
	Path    string
	rootDir string
	version string
}
