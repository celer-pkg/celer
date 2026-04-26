package buildtools

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"slices"
	"strings"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/envs"
	"github.com/celer-pkg/celer/pkgs/cmd"
	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/expr"
	"github.com/celer-pkg/celer/pkgs/fileio"

	"github.com/BurntSushi/toml"
)

var (
	Python               *python
	currentPythonVersion string // Track current version to allow switching between projects.
)

// installMiniconda runs the Miniconda installation script in batch mode.
func installMiniconda(scriptPath, installDir string) error {
	if !fileio.PathExists(scriptPath) {
		return fmt.Errorf("Miniconda script not found at %s", scriptPath)
	}

	// If installation directory already exists, check if conda is already functional.
	if fileio.PathExists(installDir) {
		binDir := expr.If(runtime.GOOS == "windows", "Scripts", "bin")
		condaName := expr.If(runtime.GOOS == "windows", "conda.exe", "conda")
		condaBinary := filepath.Join(installDir, binDir, condaName)
		if fileio.PathExists(condaBinary) {
			if err := exec.Command(condaBinary, "--version").Run(); err == nil {
				color.PrintHint("Miniconda already installed at %s", installDir)
				return nil
			}
		}
		// Directory exists but conda might be broken, try to update existing installation.
		color.PrintHint("Found existing Miniconda directory, attempting update...")
	} else {
		// Ensure install directory exists
		if err := os.MkdirAll(installDir, os.ModePerm); err != nil {
			return fmt.Errorf("failed to create conda install directory %s: %w", installDir, err)
		}
	}

	switch runtime.GOOS {
	case "linux", "darwin":
		// Make script executable on Unix systems.
		if err := os.Chmod(scriptPath, os.ModePerm); err != nil {
			return fmt.Errorf("failed to make Miniconda script executable: %w", err)
		}

		// Run Miniconda installer in batch mode with -b -p flags.
		// -b: batch mode (no interactive prompts)
		// -p: installation prefix (directory)
		// -u: update existing installation (if directory already exists)
		command := fmt.Sprintf("bash %s -b -p %s", scriptPath, installDir)
		executor := cmd.NewExecutor("[conda install]", command)
		if err := executor.Execute(); err != nil {
			// If installation failed and directory exists, try update mode.
			if fileio.PathExists(installDir) {
				color.PrintHint("Initial installation failed, trying update mode...")
				command := fmt.Sprintf("bash %s -b -u -p %s", scriptPath, installDir)
				executor := cmd.NewExecutor("[conda install in update mode]", command)
				if err := executor.Execute(); err != nil {
					return fmt.Errorf("failed to install/update Miniconda: %w", err)
				}
			} else {
				return fmt.Errorf("failed to install Miniconda: %w", err)
			}
		}

		// Verify conda binary exists.
		condaBinary := filepath.Join(installDir, "bin", "conda")
		if !fileio.PathExists(condaBinary) {
			return fmt.Errorf("conda binary not found after installation at %s", condaBinary)
		}

	case "windows":
		// On Windows, run .exe installer with /S (silent mode) and /D= (installation directory).
		command := fmt.Sprintf("%s /S /D=%s", scriptPath, installDir)
		executor := cmd.NewExecutor("[conda install]", command)
		if err := executor.Execute(); err != nil {
			return fmt.Errorf("failed to install Miniconda: %w", err)
		}

		// Verify conda binary exists
		condaBinary := filepath.Join(installDir, "Scripts", "conda.exe")
		if !fileio.PathExists(condaBinary) {
			return fmt.Errorf("conda binary not found after installation at %s", condaBinary)
		}

	default:
		return fmt.Errorf("unsupported OS for conda installation: %s", runtime.GOOS)
	}

	return nil
}

