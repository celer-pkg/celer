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
	"github.com/celer-pkg/celer/pkgs/env"
	"github.com/celer-pkg/celer/pkgs/expr"
	"github.com/celer-pkg/celer/pkgs/fileio"

	"github.com/BurntSushi/toml"
)

var Python *python

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
		if isPythonPackageInstalled(nameVersion, venvDir) {
			continue
		}

		// Install python3 library with path path.
		title := fmt.Sprintf("[python3 install tool %s]", nameVersion)
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

// setupPython sets up Python with a specific version via conda.
func setupPython(ctx context.Context, pythonVersion string) error {
	// Check if version changed - if so, allow reinitializing.
	if Python != nil && Python.version == pythonVersion {
		return nil
	}

	// Ensure conda is available for this Python version.
	condaBinaryPath, err := tryInstallCondaIfNeeded(ctx)
	if err != nil {
		return fmt.Errorf("failed to setup conda for Python %s -> %w", pythonVersion, err)
	}

	// Detect Python executable via conda.
	pythonExec, err := getPythonExecutable(pythonVersion, condaBinaryPath)
	if err != nil {
		return fmt.Errorf("failed to detect python executable for version %s -> %w", pythonVersion, err)
	}

	// Ensure virtual environment exists.
	envDir := getVersionedVenvPath(pythonVersion)
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
	Python = &python{
		Path:    venvPythonPath,
		rootDir: venvBinDir,
		version: pythonVersion,
	}
	return nil
}

