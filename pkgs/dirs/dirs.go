package dirs

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"unicode"
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
	PythonUserBase   string // "PYTHONUSERBASE"
	TmpDir           string // "tmp"
	TmpFilesDir      string // "tmp/files"
	TmpDepsDir       string // "tmp/deps"
	TestCacheDir     string // "cachedir"
)

func init() {
	currentDir, err := os.Getwd()
	if err != nil {
		panic(fmt.Errorf("cannot get current dir: %w", err))
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
	PythonUserBase = filepath.Join(WorkspaceDir, ".venv")
	TmpDir = filepath.Join(WorkspaceDir, "tmp")
	TmpFilesDir = filepath.Join(WorkspaceDir, "tmp", "files")
	TmpDepsDir = filepath.Join(WorkspaceDir, "tmp", "deps")
	TestCacheDir = filepath.Join(WorkspaceDir, "cachedir")
}

// GetPortDir returns the port directory path with first-letter classification.
// For example: GetPortDir("glog", "0.6.0") returns "ports/g/glog/0.6.0"
func GetPortDir(name, version string) string {
	if name == "" {
		return ""
	}

	// Get first character and convert to lowercase
	firstChar := strings.ToLower(string([]rune(name)[0]))

	// Use only letters/digits, default to "other" for special chars
	r := []rune(firstChar)[0]
	if !unicode.IsLetter(r) && !unicode.IsDigit(r) {
		firstChar = "other"
	}

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

func RemoveAllForTest() {
	os.RemoveAll(filepath.Join(WorkspaceDir, "celer.toml"))
	os.RemoveAll(TmpDir)
	os.RemoveAll(TestCacheDir)
	os.RemoveAll(PackagesDir)
	os.RemoveAll(InstalledDir)
	os.RemoveAll(BuildtreesDir)
}

func cleanRepos(buildtreesDir string) error {
	// Check buildtrees dir.
	if !pathExists(buildtreesDir) {
		return nil
	}

	// Read sub dirs to git clean src folders.
	entities, err := os.ReadDir(buildtreesDir)
	if err != nil {
		return fmt.Errorf("cannot read buildtrees dir: %w", err)
	}
	for _, entity := range entities {
		buildDir := filepath.Join(buildtreesDir, entity.Name())
		entities, err := os.ReadDir(buildDir)
		if err != nil {
			return fmt.Errorf("cannot read build dir: %w", err)
		}

		for _, entity := range entities {
			if entity.Name() == "src" {
				repoDir := filepath.Join(buildDir, entity.Name())
				if !pathExists(filepath.Join(repoDir, ".git")) {
					continue
				}
				if err := os.Chdir(repoDir); err != nil {
					return fmt.Errorf("cannot change dir to repo dir: %w", err)
				}

				cmd := exec.Command("git", "clean", "-xfd")
				if err := cmd.Start(); err != nil {
					return err
				}

				cmd = exec.Command("git", "reset", "--hard")
				if err := cmd.Start(); err != nil {
					return err
				}
			}
		}
	}

	return nil
}

func pathExists(path string) bool {
	_, err := os.Stat(path)
	if err == nil {
		return true
	}

	return !os.IsNotExist(err)
}