// tryInstallCondaIfNeeded proactively installs Miniconda if conda is not available in PATH.
// It installs a version-specific Miniconda so different Python versions can coexist.
// Returns nil if conda is already installed or installation succeeds; error otherwise.
func tryInstallCondaIfNeeded(ctx context.Context, pythonVersion string) error {
	// Check if conda already exists in PATH
	if _, err := exec.LookPath("conda"); err == nil {
		// conda is available, no need to install
		return nil
	}

	// Normalize version to minor version (e.g., 3.11.0 -> 3.11)
	minorVersion := pythonVersion
	if strings.Count(pythonVersion, ".") > 1 {
		parts := strings.Split(pythonVersion, ".")
		minorVersion = parts[0] + "." + parts[1]
	}

	color.PrintHint("conda not found, attempting to install Miniconda for Python %s...", minorVersion)

	// Now we need to execute the Miniconda installer script
	// Determine the path to the downloaded installer (version-specific)
	condaInstallerPath, condaInstallDir, err := getCondaInstallerPaths(ctx)
	if err != nil {
		return fmt.Errorf("failed to locate Miniconda installer: %w", err)
	}

	// Execute the Miniconda installation script.
	if err := installMiniconda(condaInstallerPath, condaInstallDir); err != nil {
		return fmt.Errorf("failed to execute Miniconda installation: %w", err)
	}

	// Update PATH to include conda's bin directory.
	condaBinDir := filepath.Join(condaInstallDir, expr.If(runtime.GOOS == "windows", "Scripts", "bin"))
	if fileio.PathExists(condaBinDir) {
		os.Setenv("PATH", condaBinDir+string(filepath.ListSeparator)+os.Getenv("PATH"))
	}

	// Get the path to conda binary (works whether conda is in PATH or not)
	condaBinary := filepath.Join(condaBinDir, expr.If(runtime.GOOS == "windows", "conda.exe", "conda"))

	// Verify conda is now available in PATH
	if _, err := exec.LookPath("conda"); err != nil {
		// If conda is not in PATH but the binary exists, try to verify it directly.
		if !fileio.PathExists(condaBinary) {
			return fmt.Errorf("conda binary not found at %s", condaBinary)
		}
		// Binary exists but not in PATH, this might be a PATH update issue but conda should still work.
		color.PrintHint("Miniconda installed at %s (added to PATH)", condaInstallDir)
	} else {
		color.PrintHint("Miniconda installed successfully at %s", condaInstallDir)
	}

	// Accept Anaconda Terms of Service for new Miniconda versions.
	// This is required for conda to work with the default channels.
	tosCmd := exec.Command(condaBinary, "tos", "accept", "--override-channels", "--channel", "https://repo.anaconda.com/pkgs/main")
	if err := tosCmd.Run(); err != nil {
		// Not critical - conda may still work even if ToS acceptance fails
		color.PrintHint("Note: Could not auto-accept conda ToS, some channels may require manual acceptance: %v", err)
	}

	// Install the specified Python version in the base environment directly.
	// This is faster and simpler than creating separate environments.
	// It ensures that miniconda3-python3.11/bin/python3 is actually Python 3.11 (not 3.13)
	color.PrintHint("Installing Python %s in base environment...", minorVersion)
	installCmd := exec.Command(condaBinary, "install", "-y", "--override-channels", "-c", "conda-forge", fmt.Sprintf("python=%s", minorVersion))
	if output, err := installCmd.CombinedOutput(); err != nil {
		// This is not critical - we can continue with the system Python as fallback.
		color.PrintHint("Note: Could not install Python %s in base environment, but conda is still functional: %v", minorVersion, err)
		color.PrintHint("Output: %s", string(output))
	} else {
		color.PrintHint("Successfully installed Python %s in base environment", minorVersion)
	}

	return nil
}

// getCondaInstallerPaths determines the paths to the Miniconda installer and installation directory.
// Returns installer path and installation directory (e.g., miniconda3)
// The installer filename is read from the TOML configuration to avoid hardcoding.
func getCondaInstallerPaths(ctx context.Context) (string, string, error) {
	downloadsDir := ctx.Downloads()

	// Get conda archive filename from TOML configuration
	installerFilename, err := getCondaArchiveFromConfig()
	if err != nil {
		return "", "", fmt.Errorf("failed to get conda archive filename from config: %w", err)
	}

	// Locate the installer in the downloads directory
	installerPath := filepath.Join(downloadsDir, installerFilename)
	if !fileio.PathExists(installerPath) {
		return "", "", fmt.Errorf("Miniconda installer not found at %s", installerPath)
	}

	installDir := filepath.Join(downloadsDir, "tools", "miniconda3")
	return installerPath, installDir, nil
}

