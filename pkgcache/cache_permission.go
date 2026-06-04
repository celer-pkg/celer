package pkgcache

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

// NFSPermission implements PermissionStrategy for NFS shared storage.
type NFSPermission struct{}

func (n *NFSPermission) GetName() string {
	return "nfs"
}

// SetPermissions sets permissions to 0777 for directories and 0666 for files.
func (n *NFSPermission) SetPermissions(path string) error {
	// Check if path is a directory.
	isDir, err := fileio.IsDirectory(path)
	if err != nil {
		return err
	}

	var perm os.FileMode
	if isDir {
		perm = os.ModePerm // 0777 for directories
	} else {
		perm = 0o666 // 0666 for files
	}

	if err := os.Chmod(path, perm); err != nil {
		return fmt.Errorf("chmod %s to %o: %w", path, perm, err)
	}

	return nil
}

// MkdirAll creates a directory path (like os.MkdirAll),
func (n *NFSPermission) MkdirAll(dir, baseDir string) error {
	if err := os.MkdirAll(dir, os.ModePerm); err != nil {
		return err
	}

	// Walk from dir up to (but not including) baseDir, setting permissions on each.
	currentDir := dir
	for currentDir != baseDir && strings.HasPrefix(currentDir, baseDir) {
		if err := n.SetPermissions(currentDir); err != nil {
			return err
		}
		parent := filepath.Dir(currentDir)
		if parent == currentDir {
			break // reached root
		}
		currentDir = parent
	}

	return nil
}

// FTPPermission implements Permission for FTP shared storage.
// It sets permissions using FTP SITE CHMOD command when available.
// Note: FTP does not support ownership changes, only permissions.
type FTPPermission struct{}

func (f *FTPPermission) GetName() string {
	return "ftp"
}

// SetPermissions FTP permission setting would be implemented at the FTP storage layer,
// not at the file system level. This is a placeholder for the permission strategy.
// The actual FTP SITE CHMOD command would be sent to the FTP server
// during file upload/transfer operations.
func (f *FTPPermission) SetPermissions(path string) error {
	// Check if path is a dirctory.
	isDir, err := fileio.IsDirectory(path)
	if err != nil {
		return err
	}

	// For now, we attempt local chmod as a fallback for local operations
	var perm os.FileMode
	if isDir {
		perm = os.ModePerm // 0777 for directories
	} else {
		perm = 0o666 // 0666 for files
	}

	if err := os.Chmod(path, perm); err != nil {
		// Don't fail on chmod error for FTP since permissions may be managed remotely
		// Just log and continue
		return nil
	}

	return nil
}

// MkdirAll creates a directory path (like os.MkdirAll),
// then sets permissions on every newly created directory from dir to baseDir.
// baseDir is the topmost directory that does NOT need permissions set.
func (f *FTPPermission) MkdirAll(dir, baseDir string) error {
	if err := os.MkdirAll(dir, os.ModePerm); err != nil {
		return err
	}

	// Walk from dir up to (but not including) baseDir, setting permissions on each.
	currentDir := dir
	for currentDir != baseDir && strings.HasPrefix(currentDir, baseDir) {
		if err := f.SetPermissions(currentDir); err != nil {
			return err
		}

		parent := filepath.Dir(currentDir)
		if parent == currentDir {
			break // reached root
		}
		currentDir = parent
	}

	return nil
}

// NewPermission creates a new Permission based on the storage type.
func NewPermission(storageType string) context.Permission {
	switch storageType {
	case "nfs":
		return &NFSPermission{}
	case "ftp":
		return &FTPPermission{}
	// case "s3":
	//	return &S3Permission{}
	default:
		panic("unsupported storage type: " + storageType)
	}
}
