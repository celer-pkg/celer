package pkgcache

import (
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"sync"

	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

// chattrWarningOnce ensures the chattr +a failure warning is only printed once
// per process, to avoid noisy repeated warnings when no sudoers rule is configured.
var chattrWarningOnce sync.Once

// mkdirAll creates a directory tree and applies chattr +a
// to every newly created directory within the pkgcache root.
// No-op on non-Linux or when cacheRootDir is empty.
// chattr failures are logged as warnings but do not cause the function to fail.
func mkdirAll(path string, perm os.FileMode, cacheRootDir string) error {
	// Non-Linux: fallback to plain MkdirAll.
	if runtime.GOOS != "linux" || cacheRootDir == "" {
		return os.MkdirAll(path, perm)
	}

	// Only apply chattr for directories within pkgcache.
	absPath, _ := filepath.Abs(path)
	absRoot, _ := filepath.Abs(cacheRootDir)
	if !fileio.IsSubPath(absRoot, absPath) {
		return os.MkdirAll(path, perm)
	}

	// Find which directories within pkgCacheRoot will be new.
	var newDirs []string
	cur := absPath
	for {
		if fileio.PathExists(cur) {
			break
		}
		newDirs = append(newDirs, cur)
		parent := filepath.Dir(cur)
		if parent == cur || parent == absRoot {
			break
		}
		cur = parent
	}

	// Create the directory tree.
	if err := os.MkdirAll(path, perm); err != nil {
		return err
	}

	// Apply chattr +a to newly created directories (top-down).
	for i := len(newDirs) - 1; i >= 0; i-- {
		setDirAppendOnly(newDirs[i])
	}
	return nil
}

// setDirAppendOnly sets chattr +a on a directory.
// If already running as root, runs chattr directly; otherwise uses sudo -n
// (relies on the sudoers rule installed by "celer setup").
// Failures are logged only once per process to avoid noisy output
// when no sudoers rule is configured (e.g. local dev without celer setup).
func setDirAppendOnly(dir string) {
	var cmd *exec.Cmd
	if os.Geteuid() == 0 {
		cmd = exec.Command("/usr/bin/chattr", "+a", dir)
	} else {
		cmd = exec.Command("sudo", "-n", "/usr/bin/chattr", "+a", dir)
	}
	output, err := cmd.CombinedOutput()
	if err != nil {
		chattrWarningOnce.Do(func() {
			color.PrintWarning("failed to set chattr +a on %s -> %s -> %s",
				dir,
				strings.TrimSpace(string(output)),
				"further chattr +a failures will be suppressed (run 'sudo celer setup --nfs-client-dir=xxx' to install the sudoers rule)")
		})
	}
}
