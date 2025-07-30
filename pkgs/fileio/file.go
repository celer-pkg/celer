package fileio

import (
	"crypto/sha256"
	"encoding/hex"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"time"
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

// FileBaseName it's a improved version to get file base name.
func FileBaseName(fileName string) string {
	fileName = filepath.Base(fileName)
	index := strings.Index(fileName, ".tar.")
	if index > 0 {
		return fileName[:index]
	}

	ext := filepath.Ext(fileName)
	return strings.TrimSuffix(fileName, ext)
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

// RenameDir rename files in src to dest.
func RenameDir(srcDir, dstDir string) error {
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

		if err := RenameFile(srcPath, dstPath); err != nil {
			return err
		}

		// Try remove parent folder if it's empty.
		if err := RemoveFolderRecursively(filepath.Dir(srcPath)); err != nil {
			return fmt.Errorf("cannot remove parent folder: %s", err)
		}

		return nil
	})
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

	// Copy normal file.
	srcFile, err := os.Open(src)
	if err != nil {
		return err
	}
	defer srcFile.Close()

	destFile, err := os.Create(dest)
	if err != nil {
		return err
	}
	defer destFile.Close()

	if _, err := io.Copy(destFile, srcFile); err != nil {
		return err
	}

	// Set the same permissions as the source file.
	if err := os.Chmod(dest, info.Mode()); err != nil {
		return err
	}

	return nil
}

func RenameFile(src, dst string) error {
	info, err := os.Lstat(src)
	if err != nil {
		return fmt.Errorf("stat source: %w", err)
	}

	if info.Mode()&os.ModeSymlink != 0 {
		return handleSymlink(src, dst)
	}

	// On Windows, files under heavy access in short time are often locked,
	// we need to retries with delays.
	return renameWithRetry(src, dst, 3) // Retry 3 times.
}

func handleSymlink(src, dst string) error {
	target, err := os.Readlink(src)
	if err != nil {
		return fmt.Errorf("read symlink: %w", err)
	}
	if err := os.MkdirAll(filepath.Dir(dst), os.ModePerm); err != nil {
		return fmt.Errorf("create directory for symlink: %w", err)
	}
	return os.Symlink(target, dst)
}

func renameWithRetry(src, dst string, maxRetries int) error {
	if err := os.MkdirAll(filepath.Dir(dst), os.ModePerm); err != nil {
		return fmt.Errorf("create directory: %w", err)
	}

	var lastErr error
	for range maxRetries {
		if err := os.Rename(src, dst); err == nil {
			return nil
		} else {
			lastErr = err
			time.Sleep(100 * time.Millisecond)
		}
	}
	return fmt.Errorf("failed after %d retries: %v", maxRetries, lastErr)
}

func MoveFile(src, dst string) error {
	srcFile, err := os.Open(src)
	if err != nil {
		return fmt.Errorf("cannot open file: %v", err)
	}
	defer srcFile.Close()

	dstFile, err := os.Create(dst)
	if err != nil {
		return fmt.Errorf("cannot create dest file: %v", err)
	}
	defer dstFile.Close()

	if _, err = io.Copy(dstFile, srcFile); err != nil {
		return fmt.Errorf("copy file: %w", err)
	}

	if err := dstFile.Sync(); err != nil {
		return fmt.Errorf("sync file: %w", err)
	}

	if err := os.Remove(src); err != nil {
		return fmt.Errorf("remove src file: %w", err)
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

func findNestedFolder(parentDir string) string {
	entries, err := os.ReadDir(parentDir)
	if err != nil {
		return ""
	}

	folderName := filepath.Base(parentDir)

	for _, entry := range entries {
		// If a folder is found that isn't the one we are currently in,
		// it's considered a nested folder.
		if entry.IsDir() && folderName == entry.Name() {
			nestedDir := filepath.Join(parentDir, entry.Name())
			if _, err := os.Stat(nestedDir); err == nil {
				return nestedDir
			}
		}
	}

	return ""
}

func moveDirectoryToParent(nestedFolder, parentFolder string) error {
	destPath := filepath.Join(parentFolder, filepath.Base(nestedFolder))
	tmpPath := filepath.Join(parentFolder, filepath.Base(nestedFolder)+".tmp")

	// Move folder that we want to a temporary path.
	if err := os.Rename(nestedFolder, tmpPath); err != nil {
		return fmt.Errorf("rename directory from %s to %s: %w", nestedFolder, nestedFolder+".old", err)
	}

	// Remove the now empty nested folder.
	if err := os.RemoveAll(destPath); err != nil {
		return fmt.Errorf("remove empty nested folder %s: %w", nestedFolder, err)
	}

	// Convert the temporary folder to the actual folder.
	if err := os.Rename(tmpPath, destPath); err != nil {
		return fmt.Errorf("move directory from %s to %s: %w", nestedFolder, destPath, err)
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

func CalculateChecksum(filePath string) (string, error) {
	file, err := os.Open(filePath)
	if err != nil {
		return "", err
	}
	defer file.Close()

	hash := sha256.New()
	if _, err := io.Copy(hash, file); err != nil {
		return "", err
	}
	checksum := hex.EncodeToString(hash.Sum(nil))
	return checksum, nil
}
