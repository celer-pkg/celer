package netfs

import (
	"fmt"
	"os"
	"path/filepath"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

type DownloadConfig struct {
	netfsCache
	ctx      context.Context
	cacheDir string // ${pkgcache_root}/downloads
	writable bool
}

func (d DownloadConfig) Store(fileName, sha256, srcPath string) error {
	// skip when offline.
	if d.ctx.Offline() {
		return nil
	}

	if sha256 == "" {
		return fmt.Errorf("sha256 of '%s' is not provided when store to netfs", fileName)
	}

	if err := os.MkdirAll(d.cacheDir, os.ModePerm); err != nil {
		return fmt.Errorf("failed to create cache dir -> %w", err)
	}

	// Filename format: {name}-{sha256}.{ext}
	remoteFileName := fmt.Sprintf("%s-%s%s", fileio.Base(fileName), sha256, fileio.Ext(fileName))
	remoteFilePath := filepath.Join(d.cacheDir, remoteFileName)

	if err := d.UploadFile(srcPath, remoteFilePath, sha256); err != nil {
		return fmt.Errorf("failed to upload file '%s' to netfs -> %w", fileName, err)
	}

	return nil
}

// Restore copies a cached download back into the downloads dir.
func (d DownloadConfig) Restore(fileName, sha256 string) (bool, error) {
	// skip when offline.
	if d.ctx.Offline() {
		return false, nil
	}

	if sha256 == "" {
		return false, fmt.Errorf("sha256 of '%s' is not provided when restore from netfs", fileName)
	}

	// Filename format: {name}-{sha256}.{ext}
	remoteFileName := fmt.Sprintf("%s-%s%s", fileio.Base(fileName), sha256, fileio.Ext(fileName))
	remoteFilePath := filepath.Join(d.cacheDir, remoteFileName)

	// Check if the file exists.
	if !fileio.PathExists(remoteFilePath) {
		return false, nil
	}

	// Do copy file from remote net fs.
	destFilePath := filepath.Join(d.ctx.Downloads(), fileName)
	if err := d.DownloadFile(remoteFilePath, destFilePath, sha256); err != nil {
		return false, err
	}

	return true, nil
}
