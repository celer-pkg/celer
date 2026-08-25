package netfs

import (
	"fmt"
	"io"
	"os"
	"path/filepath"

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
	color.PrintInline(color.Hint, "[-] validating with sha256: %s", sha256)

	// Verify file's sha256.
	computedHash, err := fileio.SHA256Sum(cachedFilePath)
	if err != nil {
		color.PrintInline(color.Hint, "[✘] validate with sha256: %s\n", sha256)
		return "", fmt.Errorf("failed to compute sha-256 for cached file -> %w", err)
	}
	if computedHash == sha256 {
		color.PrintInline(color.Hint, "[✔] validate with sha256: %s\n", sha256)
		return cachedFilePath, nil
	}

	color.PrintInline(color.Hint, "[✘] validate with sha256: %s\n", sha256)
	return "", nil
}

// Store saves a downloaded file to the cache directory using SHA256 in the filename.
func (d DownloadConfig) Store(fileName, sha256, srcPath string) (string, error) {
	if sha256 == "" {
		panic(fmt.Sprintf("no sha-256 provided when caching file to pkgcache for %s", fileName))
	}

	if err := os.MkdirAll(d.cacheDir, fileio.CacheDirPerm); err != nil {
		return "", fmt.Errorf("failed to create cache dir -> %w", err)
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
	if err := os.MkdirAll(nfsTmpDir, fileio.CacheDirPerm); err != nil {
		return "", fmt.Errorf("failed to create tmp dir -> %w", err)
	}

	// Create a local tmp file as the copy dest.
	destTmpFile, err := os.CreateTemp(nfsTmpDir, fmt.Sprintf("%s-*", fileName))
	if err != nil {
		return "", fmt.Errorf("failed to create nfs tmp file: %s -> %w", fileName, err)
	}
	defer func() {
		destTmpFile.Close()
		os.Remove(destTmpFile.Name())
	}()

	// Get file info to get file size.
	info, err := os.Stat(srcPath)
	if err != nil {
		return "", fmt.Errorf("failed to get file info for %s -> %w", srcPath, err)
	}

	// This is the file to copy from.
	srcFile, err := os.Open(srcPath)
	if err != nil {
		return "", fmt.Errorf("failed to open src file: %s -> %w", srcPath, err)
	}
	defer srcFile.Close()

	// Cache to pkgcache in progress.
	completed := func(formattedTimeCost, formattedSize string) {
		color.PrintInline(color.Pass, "[✔] %s (%s) in %s\n", fileName+" is stored", formattedSize, formattedTimeCost)
		color.PrintHint("Location: %s", d.cacheDir)
	}
	progress := fileio.NewProgressBar("caching to pkgcache: "+fileName, info.Size(), completed)
	if _, err := io.Copy(io.MultiWriter(destTmpFile, progress), srcFile); err != nil {
		return "", fmt.Errorf("failed to copy file to cache -> %w", err)
	}

	// Flush and close before rename.
	if err := destTmpFile.Sync(); err != nil {
		return "", fmt.Errorf("failed to sync file to cache -> %w", err)
	}
	if err := destTmpFile.Close(); err != nil {
		return "", fmt.Errorf("failed to close tmp file -> %w", err)
	}

	if err := os.Rename(destTmpFile.Name(), cachedFilePath); err != nil {
		// Dest may have appeared between our check and rename (another user won the race).
		if fileio.VerifyFileSHA256(cachedFilePath, sha256) {
			return cachedFilePath, nil
		}

		// Rename failed — just try again.
		if err := fileio.CopyFile(srcPath, cachedFilePath); err != nil {
			return "", fmt.Errorf("failed to copy file to cache -> %w", err)
		}
	}

	return cachedFilePath, nil
}
