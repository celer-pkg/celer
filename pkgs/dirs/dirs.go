package dirs

import (
	"fmt"
	"os"
	"path/filepath"
)

var (
	WorkspaceDir       string // absolute dir of "."
	ConfPlatformsDir   string // absolute dir of "conf/platforms"
	ConfProjectsDir    string // absolute dir of "conf/projects"
	PortsDir           string // absolute dir of "ports"
	PackagesDir        string // absolute dir of "packages"
	DownloadedDir      string // absolute dir of "downloads"
	DownloadedToolsDir string // absolute dir of "downloaded/tools"
	InstalledDir       string // absolute dir of "installed"
	BuildtreesDir      string // absolute dir of "buildtrees"
	TmpFilesDir        string // absolute dir of "tmp/files"
	TmpDepsDir         string // absolute dir of "tmp/deps"
)

func init() {
	currentDir, err := os.Getwd()
	if err != nil {
		panic(fmt.Errorf("cannot get current dir: %w", err))
	}
	Init(currentDir)
}

// Init init with specified workspace dir.
func Init(workspaceDir string) {
	WorkspaceDir = workspaceDir
	ConfPlatformsDir = filepath.Join(WorkspaceDir, "conf", "platforms")
	ConfProjectsDir = filepath.Join(WorkspaceDir, "conf", "projects")
	PortsDir = filepath.Join(WorkspaceDir, "ports")
	PackagesDir = filepath.Join(WorkspaceDir, "packages")
	DownloadedDir = filepath.Join(WorkspaceDir, "downloads")
	DownloadedToolsDir = filepath.Join(WorkspaceDir, "downloads", "tools")
	InstalledDir = filepath.Join(WorkspaceDir, "installed")
	BuildtreesDir = filepath.Join(WorkspaceDir, "buildtrees")
	TmpFilesDir = filepath.Join(WorkspaceDir, "tmp", "files")
	TmpDepsDir = filepath.Join(WorkspaceDir, "tmp", "deps")
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
		return fmt.Errorf("cannot remove tmp dir: %w", err)
	}

	if err := os.MkdirAll(TmpFilesDir, os.ModePerm); err != nil {
		return fmt.Errorf("cannot mkdir tmp dir: %w", err)
	}

	return nil
}
