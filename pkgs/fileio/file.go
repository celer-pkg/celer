package fileio

import (
	"crypto/sha256"
	"encoding/hex"
	"errors"
	"fmt"
	"io"
	"io/fs"
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"syscall"
	"time"

	"github.com/celer-pkg/celer/pkgs/dirs"
)

// IsExecutable check if file was executable
func IsExecutable(filepath string) bool {
	info, err := os.Stat(filepath)
	if err != nil {
		panic("file not found for " + filepath)
	}

	// 73: 000 001 001 001
	perm := info.Mode().Perm()
	flag := perm & os.FileMode(73)
	return uint32(flag) == uint32(73)
}

// IsReadable check if file or dir readable
func IsReadable(filepath string) bool {
	info, err := os.Stat(filepath)
	if err != nil {
		return false
	}

	return info.Mode().Perm()&(1<<(uint(8))) != 0
}

// IsWritable check if file or dir writable
func IsWritable(filepath string) bool {
	info, err := os.Stat(filepath)
	if err != nil {
		return false
	}

	return info.Mode().Perm()&(1<<(uint(7))) != 0
}

// PathExists checks if the path exists.
func PathExists(path string) bool {
	_, err := os.Stat(path)
	if err == nil {
		return true
	}

	return !os.IsNotExist(err)
}

// Base it's a improved version to get file base name.
func Base(fileName string) string {
	fileName = filepath.Base(fileName)
	index := strings.Index(fileName, ".tar.")
	if index > 0 {
		return fileName[:index]
	}

	ext := filepath.Ext(fileName)
	return strings.TrimSuffix(fileName, ext)
}

// Ext it's a improved version to get file extension, it will return .tar.gz for archive file.
func Ext(fileName string) string {
	index := strings.Index(fileName, ".tar.")
	if index > 0 {
		return fileName[index:]
	}
	return filepath.Ext(fileName)
}

// CopyDir copy files in src to dest.
func CopyDir(srcDir, dstDir string) error {
	return filepath.Walk(srcDir, func(srcPath string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}

		relPath, err := filepath.Rel(srcDir, srcPath)
		if err != nil {
			return err
		}
		dstPath := filepath.Join(dstDir, relPath)

		if info.IsDir() {
			return os.MkdirAll(dstPath, info.Mode())
		}

		return CopyFile(srcPath, dstPath)
	})
}

// FlattenNestedDir flattens a single wrapping directory into its parent.
// Many source archives extract into a single subdirectory like ffmpeg-4.4/;
// this moves the contents up into dir, removing the extra nesting level.
// Directories named "include" are left as-is to preserve system include layouts.
func FlattenNestedDir(dir string) error {
	dir = filepath.Clean(dir)
	entities, err := os.ReadDir(dir)
	if err != nil {
		return fmt.Errorf("failed to read dir -> %w", err)
	}

	if len(entities) != 1 || !entities[0].IsDir() || entities[0].Name() == "include" {
		return nil
	}

	// dir holds only the wrapper, so flattening is three renames: move the
	// wrapper aside, drop the now-empty dir, let the wrapper take its place.
	srcDir := filepath.Join(dir, entities[0].Name())
	tmpDir := dir + ".flatten"
	if err := os.Rename(srcDir, tmpDir); err != nil {
		return fmt.Errorf("failed to flatten nested dir -> %w", err)
	}
	if err := os.Remove(dir); err != nil {
		os.Rename(tmpDir, srcDir) // Best-effort rollback.
		return fmt.Errorf("failed to flatten nested dir -> %w", err)
	}
	if err := os.Rename(tmpDir, dir); err != nil {
		return fmt.Errorf("failed to flatten nested dir -> %w", err)
	}

	return nil
}

// CopyFile copy file from src to dest.
func CopyFile(src, dest string) error {
	// Read file info.
	info, err := os.Lstat(src)
	if err != nil {
		return err
	}

	// Create symlink if it's a symlink.
	if info.Mode()&os.ModeSymlink != 0 {
		target, err := os.Readlink(src)
		if err != nil {
			return err
		}

		// Remove dest if it exists before creating symlink.
		if _, err := os.Lstat(dest); err == nil {
			if err := os.Remove(dest); err != nil {
				return err
			}
		}

		return os.Symlink(target, dest)
	}

	return copyFileContent(src, dest, false)
}