// getCondaArchiveFromConfig reads the conda tool archive filename from the TOML configuration.
// This avoids hardcoding platform-specific filenames and keeps the config as a single source of truth.
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
		return "", fmt.Errorf("failed to read TOML config %s: %w", staticFile, err)
	}

	var buildTools BuildTools
	if err := toml.Unmarshal(bytes, &buildTools); err != nil {
		return "", fmt.Errorf("failed to parse TOML config %s: %w", staticFile, err)
	}

	// Find the conda tool entry
	condaTool := buildTools.findTool(nil, "conda")
	if condaTool == nil {
		return "", fmt.Errorf("conda tool not found in %s", staticFile)
	}

	// Use the Archive field if specified, otherwise use the filename from URL
	if condaTool.Archive != "" {
		return condaTool.Archive, nil
	}

	// Fallback: extract filename from URL
	return filepath.Base(condaTool.Url), nil
}

// getPythonExecutable detects and returns the path to a Python executable.
// It tries (in order): pyenv, conda, system /usr/bin/python3.X, system default /usr/bin/python3
// condaBinary: optional path to a specific conda installation to use
func getPythonExecutable(version string, condaBinary ...string) (string, error) {
	if version == "" {
		// No version specified, use system default
		return "/usr/bin/python3", nil
	}

	// Try pyenv first
	if runtime.GOOS == "linux" || runtime.GOOS == "darwin" {
		pyenvPath, err := exec.Command("pyenv", "which", version).Output()
		if err == nil {
			return strings.TrimSpace(string(pyenvPath)), nil
		}
	}

	// Try system /usr/bin/python3.X (before conda, as it's faster)
	systemPath := fmt.Sprintf("/usr/bin/python3.%s", version)
	if fileio.PathExists(systemPath) {
		return systemPath, nil
	}

	// Also try without the patch version (e.g., 3.11 -> /usr/bin/python3.11)
	if strings.Count(version, ".") > 1 {
		minorVersion := strings.Join(strings.Split(version, ".")[0:2], ".")
		systemPath := fmt.Sprintf("/usr/bin/python3.%s", minorVersion)
		if fileio.PathExists(systemPath) {
			return systemPath, nil
		}
	}

	// Try conda if available - conda can provide specific versions
	var condaPath string
	if len(condaBinary) > 0 && condaBinary[0] != "" {
		condaPath = condaBinary[0]
	}
	pythonPath := tryCondaPython(version, condaPath)
	if pythonPath != "" {
		return pythonPath, nil
	}

	// Fallback to system default python3
	return "/usr/bin/python3", nil
}

