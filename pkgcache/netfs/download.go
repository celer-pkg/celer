package netfs

import (
	"fmt"
	"os"
	"path/filepath"
	"time"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

// DownloadConfig implements pkgcache.DownloadCache for storing/restoring
// downloaded files (tools, archives) in a shared NFS cache.
type DownloadConfig struct {
	ctx      context.Context
	cacheDir string
	writable bool
}

// NewDownloadConfig creates a DownloadConfig backed by the cache dir from ctx.
func NewDownloadConfig(ctx context.Context) *DownloadConfig {
	pkgCacheConfig := ctx.PkgCacheConfig()
	if pkgCacheConfig == nil || pkgCacheConfig.GetDir(pkgcache.PkgCacheDirArtifacts) == "" {
		return nil
	}

	cacheDir := pkgCacheConfig.GetDir(pkgcache.PkgCacheDirDownloads)
	return &DownloadConfig{
		cacheDir: cacheDir,
		writable: pkgCacheConfig.IsWritable(),
	}
}

// Restore finds a cached file matching the given SHA256 and returns its path.
// Returns empty string if not found.
func (d DownloadConfig) Restore(fileName, sha256 string) (string, error) {
	if sha256 == "" {
		panic(fmt.Sprintf("no sha256 hash provided for %s/%s", d.cacheDir, fileName))
	}

	if !fileio.PathExists(d.cacheDir) {
		return "", nil
	}

	// Build cached filename directly: {name}-{sha256}.{ext}
	cachedFileName := fmt.Sprintf("%s-%s%s", fileio.Base(fileName), sha256, fileio.Ext(fileName))
	cachedFilePath := filepath.Join(d.cacheDir, cachedFileName)

	// Check if the file exists.
	if !fileio.PathExists(cachedFilePath) {
		return "", nil
	}

	color.Printf(color.Title, "\n%s\n", fmt.Sprintf("[validating file cache: %s]", fileName))
	color.PrintInline(color.Hint, "- validating with sha256: %s", sha256)

	// Verify file's sha256.
	computedHash, err := fileio.SHA256Sum(cachedFilePath)
	if err != nil {
		color.PrintInline(color.Hint, "✘ validate with sha256: %s\n", sha256)
		return "", fmt.Errorf("failed to compute sha-256 for cached file -> %w", err)
	}
	if computedHash == sha256 {
		color.PrintInline(color.Hint, "✔ validate with sha256: %s\n", sha256)
		return cachedFilePath, nil
	}

	color.PrintInline(color.Hint, "✘ validate with sha256: %s\n", sha256)
	return "", nil
}

// Store saves a downloaded file to the cache directory using SHA256 in the filename.
func (d DownloadConfig) Store(fileName, sha256, srcFile string) (string, error) {
	if sha256 == "" {
		panic(fmt.Sprintf("no sha-256 provided when caching file to pkgcache for %s", fileName))
	}

	if !fileio.PathExists(d.cacheDir) {
		if err := os.MkdirAll(d.cacheDir, fileio.CacheDirPerm); err != nil {
			return "", fmt.Errorf("failed to create cache dir -> %w", err)
		}
	}

	cachedFileName := fmt.Sprintf("%s-%s%s", fileio.Base(fileName), sha256, fileio.Ext(fileName))
	cachedFilePath := filepath.Join(d.cacheDir, cachedFileName)

	// If cache file exists and SHA256 matches, return it directly.
	if fileio.PathExists(cachedFilePath) {
		if fileio.VerifyFileSHA256(cachedFilePath, sha256) {
			return cachedFilePath, nil
		}
	}

	// Write to NFS tmp dir first (excluded from chattr +a),
	// then atomically rename into final location to avoid partial reads.
	nfsTmpDir := filepath.Join(filepath.Dir(d.cacheDir), "tmp")
	nfsTmpFileName := fmt.Sprintf("%s-%d%s", fileio.Base(fileName), time.Now().UnixNano(), fileio.Ext(fileName))
	nfsTmpPath := filepath.Join(nfsTmpDir, nfsTmpFileName)
	if err := os.MkdirAll(nfsTmpDir, fileio.CacheDirPerm); err != nil {
		return "", fmt.Errorf("failed to create tmp dir -> %w", err)
	}
	if err := fileio.CopyFile(srcFile, nfsTmpPath); err != nil {
		return "", fmt.Errorf("failed to copy file to cache -> %w", err)
	}
	defer os.Remove(nfsTmpPath)

	if err := os.Rename(nfsTmpPath, cachedFilePath); err != nil {
		// Dest may have appeared between our check and rename (another user won the race).
		if fileio.VerifyFileSHA256(cachedFilePath, sha256) {
			return cachedFilePath, nil
		}

		// Rename failed — likely chattr +a dir with existing corrupt dest.
		// Fall back to in-place overwrite (O_TRUNC).
		if err := fileio.CopyFile(srcFile, cachedFilePath); err != nil {
			return "", fmt.Errorf("failed to copy file to cache -> %w", err)
		}
	}

	return cachedFilePath, nil
}