// MoveFile moves src to dst. It creates the destination directory, handles
// symlinks, retries renames blocked by transient Windows file locks, and falls
// back to copy+delete when rename cannot be done (e.g. across file systems).
func MoveFile(src, dst string) error {
	info, err := os.Lstat(src)
	if err != nil {
		return fmt.Errorf("stat source -> %w", err)
	}

	// Symlinks are moved by recreating them at dst.
	if info.Mode()&os.ModeSymlink != 0 {
		return moveSymlink(src, dst)
	}

	if err := MkdirAll(filepath.Dir(dst), os.ModePerm); err != nil {
		return fmt.Errorf("failed to create directory -> %w", err)
	}

	// On Windows, files under heavy access in short time are often locked,
	// we need to retries with delays.
	if err := renameWithRetry(src, dst, 3); err == nil {
		return nil
	}

	// Rename failed (e.g. across file systems), fall back to copy+delete.
	if err := copyFileContent(src, dst, true); err != nil {
		return err
	}
	return os.Remove(src)
}

// moveSymlink re-creates the symlink at dst and removes the source.
func moveSymlink(src, dst string) error {
	target, err := os.Readlink(src)
	if err != nil {
		return fmt.Errorf("read symlink -> %w", err)
	}
	if err := MkdirAll(filepath.Dir(dst), os.ModePerm); err != nil {
		return fmt.Errorf("create directory for symlink -> %w", err)
	}

	// Remove dst if it exists, otherwise creating the symlink fails.
	if _, err := os.Lstat(dst); err != nil {
		if !os.IsNotExist(err) {
			return fmt.Errorf("stat dst -> %w", err)
		}
	} else if err := os.Remove(dst); err != nil {
		return err
	}

	if err := os.Symlink(target, dst); err != nil {
		return fmt.Errorf("create symlink -> %w", err)
	}
	return os.Remove(src)
}

func renameWithRetry(src, dst string, maxRetries int) error {
	var lastErr error
	for range maxRetries {
		err := os.Rename(src, dst)
		if err == nil {
			return nil
		}
		lastErr = err

		// Permanent errors fail fast, only transient ones are retried.
		if !isRetryableRenameErr(err) {
			return err
		}
		time.Sleep(100 * time.Millisecond)
	}
	return fmt.Errorf("failed after %d retries -> %w", maxRetries, lastErr)
}

func isRetryableRenameErr(err error) bool {
	// Missing paths or cross-device renames never recover on retry.
	if errors.Is(err, syscall.EXDEV) || errors.Is(err, fs.ErrNotExist) || errors.Is(err, fs.ErrInvalid) {
		return false
	}

	// Everything else (e.g. Windows file locks) may be transient.
	return true
}

// copyFileContent copies a regular file, preserving its mode; sync makes the
// dest durable before returning, callers that delete src afterwards want this.
func copyFileContent(src, dest string, sync bool) error {
	// Close src file after copy.
	srcFile, err := os.Open(src)
	if err != nil {
		return err
	}
	defer srcFile.Close()

	// Keep original file attributes.
	stat, err := srcFile.Stat()
	if err != nil {
		return err
	}

	// Remove dest if it exists to avoid "permission denied" for read-only files.
	if _, err := os.Lstat(dest); err == nil {
		if err := os.Remove(dest); err != nil {
			return err
		}
	}

	dstFile, err := os.OpenFile(dest, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, stat.Mode())
	if err != nil {
		return err
	}
	defer dstFile.Close()

	if _, err = io.Copy(dstFile, srcFile); err != nil {
		return err
	}

	// Chmod to the exact source mode, creation above is subject to umask.
	if err := os.Chmod(dest, stat.Mode()); err != nil {
		return err
	}

	if sync {
		return dstFile.Sync()
	}
	return nil
}

