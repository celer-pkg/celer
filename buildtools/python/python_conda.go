package python

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/cmd"
	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/env"
	"github.com/celer-pkg/celer/pkgs/expr"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

type CondaPython struct {
	ctx           context.Context
	archiveName   string
	pythonVersion string
	condaVersion  string
	condaBinary   string
}

func NewCondaPython(ctx context.Context, archiveName, condaVersion, pythonVersion string) *CondaPython {
	return &CondaPython{
		ctx:           ctx,
		archiveName:   archiveName,
		condaVersion:  condaVersion,
		pythonVersion: pythonVersion,
	}
}

func (c *CondaPython) installConda(scriptPath, installDir string) error {
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
				color.PrintPass("tool: %s", "conda-"+c.condaVersion)
				color.PrintHint("Location: %s\n", installDir)
				return nil
			}
		}
		// Directory exists but conda might be broken, try to update existing installation.
		color.PrintHint("Found existing conda directory, attempting update...")
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
			return fmt.Errorf("failed to make conda script executable: %w", err)
		}

		// Run conda installer in batch mode with -b -p flags.
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
					return fmt.Errorf("failed to install/update conda: %w", err)
				}
			} else {
				return fmt.Errorf("failed to install conda: %w", err)
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
			return fmt.Errorf("failed to install conda: %w", err)
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

// GetPythonExecutable attempts to find or create Python via conda environments.
func (c *CondaPython) GetExecutable() (string, error) {
	// conda binary must be provided and exist
	if c.condaBinary == "" || !fileio.PathExists(c.condaBinary) {
		return "", fmt.Errorf("conda binary is required")
	}

	// Normalize version to minor version (e.g., 3.11.0 -> 3.11).
	minorVersion := c.pythonVersion
	if strings.Count(c.pythonVersion, ".") > 1 {
		parts := strings.Split(c.pythonVersion, ".")
		minorVersion = parts[0] + "." + parts[1]
	}

	// Use underscore in environment name instead of dots (py311 instead of python3.11).
	// This avoids issues with dots in environment names.
	envName := fmt.Sprintf("py%s", strings.ReplaceAll(minorVersion, ".", ""))

	// Try to find existing environment
	cmd := exec.Command(c.condaBinary, "run", "-n", envName, "python", "--version")
	if err := cmd.Run(); err == nil {
		var (
			output []byte
			err    error
		)

		// Environment exists, get the full path using platform-specific method.
		if runtime.GOOS == "windows" {
			output, err = exec.Command(c.condaBinary, "run", "-n", envName, "python", "-c", "import sys; print(sys.executable)").Output()
		} else {
			output, err = exec.Command(c.condaBinary, "run", "-n", envName, "which", "python").Output()
		}
		if err != nil {
			return "", fmt.Errorf("failed to get Python executable path: %w", err)
		}

		if len(output) > 0 {
			return strings.TrimSpace(string(output)), nil
		}
	}

	// Environment not found, attempt to create it with the specified Python version.
	// Use conda-forge channel as the default source.
	color.Printf(color.Hint, "- creating conda environment for Python %s (venv name: %s)", minorVersion, envName)
	createCmd := exec.Command(c.condaBinary, "create", "-y", "-c", "conda-forge", "-n", envName, fmt.Sprintf("python=%s", minorVersion))
	if output, err := createCmd.CombinedOutput(); err != nil {
		return "", fmt.Errorf("failed to create conda environment for Python %s -> %s -> %w",
			minorVersion, string(output), err)
	}
	color.PrintInline(color.Hint, "✔ creating conda environment for Python %s (venv name: %s)", minorVersion, envName)

	// Verify the new environment was created.
	cmd = exec.Command(c.condaBinary, "run", "-n", envName, "python", "--version")
	if err := cmd.Run(); err != nil {
		return "", fmt.Errorf("python environment verification failed: %w", err)
	}

	// Get the path to the python executable in this environment.
	var output []byte
	if runtime.GOOS == "windows" {
		output, _ = exec.Command(c.condaBinary, "run", "-n", envName, "python", "-c", "import sys; print(sys.executable)").Output()
	} else {
		output, _ = exec.Command(c.condaBinary, "run", "-n", envName, "which", "python").Output()
	}
	if len(output) > 0 {
		return strings.TrimSpace(string(output)), nil
	}

	return "", fmt.Errorf("failed to get Python executable path")
}

func (c *CondaPython) GetVersion() string {
	return c.pythonVersion
}

// Setup proactively installs conda if conda is not available.
// Returns the path to the conda binary and error if installation fails.
func (c *CondaPython) Setup() error {
	// Determine the expected conda binary path
	condaInstallDir := filepath.Join(dirs.WorkspaceDir, "downloads", "tools", "conda-"+c.condaVersion)
	condaBinDir := filepath.Join(condaInstallDir, expr.If(runtime.GOOS == "windows", "Scripts", "bin"))
	condaBinary := filepath.Join(condaBinDir, expr.If(runtime.GOOS == "windows", "conda.exe", "conda"))

	// Check if conda is already installed at the expected location.
	if fileio.PathExists(condaBinary) {
		if err := exec.Command(condaBinary, "--version").Run(); err == nil {
			c.condaBinary = condaBinary
			return nil
		}
	}

	// Need to install conda - get the conda installer
	condaInstallerPath, condaInstallDir, err := c.getCondaInstallerPaths()
	if err != nil {
		return fmt.Errorf("failed to locate conda installer: %w", err)
	}

	// Execute the conda installation script.
	if err := c.installConda(condaInstallerPath, condaInstallDir); err != nil {
		return fmt.Errorf("failed to execute conda installation: %w", err)
	}

	// Update PATH to include conda's bin directory.
	condaBinDir = filepath.Join(condaInstallDir, expr.If(runtime.GOOS == "windows", "Scripts", "bin"))
	if fileio.PathExists(condaBinDir) {
		os.Setenv("PATH", env.JoinPaths("PATH", condaBinDir))
	}

	// Verify conda binary exists after installation.
	if !fileio.PathExists(condaBinary) {
		return fmt.Errorf("conda binary not found at %s after installation", condaBinary)
	}

	// Configure conda to use conda-forge channel as default.
	// This enables conda-forge packages by default.
	tosCmd := exec.Command(condaBinary, "config", "--add", "channels", "conda-forge")
	if err := tosCmd.Run(); err != nil {
		return fmt.Errorf("Note: Could not configure conda-forge channel: %v", err)
	}

	// Note: Python version installation is handled by getPythonExecutable via conda create
	// This avoids conflicts in the base environment (e.g., conda-anaconda-tos package conflicts)
	// Each Python version is created in a dedicated conda environment (e.g., py311)
	c.condaBinary = condaBinary
	return nil
}

// getCondaInstallerPaths determines the paths to the conda installer and installation directory.
// Returns installer path and installation directory
func (c *CondaPython) getCondaInstallerPaths() (string, string, error) {
	downloadsDir := c.ctx.Downloads()

	// Locate the installer in the downloads directory
	installerPath := filepath.Join(downloadsDir, c.archiveName)
	if !fileio.PathExists(installerPath) {
		return "", "", fmt.Errorf("conda installer not found at %s", installerPath)
	}

	installDir := filepath.Join(downloadsDir, "tools", "conda-"+c.condaVersion)
	return installerPath, installDir, nil
}
