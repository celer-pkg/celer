package buildtools

import (
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"slices"
	"strings"

	"github.com/celer-pkg/celer/buildtools/python"
	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/envs"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/cmd"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/expr"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/celer-pkg/celer/pkgs/logger"

	"github.com/BurntSushi/toml"
)

var PythonTool *python.PythonTool

func pipInstall(ctx context.Context, pipConfig context.PythonConfig, packages *[]string) error {
	// Get python version from project config if available, otherwise use default version.
	pythonVersion := GetDefaultPythonVersion()
	pythonConfig := ctx.PythonConfig()
	if pythonConfig != nil && pythonConfig.GetVersion() != "" {
		pythonVersion = pythonConfig.GetVersion()
	}
	venvDir := getPythonVenvPath(pythonVersion, ctx.Project().GetName())

	// Setup python using conda.
	if err := setupPython(ctx, pythonVersion); err != nil {
		return fmt.Errorf("failed to setup python -> %w", err)
	}

	// Collect python package specs that are not installed in the venv.
	// python3:mako@1.2.0 -> mako==1.2.0
	var specs []string
	for _, pkg := range *packages {
		if !strings.HasPrefix(pkg, "python3:") && !strings.HasPrefix(pkg, "python:") {
			continue
		}

		// Format python3 library name version.
		var nameVersion string
		if after, ok := strings.CutPrefix(pkg, "python:"); ok {
			nameVersion = after
		} else {
			nameVersion = strings.TrimPrefix(pkg, "python3:")
		}
		nameVersion = strings.ReplaceAll(nameVersion, "@", "==")

		if isPackageInstalled(nameVersion, venvDir) {
			continue
		}
		specs = append(specs, nameVersion)
	}

	// Nothing to do; still make sure the python bin dir is on PATH.
	if len(specs) == 0 {
		return finishPipInstall(packages, venvDir)
	}

	// Persistent wheelhouse per python minor version, shared across all specs as
	// pip's --find-links target.
	minorVersion := normalizeVersion(pythonVersion)
	wheelhouseDir := filepath.Join(dirs.DownloadsDir, "wheelhouse-"+minorVersion)
	if err := os.MkdirAll(wheelhouseDir, os.ModePerm); err != nil {
		return fmt.Errorf("failed to mkdir wheelhouse '%s' -> %w", wheelhouseDir, err)
	}

	pkgCache := ctx.PkgCache()
	var wheelCache pkgcache.PythonWheelCache
	if pkgCache != nil {
		wheelCache = pkgCache.GetPythonWheelCache()
	}
	canCache := pkgCache != nil &&
		pkgCache.GetOptions().Writable &&
		pkgCache.GetOptions().PythonWheels &&
		!ctx.Offline() &&
		wheelCache != nil
	wheelPlatformScope := fmt.Sprintf("py%s-%s-%s", minorVersion, runtime.GOOS, runtime.GOARCH)

	// L1: Try to install every spec straight from the persistent wheelhouse.
	if wheelhouseHasWheels(wheelhouseDir) {
		if err := PythonTool.InstallFromWheelhouse(specs, wheelhouseDir); err == nil {
			return finishPipInstall(packages, venvDir)
		}
	}

	// Process each spec independently so its wheels are cached under
	// python-wheels/<platform>/<name>/<version>/ and reused across machines.
	for _, spec := range specs {
		name, version := parseSpec(spec)
		cacheKey := filepath.Join(wheelPlatformScope, name, version)

		// L2: Restore this spec's cached wheels from pkgcache into the wheelhouse.
		restored := false
		if canCache {
			if ok, err := wheelCache.Restore(cacheKey, wheelhouseDir); err != nil {
				logger.Printf(logger.Warning, "[✘] failed to restore python wheel cache for '%s': %v\n", spec, err)
			} else {
				restored = ok
			}
		}

		// L3: On cache miss, download the resolved wheel set (include its dependencies)
		// into a fresh tmp dir, merge into the wheelhouse, and store into pkgcache finally.
		if !restored {
			tmpDir, err := dirs.NewTmpFilesDir()
			if err != nil {
				return fmt.Errorf("failed to create tmp dir for pip download -> %w", err)
			}
			if err := PythonTool.PipDownload([]string{spec}, tmpDir, pipConfig); err != nil {
				os.RemoveAll(tmpDir)
				return fmt.Errorf("failed to download python wheels %v -> %w", spec, err)
			}

			if err := fileio.MergeDir(tmpDir, wheelhouseDir); err != nil {
				os.RemoveAll(tmpDir)
				return fmt.Errorf("failed to merge wheels into wheelhouse -> %w", err)
			}

			if canCache {
				if err := wheelCache.Store(cacheKey, tmpDir); err != nil {
					logger.Printf(logger.Warning, "[✘] failed to cache python wheels for '%s': %v\n", spec, err)
				}
			}
			os.RemoveAll(tmpDir)
		}

		// Install this spec from the wheelhouse finally.
		if err := PythonTool.InstallFromWheelhouse([]string{spec}, wheelhouseDir); err != nil {
			return fmt.Errorf("failed to install python package '%s' -> %w", spec, err)
		}
	}
	return finishPipInstall(packages, venvDir)
}

