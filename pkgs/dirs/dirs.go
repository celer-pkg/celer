package dirs

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/pkgs/errors"
)

var (
	WorkspaceDir     string // "."
	ConfDir          string // "conf"
	ConfPlatformsDir string // "conf/platforms"
	ConfProjectsDir  string // "conf/projects"
	PortsDir         string // "ports"
	PackagesDir      string // "packages"
	InstalledDir     string // "installed"
	BuildtreesDir    string // "buildtrees"
	DownloadsDir     string // "downloads"
	PythonUserBase   string // "PYTHONUSERBASE"
	TmpDir           string // "tmp"
	TmpFilesDir      string // "tmp/files"
	TmpDepsDir       string // "tmp/deps"
	TestPkgCacheDir  string // "pkg-cache"
)

func init() {
	currentDir, err := os.Getwd()
	if err != nil {
		panic(fmt.Errorf("cannot get current dir -> %w", err))
	}
	Init(currentDir)
}

// Init initialize with specified workspace dir.
func Init(workspaceDir string) {
	WorkspaceDir = workspaceDir
	ConfDir = filepath.Join(WorkspaceDir, "conf")
	ConfPlatformsDir = filepath.Join(WorkspaceDir, "conf", "platforms")
	ConfProjectsDir = filepath.Join(WorkspaceDir, "conf", "projects")
	PortsDir = filepath.Join(WorkspaceDir, "ports")
	PackagesDir = filepath.Join(WorkspaceDir, "packages")
	InstalledDir = filepath.Join(WorkspaceDir, "installed")
	BuildtreesDir = filepath.Join(WorkspaceDir, "buildtrees")
	DownloadsDir = filepath.Join(WorkspaceDir, "downloads")
	PythonUserBase = filepath.Join(WorkspaceDir, ".venv")
	TmpDir = filepath.Join(WorkspaceDir, "tmp")
	TmpFilesDir = filepath.Join(WorkspaceDir, "tmp", "files")
	TmpDepsDir = filepath.Join(WorkspaceDir, "tmp", "deps")
	TestPkgCacheDir = filepath.Join(WorkspaceDir, "pkg-cache")
}

// GetPortDir returns the port directory path with first-letter classification.
// For example: GetPortDir("glog", "0.6.0") returns "ports/g/glog/0.6.0"
func GetPortDir(name, version string) string {
	if name == "" {
		return ""
	}

	// Get first character and convert to lowercase
	firstChar := strings.ToLower(string([]rune(name)[0]))
	return filepath.Join(PortsDir, firstChar, name, version)
}

// GetPortPath returns the port.toml file path with first-letter classification.
// For example: GetPortPath("glog", "0.6.0") returns "ports/g/glog/0.6.0/port.toml"
func GetPortPath(name, version string) string {
	return filepath.Join(GetPortDir(name, version), "port.toml")
}

// ResolveProjectPort returns the port.toml path for (name@version), searching
// in priority order:
//
//  1. <ConfProjectsDir>/<project>/<name>/<version>/port.toml         (project top-level)
//  2. <ConfProjectsDir>/<project>/ports/<name>/<version>/port.toml   (project vendor)
//  3. <PortsDir>/<first-char>/<name>/<version>/port.toml             (global)
//
// Returns the found path, or:
//   - ErrAmbiguousProjectPort if both (1) and (2) exist
//   - ErrPortNotFound if none of the three exists
func ResolveProjectPort(project, name, version string) (string, error) {
	topLevelPort := filepath.Join(ConfProjectsDir, project, name, version, "port.toml")
	vendorPort := filepath.Join(ConfProjectsDir, project, "ports", name, version, "port.toml")

	hasTopLevelPort := exist(topLevelPort)
	hasVendorPort := exist(vendorPort)

	switch {
	case hasTopLevelPort && hasVendorPort:
		return "", fmt.Errorf("%w: %s and %s — remove one to disambiguate",
			errors.ErrAmbiguousProjectPort, topLevelPort, vendorPort)

	case hasTopLevelPort:
		return topLevelPort, nil

	case hasVendorPort:
		return vendorPort, nil
	}

	// Fall back to the global ports/ collection.
	publicPort := GetPortPath(name, version)
	if exist(publicPort) {
		return publicPort, nil
	}

	return "", errors.ErrPortNotFound
}

// exist reports whether p exists and is a regular file.
func exist(p string) bool {
	info, err := os.Stat(p)
	if err != nil {
		return false
	}
	return !info.IsDir()
}

// ParentDir return the parent directory of path.
func ParentDir(path string, levels int) string {
	for range levels {
		parent := filepath.Dir(path)
		if parent == path {
			return parent
		}
		path = parent
	}
	return path
}

// CleanTmpFilesDir remove tmp dir and create new one.
func CleanTmpFilesDir() error {
	if err := os.RemoveAll(TmpFilesDir); err != nil {
		return fmt.Errorf("cannot remove tmp dir -> %w", err)
	}

	if err := os.MkdirAll(TmpFilesDir, os.ModePerm); err != nil {
		return fmt.Errorf("cannot mkdir tmp dir -> %w", err)
	}

	return nil
}

func RemoveAllForTest() {
	os.RemoveAll(filepath.Join(WorkspaceDir, "celer.toml"))
	os.RemoveAll(TmpDir)
	os.RemoveAll(TestPkgCacheDir)
	os.RemoveAll(PackagesDir)
	os.RemoveAll(InstalledDir)
	os.RemoveAll(BuildtreesDir)
}
