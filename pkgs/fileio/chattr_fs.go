package fileio

import (
	"io"
	"os"
	"path/filepath"
)

// ChattrFS encapsulates chattr +a compatible file operations bound to a specific cacheRootDir.
type ChattrFS struct {
	rootDir string
}

func NewChattrFS(rootDir string) *ChattrFS {
	return &ChattrFS{rootDir: rootDir}
}

// MkdirAll creates a directory tree within append-only compatible workflows.
func (fs *ChattrFS) MkdirAll(path string, perm os.FileMode) error {
	return os.MkdirAll(path, perm)
}

// CopyFile it does not remove the destination. it opens with O_WRONLY|O_CREATE|O_TRUNC.
// This is compatible with chattr +a directories where deletion is blocked.
func (fs *ChattrFS) CopyFile(src, dest string) error {
	srcFile, err := os.Open(src)
	if err != nil {
		return err
	}
	defer srcFile.Close()

	if err := fs.MkdirAll(filepath.Dir(dest), CacheDirPerm); err != nil {
		return err
	}

	// Open with O_WRONLY|O_CREATE|O_TRUNC — creates new or truncates existing.
	destFile, err := os.OpenFile(dest, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, CacheFilePerm)
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
	if err := fs.MkdirAll(filepath.Dir(path), CacheDirPerm); err != nil {
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
