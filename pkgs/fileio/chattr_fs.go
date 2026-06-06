package fileio

import (
	"fmt"
	"io"
	"os"
	"os/exec"
	"os/user"
	"path/filepath"
	"runtime"
	"strings"
)

// ChattrFS encapsulates chattr +a compatible file operations bound to a specific cacheRootDir.
type ChattrFS struct {
	rootDir string
}

func NewChattrFS(rootDir string) *ChattrFS {
	return &ChattrFS{rootDir: rootDir}
}

// MkdirAll creates a directory tree and applies chattr +a
// to every newly created directory within the pkgcache root.
func (fs *ChattrFS) MkdirAll(path string, perm os.FileMode) error {
	// Non-Linux: fallback to plain MkdirAll.
	if runtime.GOOS != "linux" || fs.rootDir == "" {
		return os.MkdirAll(path, perm)
	}

	// Only apply chattr for directories within path.
	absPath, _ := filepath.Abs(path)
	absRoot, _ := filepath.Abs(fs.rootDir)
	if !IsSubPath(absRoot, absPath) {
		return os.MkdirAll(path, perm)
	}

	// Find which directories within path will be new.
	var newDirs []string
	cur := absPath
	for {
		if PathExists(cur) {
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

	// Apply chattr +a to newly created directories (top-to-down).
	for i := len(newDirs) - 1; i >= 0; i-- {
		if err := fs.setDirAppendOnly(newDirs[i]); err != nil {
			return err
		}
	}

	// Always apply chattr +a to the target directory, even if it already existed.
	if err := fs.setDirAppendOnly(absPath); err != nil {
		return err
	}

	return nil
}

// CopyFile it does not remove the destination. it opens with O_WRONLY|O_CREATE|O_TRUNC.
// This is compatible with chattr +a directories where deletion is blocked.
func (fs *ChattrFS) CopyFile(src, dest string) error {
	srcFile, err := os.Open(src)
	if err != nil {
		return err
	}
	defer srcFile.Close()

	info, err := srcFile.Stat()
	if err != nil {
		return err
	}

	if err := fs.MkdirAll(filepath.Dir(dest), os.ModePerm); err != nil {
		return err
	}

	// Open with O_WRONLY|O_CREATE|O_TRUNC — creates new or truncates existing.
	destFile, err := os.OpenFile(dest, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, info.Mode())
	if err != nil {
		return err
	}
	defer destFile.Close()

	if _, err := io.Copy(destFile, srcFile); err != nil {
		return err
	}
	return destFile.Sync()
}

// WriteFile writes data to path, creating or truncating in-place.
// Unlike os.WriteFile, this sets explicit permissions, compatible with chattr +a directories.
func (fs *ChattrFS) WriteFile(path string, data []byte, perm os.FileMode) error {
	if err := fs.MkdirAll(filepath.Dir(path), os.ModePerm); err != nil {
		return err
	}
	file, err := os.OpenFile(path, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, perm)
	if err != nil {
		return err
	}

	defer file.Close()
	if _, err := file.Write(data); err != nil {
		return err
	}

	return nil
}

// setDirAppendOnly sets chattr +a on a directory.
//   - Root: run chattr directly; hard error on failure.
//   - Non-root in celer group: run chattr; skip on NFS ("Operation not supported")
//     and on insufficient privilege ("Operation not permitted", e.g. local dev),
//     hard error on other failures.
//   - Non-root not in celer group: skip (local dev environment, no NFS cache protection needed).
func (fs *ChattrFS) setDirAppendOnly(dir string) error {
	if os.Geteuid() != 0 {
		inGroup, err := isInCelerGroup()
		if err != nil || !inGroup {
			return nil // Not in celer group — likely a local dev environment without NFS cache.
		}
	}

	cmd := exec.Command("/usr/bin/chattr", "+a", dir)
	output, err := cmd.CombinedOutput()
	if err != nil {
		out := strings.TrimSpace(string(output))
		if strings.Contains(out, "Operation not supported") {
			return nil // NFS filesystem: protection is handled server-side by the cron job.
		}
		if strings.Contains(out, "Operation not permitted") {
			return nil // Non-root on local filesystem: expected in dev/test environments.
		}
		return fmt.Errorf("failed to set chattr +a on %s: %s", dir, out)
	}
	return nil
}

// isInCelerGroup checks whether the current user belongs to the "celer" group.
func isInCelerGroup() (bool, error) {
	currentUser, err := user.Current()
	if err != nil {
		return false, err
	}
	groupIds, err := currentUser.GroupIds()
	if err != nil {
		return false, err
	}
	for _, gid := range groupIds {
		group, err := user.LookupGroupId(gid)
		if err != nil {
			continue
		}
		if group.Name == "celer" {
			return true, nil
		}
	}
	return false, nil
}