func installConda(scriptPath, installDir string) error {
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
				color.PrintPass("tool: %s", "Miniconda")
				color.PrintHint("Location: %s\n", installDir)
				return nil
			}
		}
		// Directory exists but conda might be broken, try to update existing installation.
		color.PrintHint("Found existing Miniconda directory, attempting update...")
	} else {
		// Ensure install directory exists
		if err := os.MkdirAll(installDir, os.ModePerm); err != nil {
			return fmt.Errorf("failed to create conda install directory %s -> %w", installDir, err)
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

func checkCondaPythonVersionExists(condaBinary string, requestedVersion string) bool {
	// Normalize version to minor version (e.g., 3.11.0 -> 3.11)
	minorVersion := requestedVersion
	if strings.Count(requestedVersion, ".") > 1 {
		parts := strings.Split(requestedVersion, ".")
		minorVersion = parts[0] + "." + parts[1]
	}

	// First, check if the version-specific environment already exists (e.g., py311 for Python 3.11)
	// This is the preferred location where tryCondaPython would create it
	envName := fmt.Sprintf("py%s", strings.ReplaceAll(minorVersion, ".", ""))
	cmd := exec.Command(condaBinary, "run", "-n", envName, "python", "--version")
	if output, err := cmd.CombinedOutput(); err == nil {
		// The environment exists and Python is installed
		versionStr := strings.TrimSpace(string(output))
		parts := strings.Fields(versionStr)
		if len(parts) >= 2 {
			fullVersion := parts[1]
			versionParts := strings.Split(fullVersion, ".")
			if len(versionParts) >= 2 {
				installedMinor := versionParts[0] + "." + versionParts[1]
				if installedMinor == minorVersion {
					return true
				}
			}
		}
	}

	// Also check if the version is available in base environment as fallback
	cmd = exec.Command(condaBinary, "run", "-n", "base", "python", "--version")
	if output, err := cmd.CombinedOutput(); err == nil {
		versionStr := strings.TrimSpace(string(output))
		parts := strings.Fields(versionStr)
		if len(parts) >= 2 {
			fullVersion := parts[1]
			versionParts := strings.Split(fullVersion, ".")
			if len(versionParts) >= 2 {
				installedMinor := versionParts[0] + "." + versionParts[1]
				if installedMinor == minorVersion {
					return true
				}
			}
		}
	}

	return false
}

// tryInstallCondaIfNeeded proactively installs Miniconda if conda is not available.
// Returns the path to the conda binary and error if installation fails.
func tryInstallCondaIfNeeded(ctx context.Context) (string, error) {
	// Determine the expected conda binary path
	condaInstallDir := filepath.Join(dirs.WorkspaceDir, "downloads", "tools", "miniconda3")
	condaBinDir := filepath.Join(condaInstallDir, expr.If(runtime.GOOS == "windows", "Scripts", "bin"))
	condaBinary := filepath.Join(condaBinDir, expr.If(runtime.GOOS == "windows", "conda.exe", "conda"))

	// Check if conda is already installed at the expected location.
	if fileio.PathExists(condaBinary) {
		if err := exec.Command(condaBinary, "--version").Run(); err == nil {
			return condaBinary, nil
		}
	}

	// Need to install conda - get the Miniconda installer
	condaInstallerPath, condaInstallDir, err := getCondaInstallerPaths(ctx)
	if err != nil {
		return "", fmt.Errorf("failed to locate Miniconda installer: %w", err)
	}

	// Execute the Miniconda installation script.
	if err := installConda(condaInstallerPath, condaInstallDir); err != nil {
		return "", fmt.Errorf("failed to execute Miniconda installation: %w", err)
	}

	// Update PATH to include conda's bin directory.
	condaBinDir = filepath.Join(condaInstallDir, expr.If(runtime.GOOS == "windows", "Scripts", "bin"))
	if fileio.PathExists(condaBinDir) {
		os.Setenv("PATH", env.JoinPaths("PATH", condaBinDir))
	}

	// Verify conda binary exists after installation.
	if !fileio.PathExists(condaBinary) {
		return "", fmt.Errorf("conda binary not found at %s after installation", condaBinary)
	}

	// Accept Anaconda Terms of Service for new Miniconda versions.
	// This is required for conda to work with the default channels.
	tosCmd := exec.Command(condaBinary, "tos", "accept", "--override-channels", "--channel", "https://repo.anaconda.com/pkgs/main")
	if err := tosCmd.Run(); err != nil {
		return "", fmt.Errorf("Note: Could not auto-accept conda ToS, some channels may require manual acceptance: %v", err)
	}

	// Note: Python version installation is handled by getPythonExecutable via conda create
	// This avoids conflicts in the base environment (e.g., conda-anaconda-tos package conflicts)
	// Each Python version is created in a dedicated conda environment (e.g., py311)
	return condaBinary, nil
}

// getCondaInstallerPaths determines the paths to the Miniconda installer and installation directory.
// Returns installer path and installation directory (e.g., miniconda3)
func getCondaInstallerPaths(ctx context.Context) (string, string, error) {
	downloadsDir := ctx.Downloads()

	// Get conda archive filename from TOML configuration.
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

// tryCondaPython attempts to find or create Python via conda environments.
func getPythonExecutable(version string, condaBinary string) (string, error) {
	// conda binary must be provided and exist
	if condaBinary == "" || !fileio.PathExists(condaBinary) {
		return "", fmt.Errorf("conda binary is required")
	}

	// Normalize version to minor version (e.g., 3.11.0 -> 3.11).
	minorVersion := version
	if strings.Count(version, ".") > 1 {
		parts := strings.Split(version, ".")
		minorVersion = parts[0] + "." + parts[1]
	}

	// Use underscore in environment name instead of dots (py311 instead of python3.11).
	// This avoids issues with dots in environment names.
	envName := fmt.Sprintf("py%s", strings.ReplaceAll(minorVersion, ".", ""))

	// Try to find existing environment
	cmd := exec.Command(condaBinary, "run", "-n", envName, "python", "--version")
	if err := cmd.Run(); err == nil {
		var (
			output []byte
			err    error
		)

		// Environment exists, get the full path using platform-specific method.
		if runtime.GOOS == "windows" {
			output, err = exec.Command(condaBinary, "run", "-n", envName, "python", "-c", "import sys; print(sys.executable)").Output()
		} else {
			output, err = exec.Command(condaBinary, "run", "-n", envName, "which", "python").Output()
		}
		if err != nil {
			return "", fmt.Errorf("failed to get Python executable path: %w", err)
		}

		if len(output) > 0 {
			return strings.TrimSpace(string(output)), nil
		}
	}

	// Environment not found, attempt to create it with the specified Python version.
	// Use conda-forge channel as fallback to ensure Python versions are available.
	// Use --override-channels to avoid Terms of Service issues with default channels.
	color.Printf(color.Hint, "- creating conda environment for Python %s (venv name: %s)", minorVersion, envName)
	createCmd := exec.Command(condaBinary, "create", "-y", "--override-channels", "-c", "conda-forge", "-n", envName, fmt.Sprintf("python=%s", minorVersion))
	if output, err := createCmd.CombinedOutput(); err != nil {
		return "", fmt.Errorf("failed to create conda environment for Python %s -> %s -> %w",
			minorVersion, string(output), err)
	}
	color.PrintInline(color.Hint, "✔ creating conda environment for Python %s (venv name: %s)", minorVersion, envName)

	// Verify the new environment was created.
	cmd = exec.Command(condaBinary, "run", "-n", envName, "python", "--version")
	if err := cmd.Run(); err != nil {
		return "", fmt.Errorf("python environment verification failed: %w", err)
	}

	// Get the path to the python executable in this environment.
	var output []byte
	if runtime.GOOS == "windows" {
		output, _ = exec.Command(condaBinary, "run", "-n", envName, "python", "-c", "import sys; print(sys.executable)").Output()
	} else {
		output, _ = exec.Command(condaBinary, "run", "-n", envName, "which", "python").Output()
	}
	if len(output) > 0 {
		return strings.TrimSpace(string(output)), nil
	}

	return "", fmt.Errorf("failed to get Python executable path")
}

// isPythonPackageInstalled checks if a Python package is already installed.
// This avoids frequent PyPI requests that could lead to IP blocking.
func isPythonPackageInstalled(packageName string, venvDir string) bool {
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

func getVersionedVenvPath(pythonVersion string) string {
	// Normalize version to minor version format for directory name (e.g., 3.10.5 -> 3.10)
	minorVersion := pythonVersion
	if strings.Count(pythonVersion, ".") > 1 {
		parts := strings.Split(pythonVersion, ".")
		minorVersion = parts[0] + "." + parts[1]
	}
	return filepath.Join(dirs.WorkspaceDir, "installed", fmt.Sprintf("venv-%s", minorVersion))
}

type python struct {
	Path    string
	rootDir string
	version string
}