// finishPipInstall removes python3: entries from the libraries list and ensures
// the venv bin dir is on PATH.
func finishPipInstall(packages *[]string, venvDir string) error {
	// Remove python3:xxx from list.
	*packages = slices.DeleteFunc(*packages, func(element string) bool {
		return strings.HasPrefix(element, "python3")
	})

	// Always ensure Python bin directory is in PATH.
	envs.AppendPythonBinDir(venvDir)
	return nil
}

// wheelhouseHasWheels reports whether dir contains at least one .whl file.
func wheelhouseHasWheels(dir string) bool {
	matches, _ := filepath.Glob(filepath.Join(dir, "*.whl"))
	return len(matches) > 0
}

// parseSpec splits a pip requirement specifier "name==version" into its name
// and version. A spec without a version pins nothing and returns an empty version.
func parseSpec(spec string) (name, version string) {
	if name, version, ok := strings.Cut(spec, "=="); ok {
		return name, version
	}
	return spec, ""
}

// setupPython sets up Python with a specific version.
// Strategy: Try system Python first, fallback to conda if version mismatches.
func setupPython(ctx context.Context, pythonVersion string) error {
	// Quick return only if version hasn't changed AND venv still exists on disk.
	envDir := getPythonVenvPath(pythonVersion, ctx.Project().GetName())
	if PythonTool != nil && PythonTool.Version == pythonVersion && fileio.PathExists(envDir) {
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
	PythonTool = python.NewPythonTool(venvPythonPath, venvBinDir, envDir, pythonVersion, condaLibDir)
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

	// Split into name and optional version:
	// "MarkupSafe==2.1.0" -> ("MarkupSafe", "2.1.0").
	name, version, _ := strings.Cut(packageName, "==")

	// versionGlob is the version segment in dist-info/egg-info dir names:
	// exact "2.1.0" when pinned, or "*" when any version is acceptable.
	versionGlob := "*"
	if version != "" {
		versionGlob = version
	}

	var packageDirPattern, distInfoPattern, eggInfoPattern string
	switch runtime.GOOS {
	case "windows":
		// Windows: Lib/site-packages/{name} and Lib/site-packages/{name}-{version}.dist-info.
		packageDirPattern = filepath.Join(libDir, "site-packages", name)
		distInfoPattern = filepath.Join(libDir, "site-packages", name+"-"+versionGlob+".dist-info")
		eggInfoPattern = filepath.Join(libDir, "site-packages", name+"-"+versionGlob+".egg-info")

	case "linux", "darwin":
		// Linux/Darwin: lib/python*/site-packages/{name} and lib/python*/site-packages/{name}-{version}.dist-info.
		packageDirPattern = filepath.Join(libDir, "python*", "site-packages", name)
		distInfoPattern = filepath.Join(libDir, "python*", "site-packages", name+"-"+versionGlob+".dist-info")
		eggInfoPattern = filepath.Join(libDir, "python*", "site-packages", name+"-"+versionGlob+".egg-info")

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

	matches, err = filepath.Glob(eggInfoPattern)
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