// tryCondaPython attempts to find or create Python via conda environments.
// If a specific version is requested, it will:
// 1. Try to find an existing environment
// 2. If not found, create a new conda environment with the specified Python version
// condaBinary: path to the conda executable (if available)
func tryCondaPython(version string, condaBinary string) string {
	// Check if conda command exists (either from parameter or PATH)
	var condaPath string
	if condaBinary != "" && fileio.PathExists(condaBinary) {
		condaPath = condaBinary
	} else if p, err := exec.LookPath("conda"); err == nil {
		condaPath = p
	} else {
		// conda not available, return empty
		return ""
	}

	// Normalize version to minor version (e.g., 3.11.0 -> 3.11)
	minorVersion := version
	if strings.Count(version, ".") > 1 {
		parts := strings.Split(version, ".")
		minorVersion = parts[0] + "." + parts[1]
	}

	// Use underscore in environment name instead of dots (py311 instead of python3.11)
	// This avoids issues with dots in environment names
	envName := fmt.Sprintf("py%s", strings.ReplaceAll(minorVersion, ".", ""))

	// Try to find existing environment
	cmd := exec.Command(condaPath, "run", "-n", envName, "python", "--version")
	if err := cmd.Run(); err == nil {
		// Environment exists, get the full path
		output, err := exec.Command(condaPath, "run", "-n", envName, "which", "python").Output()
		if err == nil {
			pythonPath := strings.TrimSpace(string(output))
			color.PrintHint("Found Python %s in conda environment: %s", minorVersion, pythonPath)
			return pythonPath
		}
	}

	// Environment not found, attempt to create it with the specified Python version
	// Use conda-forge channel as fallback to ensure Python versions are available
	// Use --override-channels to avoid Terms of Service issues with default channels
	color.PrintHint("Creating conda environment for Python %s (environment name: %s)...", minorVersion, envName)
	createCmd := exec.Command(condaPath, "create", "-y", "--override-channels", "-c", "conda-forge", "-n", envName, fmt.Sprintf("python=%s", minorVersion))

	// Run with output capture to help debug issues
	if output, err := createCmd.CombinedOutput(); err != nil {
		color.PrintHint("Failed to create conda environment for Python %s: %v", minorVersion, err)
		color.PrintHint("Error output: %s", string(output))
		return ""
	}

	// Verify the new environment was created
	cmd = exec.Command(condaPath, "run", "-n", envName, "python", "--version")
	if err := cmd.Run(); err != nil {
		color.PrintHint("Python environment created but verification failed: %v", err)
		return ""
	}

	// Get the path to the python executable in this environment
	var output []byte
	if runtime.GOOS == "windows" {
		// On Windows, use different command to get the path
		output, _ = exec.Command(condaPath, "run", "-n", envName, "python", "-c", "import sys; print(sys.executable)").Output()
	} else {
		// On Unix, use which command
		output, _ = exec.Command(condaPath, "run", "-n", envName, "which", "python").Output()
	}
	if len(output) > 0 {
		pythonPath := strings.TrimSpace(string(output))
		color.PrintHint("Successfully created Python %s environment: %s", minorVersion, pythonPath)
		return pythonPath
	}

	return ""
}