func moveNestedFolderIfExist(filePath string) error {
	// We assume the archive contains a single root folder, check if it has nested folder.
	if nestedFolder := findNestedFolder(filePath); nestedFolder != "" {
		// Move the entire nested folder to the parent directory
		if err := moveDirectoryToParent(nestedFolder, filepath.Dir(filePath)); err != nil {
			return err
		}
	}

	return nil
}

// findNestedFolder A nested folder only exists when the archive extracted a single wrapping directory.
func findNestedFolder(parentDir string) string {
	entries, err := os.ReadDir(parentDir)
	if err != nil {
		return ""
	}

	if len(entries) != 1 || !entries[0].IsDir() {
		return ""
	}

	folderName := filepath.Base(parentDir)
	if entries[0].Name() != folderName {
		return ""
	}

	nestedDir := filepath.Join(parentDir, entries[0].Name())
	if _, err := os.Stat(nestedDir); err == nil {
		return nestedDir
	}

	return ""
}

func moveDirectoryToParent(nestedFolder, parentFolder string) error {
	destPath := filepath.Join(parentFolder, filepath.Base(nestedFolder))
	tmpPath := filepath.Join(parentFolder, filepath.Base(nestedFolder)+".tmp")

	// Move folder that we want to a temporary path.
	if err := os.Rename(nestedFolder, tmpPath); err != nil {
		return fmt.Errorf("rename directory from %s to %s -> %w", nestedFolder, nestedFolder+".old", err)
	}

	// Remove the now empty nested folder.
	if err := os.RemoveAll(destPath); err != nil {
		return fmt.Errorf("remove empty nested folder %s -> %w", nestedFolder, err)
	}

	// Convert the temporary folder to the actual folder.
	if err := os.Rename(tmpPath, destPath); err != nil {
		return fmt.Errorf("move directory from %s to %s -> %w", nestedFolder, destPath, err)
	}

	return nil
}

func RemoveFolderRecursively(path string) error {
	// Not exists, skip.
	if !PathExists(path) {
		return nil
	}

	entities, err := os.ReadDir(path)
	if err != nil {
		return err
	}

	// Empty folder, remove it.
	if len(entities) == 0 {
		if err := os.RemoveAll(path); err != nil {
			return err
		}

		// Remove parent folder if it's empty.
		if err := RemoveFolderRecursively(filepath.Dir(path)); err != nil {
			return err
		}

		return nil
	}

	return nil
}

// ToCygpath convert windows path to cygpath.
func ToCygpath(path string) string {
	if runtime.GOOS == "windows" {
		path = filepath.Clean(path)
		path = filepath.ToSlash(path)

		// Handle disk driver（for example: `C:/` → `/c/`）
		if len(path) >= 2 && path[1] == ':' {
			drive := strings.ToLower(string(path[0]))
			path = "/" + drive + path[2:]
		}

		return path
	}

	return path
}

func CleanDir(dir string) error {
	if err := os.RemoveAll(dir); err != nil {
		return fmt.Errorf("cannot remove dir -> %w", err)
	}

	if err := os.MkdirAll(dir, os.ModePerm); err != nil {
		return fmt.Errorf("cannot mkdir dir -> %w", err)
	}

	return nil
}

// MkdirAll create directory with retry to handle Windows file system delays.
func MkdirAll(path string, perm os.FileMode) error {
	// Already exists.
	if PathExists(path) {
		return nil
	}

	// Initial attempt
	err := os.MkdirAll(path, perm)
	if err == nil {
		return nil
	}
	if os.IsExist(err) {
		if info, statErr := os.Stat(path); statErr == nil && info.IsDir() {
			return nil
		}
	}

	// Retry mkdir several times.
	for range 3 {
		time.Sleep(10 * time.Millisecond)
		err = os.MkdirAll(path, perm)
		if err == nil {
			return nil
		}
		if os.IsExist(err) {
			if info, statErr := os.Stat(path); statErr == nil && info.IsDir() {
				return nil
			}
		}
	}
	return err
}

