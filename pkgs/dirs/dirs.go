package dirs

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"time"
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
	TestPkgCacheDir  string // "pkg-cache"
)

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

// NewTmpFilesDir creates a unique timestamped directory under tmp and
// returns its path.
func NewTmpFilesDir() (string, error) {
	tmpRoot := filepath.Join(WorkspaceDir, "tmp")
	if err := os.MkdirAll(tmpRoot, os.ModePerm); err != nil {
		return "", fmt.Errorf("cannot mkdir tmp dir '%s' -> %w", tmpRoot, err)
	}

	// Create unique tmp files dir with nano timestamp.
	dir, err := os.MkdirTemp(tmpRoot, fmt.Sprintf("files-%d-*", time.Now().UnixNano()))
	if err != nil {
		return "", fmt.Errorf("cannot create tmp files dir -> %w", err)
	}
	return dir, nil
}

// NewTmpStagingDir creates a unique timestamped directory under tmp and
// returns its path.
func NewTmpStagingDir(nameVersion string) (string, error) {
	tmpRoot := filepath.Join(WorkspaceDir, "tmp")
	if err := os.MkdirAll(tmpRoot, os.ModePerm); err != nil {
		return "", fmt.Errorf("cannot mkdir tmp dir '%s' -> %w", tmpRoot, err)
	}

	// Create unique tmp staging dir with nano timestamp.
	dir, err := os.MkdirTemp(tmpRoot, fmt.Sprintf("staging-%s-%d-*", nameVersion, time.Now().UnixNano()))
	if err != nil {
		return "", fmt.Errorf("cannot create tmp staging dir -> %w", err)
	}
	return dir, nil
}

func RemoveAllForTest() {
	os.RemoveAll(filepath.Join(WorkspaceDir, "celer.toml"))
	os.RemoveAll(TmpDir)
	os.RemoveAll(TestPkgCacheDir)
	os.RemoveAll(PackagesDir)
	os.RemoveAll(InstalledDir)
	os.RemoveAll(BuildtreesDir)
}

func init() {
	currentDir, err := os.Getwd()
	if err != nil {
		panic(fmt.Errorf("cannot get current dir -> %w", err))
	}

	// Find workspace dir from current dir.
	workspaceDir := findWorkspaceRoot(currentDir)
	Init(workspaceDir)
}

// findWorkspaceRoot walks up from startDir until it finds a directory
// containing celer.toml. If not found, returns startDir unchanged.
func findWorkspaceRoot(currentDir string) string {
	dir := currentDir
	for {
		if _, err := os.Stat(filepath.Join(dir, "celer.toml")); err == nil {
			return dir
		}

		// If reached root, fall back to original dir.
		parent := filepath.Dir(dir)
		if parent == dir {
			return currentDir
		}
		dir = parent
	}
}
