package filesystem

import (
	"fmt"
	"os"
	"path/filepath"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/errors"
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

// Store saves a downloaded file to the cache dir: downloads/{fileName}.
func (d DownloadConfig) Store(fileName, sha256, srcPath string) error {
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

	remoteFilePath := filepath.Join(d.cacheDir, fileName)

	// The cache is authoritative: never auto-overwrite an existing entry.
	if fileio.PathExists(remoteFilePath) {
		remoteSha256, err := fileio.SHA256Sum(remoteFilePath)
		if err != nil {
			return fmt.Errorf("failed to calculate sha256 of '%s' -> %w", fileName, err)
		}
		if remoteSha256 == sha256 {
			return nil // already cached with the same content
		}
		return fmt.Errorf("%w: '%s' is cached with sha256=%s, but storing sha256=%s; please remove the cached file manually if you want to replace it",
			errors.ErrSha256Mismatch, fileName, remoteSha256, sha256)
	}

	if err := d.uploadFile(srcPath, remoteFilePath, sha256, fileName); err != nil {
		return fmt.Errorf("failed to upload file '%s' to pkgcache -> %w", fileName, err)
	}

	return nil
}

// Restore restores the cached file for fileName to the downloads dir.
func (d DownloadConfig) Restore(fileName, sha256 string) (bool, error) {
	// Skip for offline.
	if d.ctx.Offline() {
		return false, nil
	}

	if sha256 == "" {
		return false, fmt.Errorf("no sha-256 hash provided for '%s'", fileName)
	}

	remoteFilePath := filepath.Join(d.cacheDir, fileName)
	if !fileio.PathExists(remoteFilePath) {
		return false, nil
	}

	// Download the cached file into a task-owned tmp dir with progress.
	localTmpDir, err := dirs.NewTmpFilesDir()
	if err != nil {
		return false, err
	}
	defer os.RemoveAll(localTmpDir)

	tmpDownloaded, err := d.downloadFile(localTmpDir, remoteFilePath, fileName)
	if err != nil {
		return false, err
	}

	// Copy the cached file into the downloads dir; callers deploy from there.
	if err := os.MkdirAll(d.ctx.Downloads(), os.ModePerm); err != nil {
		return false, fmt.Errorf("failed to mkdir for '%s' -> %w", d.ctx.Downloads(), err)
	}
	destPath := filepath.Join(d.ctx.Downloads(), fileName)
	if err := os.Rename(tmpDownloaded, destPath); err != nil {
		return false, err
	}

	return true, nil
}