// Convert try to convert absolute path to relative path based on current workspace.
func ToRelPath(absPath string) string {
	relativePath, err := filepath.Rel(dirs.WorkspaceDir, absPath)
	if err != nil {
		return filepath.ToSlash(absPath)
	}
	return "${WORKSPACE_ROOT}/" + filepath.ToSlash(relativePath)
}

func IsSubPath(parent, child string) bool {
	rel, err := filepath.Rel(parent, child)
	if err != nil {
		return false
	}

	return rel != "." && !strings.HasPrefix(rel, ".."+string(filepath.Separator)) && rel != ".."
}

// IsDirectory check if a path is directory or not.
func IsDirectory(path string) (bool, error) {
	fileInfo, err := os.Stat(path)
	if err != nil {
		return false, err
	}
	return fileInfo.IsDir(), nil
}

// ReplaceContent replaces or appends a line in a config file based on the provided condition.
func ReplaceContent(filePath, lineToAdd string, shouldRemove func(string) bool) error {
	content, err := os.ReadFile(filePath)
	if err != nil && !os.IsNotExist(err) {
		return err
	}

	// If file does not exist or is empty, just write the new line.
	text := strings.TrimRight(string(content), "\n")
	if text == "" {
		return os.WriteFile(filePath, []byte(lineToAdd+"\n"), 0644)
	}

	// Remove the old line if it exists to avoid duplicates,
	// and preserve other lines and the overall file structure as much as possible.
	filtered := make([]string, 0)
	for existingLine := range strings.SplitSeq(text, "\n") {
		if shouldRemove(existingLine) {
			continue
		}
		filtered = append(filtered, existingLine)
	}

	// Append the new line at the end.
	filtered = append(filtered, lineToAdd)
	return os.WriteFile(filePath, []byte(strings.Join(filtered, "\n")+"\n"), 0644)
}

// RemoveContent removes lines in a config file based on the provided condition.
func RemoveContent(filePath string, shouldRemove func(string) bool) error {
	content, err := os.ReadFile(filePath)
	if os.IsNotExist(err) {
		return nil
	}
	if err != nil {
		return err
	}

	text := strings.TrimRight(string(content), "\n")
	if text == "" {
		return nil
	}

	// Remove the line if it exists, and preserve other lines
	// and the overall file structure as much as possible.
	filtered := make([]string, 0)
	for existingLine := range strings.SplitSeq(text, "\n") {
		if shouldRemove(existingLine) {
			continue
		}
		filtered = append(filtered, existingLine)
	}

	if len(filtered) == 0 {
		return os.WriteFile(filePath, nil, 0644)
	}
	return os.WriteFile(filePath, []byte(strings.Join(filtered, "\n")+"\n"), 0644)
}

// isELF tell whether path is a regular file whose first 4 bytes are the
// ELF magic (\x7fELF). Cheaper and more accurate than checking extensions
// or the executable mode bit (which would mis-classify shell scripts).
func IsELFFile(path string) bool {
	f, err := os.Open(path)
	if err != nil {
		return false
	}
	defer f.Close()

	var magic [4]byte
	if _, err := f.Read(magic[:]); err != nil {
		return false
	}
	return magic[0] == 0x7f && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F'
}

// SHA256Sum computes the SHA256 hash of a file.
func SHA256Sum(filePath string) (string, error) {
	file, err := os.Open(filePath)
	if err != nil {
		return "", fmt.Errorf("failed to open file -> %w", err)
	}
	defer file.Close()

	hash := sha256.New()
	if _, err := io.Copy(hash, file); err != nil {
		return "", fmt.Errorf("failed to compute hash -> %w", err)
	}

	// Reset file seek.
	if _, err := file.Seek(0, io.SeekStart); err != nil {
		return "", err
	}

	return hex.EncodeToString(hash.Sum(nil)), nil
}

// verifySHA256 verifies if a file's SHA256 matches the expected hash.
func VerifyFileSHA256(filePath, expectedHash string) bool {
	if expectedHash == "" {
		panic("no sha256 provided for " + filePath)
	}

	computedHash, err := SHA256Sum(filePath)
	if err != nil {
		return false
	}

	return computedHash == expectedHash
}
