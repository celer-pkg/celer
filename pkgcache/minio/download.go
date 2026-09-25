package minio

import (
	"fmt"
	"os"
	"path/filepath"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/celer-pkg/celer/pkgs/logger"
	"github.com/minio/minio-go/v7"
)

// DownloadConfig implements pkgcache.DownloadCache for storing/restoring
// downloaded files (tools, archives) in a shared cache.
type DownloadConfig struct {
	minioCache
	ctx      context.Context
	cacheDir string
	writable bool
}

func NewDownloadConfig(ctx context.Context, client *minio.Client) *DownloadConfig {
	pkgCache := ctx.PkgCache()
	if pkgCache == nil || pkgCache.GetMinio() == nil {
		return nil
	}

	minioConfig := pkgCache.GetMinio()
	return &DownloadConfig{
		ctx: ctx,
		minioCache: minioCache{
			client:     client,
			bucketName: bucketName,
		},
		cacheDir: minioConfig.GetDir(pkgcache.DirDownloads, ctx.Version()),
		writable: pkgCache.GetOptions().Writable,
	}
}

// Store saves a downloaded file to the cache dir: downloads/{fileName}.
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
	objectName := filepath.Join(d.cacheDir, fileName)

	// The cache is authoritative: never auto-overwrite an existing entry.
	if info, err := d.GetFileInfo(objectName); err != nil {
		return fmt.Errorf("failed to check cached file '%s' -> %w", objectName, err)
	} else if info != nil {
		remoteSha256 := d.metaSha256(info)
		if remoteSha256 == "" {
			return fmt.Errorf("'%s' is cached but has no sha256 metadata; remove it before re-storing", fileName)
		}
		if remoteSha256 == sha256 {
			return nil // already cached with the same content
		}

		return fmt.Errorf("'%s' is cached with sha256=%s, but storing sha256=%s; remove the cached object manually if you want to replace it", fileName, remoteSha256, sha256)
	}

	// Upload file with progress.
	if err := d.uploadFile(fileToStore, objectName, fileName); err != nil {
		return err
	}

	return nil
}

// Restore restores the cached file for fileName to the downloads dir.
func (d DownloadConfig) Restore(fileName, sha256 string) (bool, error) {
	// skip when offline.
	if d.ctx.Offline() {
		return false, nil
	}

	if sha256 == "" {
		return false, fmt.Errorf("no sha256 hash provided for %s/%s", d.cacheDir, fileName)
	}

	objectName := filepath.Join(d.cacheDir, fileName)

	remoteInfo, err := d.GetFileInfo(objectName)
	if err != nil {
		return false, err
	}

	// No cache found, then download from source.
	if remoteInfo == nil {
		return false, nil
	}

	// Remote sha256: read from minio's metadata.
	remoteSha256 := d.metaSha256(remoteInfo)

	// Download the cached file into a task-owned tmp dir with progress.
	localTmpDir, err := dirs.NewTmpFilesDir()
	if err != nil {
		return false, err
	}
	defer os.RemoveAll(localTmpDir)

	tmpDownloaded, err := d.downloadFile(localTmpDir, objectName, fileName)
	if err != nil {
		return false, err
	}

	// Verify transfer integrity against the hosted metadata sha256 (not the
	// requested one). A mismatch means the cached object is corrupted — treat
	// it as a miss and let the caller re-download from source.
	if remoteSha256 != "" {
		if got, err := fileio.SHA256Sum(tmpDownloaded); err != nil {
			return false, err
		} else if got != remoteSha256 {
			logger.PrintWarning("======== cached '%s' is corrupted (sha-256 mismatch), re-download from source ========", fileName)
			return false, nil
		}
	}

	if err := os.MkdirAll(d.ctx.Downloads(), os.ModePerm); err != nil {
		return false, fmt.Errorf("failed to mkdir for '%s' -> %w", d.ctx.Downloads(), err)
	}

	destPath := filepath.Join(d.ctx.Downloads(), fileName)
	if err := os.Rename(tmpDownloaded, destPath); err != nil {
		return false, err
	}

	return true, nil
}