// setupPython sets up Python with a specific version.
// If pythonVersion is empty, uses system default (backward compatible).
// Supports switching between different Python versions for different projects.
func setupPython(ctx context.Context, rootDir string, pythonVersion string) error {
	// Check if version changed - if so, allow reinitializing
	if Python != nil {
		if currentPythonVersion == pythonVersion {
			// Same version, no need to reinitialize
			return nil
		}
		// Different version, reset to allow reinitialization
		Python = nil
		currentPythonVersion = ""
	}

	if Python != nil {
		return nil
	}

	// If a specific Python version is requested, ensure conda is available as a fallback
	if pythonVersion != "" {
		if err := tryInstallCondaIfNeeded(ctx, pythonVersion); err != nil {
			color.PrintHint("conda installation/verification failed, attempting without conda: %v", err)
			// Continue anyway - other Python detection methods may still work
		}
	}

	// Get path to version-specific conda binary if it exists
	var condaBinaryPath string
	if pythonVersion != "" {
		// Construct path to version-specific conda
		condaInstallDir := filepath.Join(dirs.WorkspaceDir, "downloads", "tools", "miniconda3")
		binDir := expr.If(runtime.GOOS == "windows", "Scripts", "bin")
		condaName := expr.If(runtime.GOOS == "windows", "conda.exe", "conda")
		condaBinaryPath = filepath.Join(condaInstallDir, binDir, condaName)
	}

	// Detect Python executable based on version (pass conda path if available)
	var pythonExec string
	var err error
	if condaBinaryPath != "" && fileio.PathExists(condaBinaryPath) {
		pythonExec, err = getPythonExecutable(pythonVersion, condaBinaryPath)
	} else {
		pythonExec, err = getPythonExecutable(pythonVersion)
	}
	if err != nil {
		return fmt.Errorf("failed to detect python executable for version %s -> %w", pythonVersion, err)
	}

	// Determine venv directory based on version
	var envDir string
	if pythonVersion != "" {
		envDir = filepath.Join(dirs.WorkspaceDir, ".venv")
	} else {
		// Default venv directory (backward compatible)
		envDir = dirs.PythonUserBase
	}

	// Init python3 for windows/linux.
	switch runtime.GOOS {
	case "linux", "darwin":
		Python = &python{
			Path:    pythonExec,
			rootDir: rootDir,
		}
	case "windows":
		Python = &python{
			Path:    pythonExec,
			rootDir: rootDir,
		}
	default:
		return fmt.Errorf("unsupported os: %s", runtime.GOOS)
	}

	// Ensure virtual environment exists.
	if !fileio.PathExists(envDir) {
		command := fmt.Sprintf("%s -m venv %s", Python.Path, envDir)
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

	if fileio.PathExists(venvPythonPath) && fileio.PathExists(venvPipPath) {
		Python.Path = venvPythonPath
		Python.rootDir = venvBinDir
	} else {
		var deleteCmd string
		if runtime.GOOS == "windows" {
			deleteCmd = fmt.Sprintf("rmdir /s /q %s", envDir)
		} else {
			deleteCmd = fmt.Sprintf("rm -rf %s", envDir)
		}
		return fmt.Errorf("python virtual environment is incomplete at %s\n "+
			"Please delete the directory and try again: %s\n",
			envDir, deleteCmd)
	}

	// Record the current Python version for future version switching
	currentPythonVersion = pythonVersion
	return nil
}

// pip3Install checks if the python3 is installed.
// ctx parameter is used to get the Python version from project configuration.
func pip3Install(ctx context.Context, python3Tool *BuildTool, libraries *[]string) error {
	// Extract python version from project if context is provided
	var pythonVersion string
	if proj := ctx.Project(); proj != nil {
		pythonVersion = proj.GetPythonVersion()
	}

	// Setup python3 if not set.
	var err error
	if python3Tool != nil {
		err = setupPython(ctx, python3Tool.rootDir, pythonVersion)
	} else if runtime.GOOS == "linux" || runtime.GOOS == "darwin" {
		err = setupPython(ctx, "/usr/bin", pythonVersion)
	} else {
		return fmt.Errorf("unsupported os: %s", runtime.GOOS)
	}
	if err != nil {
		return fmt.Errorf("failed to setup python3 -> %w", err)
	}

	// Determine venv directory for package installation
	var venvDir string
	if pythonVersion != "" {
		venvDir = filepath.Join(dirs.WorkspaceDir, ".venv")
	} else {
		venvDir = dirs.PythonUserBase
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
		if isPythonPackageInstalled(nameVersion, venvDir) {
			continue
		}

		// Install python3 library with path path.
		title := fmt.Sprintf("[python3 install tool] %s", nameVersion)
		command := fmt.Sprintf("%s -m pip install --ignore-installed %s", Python.Path, nameVersion)
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

// isPythonPackageInstalled checks if a Python package is already installed.
// This avoids frequent PyPI requests that could lead to IP blocking.
// pythonVenvDir specifies the virtual environment directory to check in.
func isPythonPackageInstalled(packageName string, venvDir string) bool {
	libDir := filepath.Join(venvDir, expr.If(runtime.GOOS == "windows", "Lib", "lib"))
	if !fileio.PathExists(libDir) {
		return false
	}

	var packageDirPattern, distInfoPattern string
	switch runtime.GOOS {
	case "windows":
		// Windows: Lib/site-packages/{packageName} and Lib/site-packages/{packageName}-*.dist-info
		packageDirPattern = filepath.Join(libDir, "site-packages", packageName)
		distInfoPattern = filepath.Join(libDir, "site-packages", packageName+"-*.dist-info")

	case "linux", "darwin":
		// Linux/Darwin: lib/python*/site-packages/{packageName} and lib/python*/site-packages/{packageName}-*.dist-info
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

type python struct {
	Path    string
	rootDir string
}
