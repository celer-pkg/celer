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
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/expr"
	"github.com/celer-pkg/celer/pkgs/fileio"

	"github.com/BurntSushi/toml"
)

var PythonTool *pythonTool

func pip3Install(ctx context.Context, pipConfig context.PythonConfig, libraries *[]string) error {
	// Get python version from project config if available, otherwise use default version.
	pythonVersion := GetDefaultPythonVersion()
	pythonConfig := ctx.PythonConfig()
	if pythonConfig != nil && pythonConfig.GetVersion() != "" {
		pythonVersion = pythonConfig.GetVersion()
	}
	venvDir := getPythonVenvPath(pythonVersion, ctx.Project().GetName())

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

		// Build pip install command with PyPI source configuration.
		var builder strings.Builder

		// If Python needs LD_LIBRARY_PATH, prepend it to the command.
		if PythonTool.ldLibraryPath != "" {
			fmt.Fprintf(&builder, "LD_LIBRARY_PATH=%s ", PythonTool.ldLibraryPath)
		}

		builder.WriteString(PythonTool.Path)
		builder.WriteString(" -m pip install --ignore-installed")

		// Add PyPI source configuration if available.
		if pipConfig != nil {
			if indexUrl := pipConfig.GetIndexUrl(); indexUrl != "" {
				builder.WriteString(" -i ")
				builder.WriteString(indexUrl)
			}
			for _, extraUrl := range pipConfig.GetExtraIndexUrls() {
				builder.WriteString(" --extra-index-url ")
				builder.WriteString(extraUrl)
			}
			for _, host := range pipConfig.GetTrustedHosts() {
				builder.WriteString(" --trusted-host ")
				builder.WriteString(host)
			}
		}

		builder.WriteString(" ")
		builder.WriteString(nameVersion)

		// Install python3 library with path path.
		title := fmt.Sprintf("[python3 install tool %s]", nameVersion)
		command := builder.String()
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
	envDir := getPythonVenvPath(pythonVersion, ctx.Project().GetName())
	if PythonTool != nil && PythonTool.version == pythonVersion && fileio.PathExists(envDir) {
		return nil
	}

	// Try to use system Python first if versions match or empty.
	useSystemPython := false
	systemPythonVer := GetDefaultPythonVersion()
	versionMatches := normalizeVersion(systemPythonVer) == normalizeVersion(pythonVersion)
	if pythonVersion == "" || versionMatches {
		useSystemPython = true
	}

	var currentPython python.Python
	var isCondaPython bool = false
	if useSystemPython { // Use system Python if version matches.
		currentPython = &python.SystemPython{}
	} else { // Fallback to conda if system Python doesn't match or doesn't exist.
		condaTool, err := FindBuildTool(ctx, "conda")
		if err != nil {
			return fmt.Errorf("failed to find conda tool for python setup -> %w", err)
		}
		currentPython = python.NewCondaPython(ctx, condaTool.Archive, condaTool.Version, pythonVersion)
		isCondaPython = true
	}

	if err := currentPython.Setup(); err != nil {
		return fmt.Errorf("failed to setup Python %s -> %w", pythonVersion, err)
	}

	// Create version specific python environment if not exist.
	pythonExec, err := currentPython.GetExecutable()
	if err != nil {
		return fmt.Errorf("failed to detect python executable for version %s -> %w", pythonVersion, err)
	}

	// For conda Python, compute the lib directory for LD_LIBRARY_PATH on Linux/macOS
	var condaLibDir string
	if isCondaPython && runtime.GOOS != "windows" {
		// conda python executable is typically at {conda_env}/bin/python
		// so its lib is at {conda_env}/lib
		pythonDir := filepath.Dir(pythonExec) // get the bin directory.
		condaLibDir = filepath.Join(filepath.Dir(pythonDir), "lib")
	}

	// Ensure virtual environment exists.
	if !fileio.PathExists(envDir) {
		// Detect Python major version to choose appropriate venv creation method
		majorVersion := "3"
		if strings.HasPrefix(pythonVersion, "2") {
			majorVersion = "2"
		}

		var command string
		if majorVersion == "2" {
			// Python2 doesn't have built-in venv module, need to use virtualenv package.
			// First, ensure virtualenv is installed.
			var installBuilder strings.Builder
			if condaLibDir != "" {
				fmt.Fprintf(&installBuilder, "LD_LIBRARY_PATH=%s ", condaLibDir)
			}
			fmt.Fprintf(&installBuilder, "%s -m pip install --quiet virtualenv", pythonExec)
			installCmd := installBuilder.String()

			executor := cmd.NewExecutor("[install virtualenv for python2]", installCmd)
			if err := executor.Execute(); err != nil {
				return fmt.Errorf("failed to install virtualenv for Python2 -> %w", err)
			}

			// Create venv using virtualenv.
			var cmdBuilder strings.Builder
			if condaLibDir != "" {
				fmt.Fprintf(&cmdBuilder, "LD_LIBRARY_PATH=%s ", condaLibDir)
			}
			fmt.Fprintf(&cmdBuilder, "%s -m virtualenv %s", pythonExec, envDir)
			command = cmdBuilder.String()
		} else {
			// Python3 has built-in venv module.
			var installBuilder strings.Builder
			if condaLibDir != "" {
				fmt.Fprintf(&installBuilder, "LD_LIBRARY_PATH=%s ", condaLibDir)
			}
			fmt.Fprintf(&installBuilder, "%s -m venv %s", pythonExec, envDir)
			command = installBuilder.String()
		}

		executor := cmd.NewExecutor("[create python venv]", command)
		if err := executor.Execute(); err != nil {
			return fmt.Errorf("failed to create python venv -> %w", err)
		}
	}

	// Use virtual environment python with platform-specific paths.
	// Detect Python major version for correct executable name
	majorVersion := "3"
	if strings.HasPrefix(pythonVersion, "2") {
		majorVersion = "2"
	}

	var venvPythonPath, venvPipPath, venvBinDir string
	if runtime.GOOS == "windows" {
		venvPythonPath = filepath.Join(envDir, "Scripts", "python.exe")
		venvPipPath = filepath.Join(envDir, "Scripts", "pip.exe")
		venvBinDir = filepath.Join(envDir, "Scripts")
	} else {
		pythonExeName := expr.If(majorVersion == "2", "python", "python3")
		venvPythonPath = filepath.Join(envDir, "bin", pythonExeName)
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
		Path:          venvPythonPath,
		rootDir:       venvBinDir,
		venvDir:       envDir,
		version:       pythonVersion,
		ldLibraryPath: condaLibDir,
	}
	return nil
}

