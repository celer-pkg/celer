package python

import (
	"fmt"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/cmd"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

// Python interface defines methods for Python tools, including system Python and conda Python.
type Python interface {
	Setup() error
	GetVersion() string
	GetExecutable() (string, error)
}

// GetSystemPythonVersion detects the system python version.
func GetSystemPythonVersion() string {
	var (
		output []byte
		err    error
	)

	if runtime.GOOS == "windows" {
		// On Windows, try python.exe or python3.exe
		output, err = exec.Command("python", "--version").Output()
		if err != nil {
			output, err = exec.Command("python3", "--version").Output()
		}
	} else {
		// On Unix-like systems, try python3
		output, err = exec.Command("python3", "--version").Output()
		if err != nil {
			output, err = exec.Command("python", "--version").Output()
		}
	}

	if err != nil {
		return ""
	}

	// Parse "Python 3.11.0" -> "3.11.0"
	versionStr := strings.TrimSpace(string(output))
	parts := strings.Fields(versionStr)
	if len(parts) >= 2 {
		return parts[1]
	}
	return ""
}

type PythonTool struct {
	Path          string
	Version       string
	rootDir       string
	venvDir       string
	ldLibraryPath string
}

func NewPythonTool(path, rootDir, venvDir, version, ldLibraryPath string) *PythonTool {
	return &PythonTool{
		Path:          path,
		rootDir:       rootDir,
		venvDir:       venvDir,
		Version:       version,
		ldLibraryPath: ldLibraryPath,
	}
}

func (p PythonTool) LdLibraryPath() string {
	return p.ldLibraryPath
}

func (p PythonTool) VenvDir() string {
	return p.venvDir
}

// SitePackagesDir returns the relative path from prefix to site-packages.
// e.g. "lib/python3.10/site-packages" on Linux, "Lib/site-packages" on Windows.
func (p PythonTool) SitePackagesDir() string {
	if runtime.GOOS == "windows" {
		return filepath.Join("Lib", "site-packages")
	}

	minorVersion := p.Version
	if strings.Count(p.Version, ".") > 1 {
		parts := strings.Split(p.Version, ".")
		minorVersion = parts[0] + "." + parts[1]
	}

	return filepath.Join("lib", "python"+minorVersion, "site-packages")
}

// RegisterExprVars registers all Python-related expression variables.
func (p PythonTool) RegisterExprVars(exprVars *context.ExprVars) {
	if p.Path == "" {
		return
	}
	exprVars.Put("PYTHON_PATH", fileio.ToRelPath(p.Path))
	exprVars.Put("PYTHON_VENV_DIR", fileio.ToRelPath(p.venvDir))
}

// InstallFromWheelhouse installs the specs using only the wheels in
// wheelhouseDir (--no-index --find-links), i.e. fully offline.
func (p PythonTool) InstallFromWheelhouse(specs []string, wheelhouseDir string) error {
	var builder strings.Builder
	if p.LdLibraryPath() != "" {
		fmt.Fprintf(&builder, "LD_LIBRARY_PATH=%s ", p.LdLibraryPath())
	}
	builder.WriteString(p.Path)
	builder.WriteString(" -m pip install --no-index --find-links=")
	builder.WriteString(wheelhouseDir)
	builder.WriteString(" ")
	builder.WriteString(strings.Join(specs, " "))

	executor := cmd.NewExecutor("[python3 install from wheelhouse]", builder.String())
	return executor.Execute()
}

// pipDownload runs `pip download` to materialize the full resolved wheel set
// (including transitive deps) into destDir.
func (p PythonTool) PipDownload(specs []string, destDir string, pipConfig context.PythonConfig) error {
	var builder strings.Builder
	if p.ldLibraryPath != "" {
		fmt.Fprintf(&builder, "LD_LIBRARY_PATH=%s ", p.ldLibraryPath)
	}
	builder.WriteString(p.Path)
	builder.WriteString(" -m pip download -d ")
	builder.WriteString(destDir)
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
	builder.WriteString(strings.Join(specs, " "))

	executor := cmd.NewExecutor("[python3 download wheels]", builder.String())
	return executor.Execute()
}
