package filesystem

import (
	"fmt"
	"os"
	"path/filepath"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

// DownloadConfig implements pkgcache.DownloadCache for storing/restoring
// downloaded files (tools, archives) in a shared FS cache.
type DownloadConfig struct {
	fsCache
	ctx      context.Context
	cacheDir string
	writable bool
}

func NewDownloadConfig(ctx context.Context) *DownloadConfig {
	pkgCache := ctx.PkgCache()
	if pkgCache == nil || pkgCache.GetFS() == nil || pkgCache.GetFS().GetDir(pkgcache.DirRoot, ctx.Version()) == "" {
		return nil
	}

	filesystem := pkgCache.GetFS()
	cacheRootDir := filesystem.GetDir(pkgcache.DirRoot, ctx.Version())
	return &DownloadConfig{
		fsCache: fsCache{
			cacheDirRoot: cacheRootDir,
		},
		ctx:      ctx,
		cacheDir: filesystem.GetDir(pkgcache.DirDownloads, ctx.Version()),
		writable: pkgCache.GetOptions().Writable,
	}
}

// Store saves a downloaded file to the cache directory using SHA256 in the filename.
func (d DownloadConfig) Store(kind pkgcache.Kind, fileName, sha256, srcPath string) error {
	// skip when offline.
	if d.ctx.Offline() {
		return nil
	}

	if sha256 == "" {
		return fmt.Errorf("no sha-256 provided when caching file to pkgcache for %s", fileName)
	}

	if err := os.MkdirAll(d.cacheDir, os.ModePerm); err != nil {
		return fmt.Errorf("failed to create cache dir -> %w", err)
	}

	// Filename format: {name}-{sha256}.{ext}
	remoteFileName := fmt.Sprintf("%s-%s%s", fileio.Base(fileName), sha256, fileio.Ext(fileName))
	remoteFilePath := filepath.Join(d.cacheDir, remoteFileName)

	if err := d.uploadFile(kind, srcPath, remoteFilePath, sha256, fileName); err != nil {
		return fmt.Errorf("failed to upload file '%s' to pkgcache -> %w", fileName, err)
	}

	return nil
}

func (d DownloadConfig) Restore(kind pkgcache.Kind, fileName, sha256 string) (bool, error) {
	// Skip for offline.
	if d.ctx.Offline() {
		return false, nil
	}

	if sha256 == "" {
		return false, fmt.Errorf("no sha256 hash provided for %s/%s", d.cacheDir, fileName)
	}

	// Filename format: {name}-{sha256}.{ext}
	remoteFileName := fmt.Sprintf("%s-%s%s", fileio.Base(fileName), sha256, fileio.Ext(fileName))
	remoteFilePath := filepath.Join(d.cacheDir, remoteFileName)
	if !fileio.PathExists(remoteFilePath) {
		return false, nil
	}

	// Download the cached file to a tmp file with progress.
	downloaded, err := d.downloadFile(kind, remoteFilePath, fileName)
	if err != nil {
		return false, err
	}
	defer func() {
		// Remove it if verify failed.
		if fileio.PathExists(downloaded) {
			os.Remove(downloaded)
		}
	}()

	// Verify the downloaded content with sha256.
	if localSha256, err := fileio.SHA256Sum(downloaded); err != nil {
		return false, err
	} else if localSha256 != sha256 {
		return false, nil
	}

	// Copy the cached file into the downloads dir; callers deploy from there.
	if err := os.MkdirAll(d.ctx.Downloads(), os.ModePerm); err != nil {
		return false, fmt.Errorf("failed to mkdir for '%s' -> %w", d.ctx.Downloads(), err)
	}
	destPath := filepath.Join(d.ctx.Downloads(), fileName)
	if err := os.Rename(downloaded, destPath); err != nil {
		return false, err
	}

	return true, nil
}
