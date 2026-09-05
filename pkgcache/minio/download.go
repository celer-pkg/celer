package minio

import (
	"fmt"
	"os"
	"path/filepath"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

// DownloadConfig implements pkgcache.DownloadCache for storing/restoring
// downloaded files (tools, archives) in a shared cache.
type DownloadConfig struct {
	minioCache
	ctx      context.Context
	cacheDir string
	writable bool
}

// Store saves a downloaded file to the cache directory using SHA256 in the filename.
func (d DownloadConfig) Store(fileName, sha256, srcPath string) error {
	// skip when offline.
	if d.ctx.Offline() {
		return nil
	}

	if sha256 == "" {
		panic(fmt.Sprintf("no sha-256 provided when caching file to pkgcache for %s", fileName))
	}

	// Create default bucket if not exist.
	if err := d.CreateBucketIfNotExist(); err != nil {
		return err
	}

	fileToStore := filepath.Join(d.ctx.Downloads(), fileName)
	cachedFileName := fmt.Sprintf("%s-%s%s", fileio.Base(fileName), sha256, fileio.Ext(fileName))
	cachedFilePath := filepath.Join(d.cacheDir, cachedFileName)

	// Check if already cached.
	remoteInfo, err := d.GetFileInfo(cachedFilePath)
	if err != nil {
		return fmt.Errorf("failed to check if exist for '%s' -> %w", cachedFilePath, err)
	}
	if remoteInfo != nil {
		localSha256, err := fileio.SHA256Sum(fileToStore)
		if err != nil {
			return fmt.Errorf("faild to calculate sha256sum for '%s' -> %w", fileToStore, err)
		}

		if d.metaSha256(remoteInfo) == localSha256 {
			return nil
		}
	}

	// Upload file with progress.
	if _, err := d.UploadFile(fileToStore, cachedFilePath, func(percent int) {
		if percent < 100 {
			color.PrintInline(color.Hint, "[-] %s is uploading %d%%", fileName, percent)
		} else if percent == 100 {
			color.PrintInline(color.Pass, "[✔] %s is stored to pkgcache.\n", fileName)
		}
	}); err != nil {
		return err
	}

	return nil
}

// Restore finds a cached file matching the given SHA256 and restores it to
// the downloads dir.
func (d DownloadConfig) Restore(fileName, sha256 string) (bool, error) {
	// skip when offline.
	if d.ctx.Offline() {
		return false, nil
	}

	if sha256 == "" {
		return false, fmt.Errorf("no sha256 hash provided for %s/%s", d.cacheDir, fileName)
	}

	// Build cached filename: {name}-{sha256}.{ext}
	remoteFileName := fmt.Sprintf("%s-%s%s", fileio.Base(fileName), sha256, fileio.Ext(fileName))
	remoteFilePath := filepath.Join(d.cacheDir, remoteFileName)

	// Get file info and check if checksum matches.
	remoteInfo, err := d.GetFileInfo(remoteFilePath)
	if err != nil {
		return false, err
	}

	// No cache found, then download from source.
	if remoteInfo == nil {
		return false, nil
	}

	// Cached sha256sum doesn't match -> download from source.
	if remoteSHA256 := d.metaSha256(remoteInfo); remoteSHA256 != "" && remoteSHA256 != sha256 {
		return false, nil
	}

	downloaded, err := d.DownloadFile(remoteFilePath)
	if err != nil {
		return false, err
	}
	defer func() {
		// Remove it if verify failed.
		if fileio.PathExists(downloaded) {
			os.Remove(downloaded)
		}
	}()

	// Verify the downloaded content with sha265.
	if localSha256, err := fileio.SHA256Sum(downloaded); err != nil {
		return false, err
	} else if localSha256 != sha256 {
		return false, nil
	}

	if err := os.MkdirAll(d.ctx.Downloads(), os.ModePerm); err != nil {
		return false, fmt.Errorf("failed to mkdir for '%s' -> %w", d.ctx.Downloads(), err)
	}

	destPath := filepath.Join(d.ctx.Downloads(), fileName)
	if err := os.Rename(downloaded, destPath); err != nil {
		return false, err
	}

	return true, nil
}