func getPythonVenvPath(pythonVersion, projectName string) string {
	// Normalize version to minor version format for directory name (e.g., 3.10.5 -> 3.10)
	minorVersion := pythonVersion
	if strings.Count(pythonVersion, ".") > 1 {
		parts := strings.Split(pythonVersion, ".")
		minorVersion = parts[0] + "." + parts[1]
	}
	return filepath.Join(dirs.WorkspaceDir, "installed", fmt.Sprintf("venv-%s@%s", minorVersion, projectName))
}

// GetDefaultPythonVersion returns the default Python version for the current platform.
// - Windows: reads from buildtools/static TOML python tool definition (via GetDefaultPythonVersion from build_tools.go)
// - Linux/macOS: returns detected system python3 version.
func GetDefaultPythonVersion() string {
	// For Windows, delegate to build_tools implementation.
	if runtime.GOOS == "windows" {
		return getWindowsDefaultPythonVersion()
	} else { // For Linux/macOS, detect system python3 version.
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

	// Get package name without version.
	packageName = strings.Split(packageName, "==")[0]

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

	pythonTool, err := buildTools.findTool(nil, "python3")
	if err == nil && pythonTool != nil && pythonTool.Version != "" {
		return pythonTool.Version
	}

	panic("failed to read windows default python version")
}

func shouldUseConda(ctx context.Context) bool {
	// If no Python config or version is specified, use system Python.
	pythonConfig := ctx.PythonConfig()
	if pythonConfig == nil || pythonConfig.GetVersion() == "" {
		return false
	}

	// Use conda if specified Python version doesn't match system
	// default version to avoid potential version mismatch issues.
	systemDefault := GetDefaultPythonVersion()
	return normalizeVersion(pythonConfig.GetVersion()) != normalizeVersion(systemDefault)
}

func normalizeVersion(fullVersion string) string {
	parts := strings.Split(fullVersion, ".")
	if len(parts) >= 2 {
		return parts[0] + "." + parts[1]
	}
	return fullVersion
}

type pythonTool struct {
	Path          string
	rootDir       string
	venvDir       string
	version       string
	ldLibraryPath string
}

func (p pythonTool) LdLibraryPath() string {
	return p.ldLibraryPath
}

func (p pythonTool) VenvDir() string {
	return p.venvDir
}
