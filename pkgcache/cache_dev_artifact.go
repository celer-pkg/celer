package pkgcache

import (
	"crypto/sha256"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

type DevArtifactCache struct {
	cacheDir string
	ctx      context.Context
}

func NewDevArtifactCache(ctx context.Context, cacheDir string) *DevArtifactCache {
	return &DevArtifactCache{
		ctx:      ctx,
		cacheDir: cacheDir,
	}
}

// Restore restores the cached package to package directory if cache hit, and return the archive path.
// If cache miss, just return empty string without error.
func (d DevArtifactCache) Restore(nameVersion, buildHash, packageDir string) (string, error) {
	cachePath := filepath.Join(d.cacheDir, nameVersion, buildHash+".tar.gz")
	if !fileio.PathExists(cachePath) {
		return "", nil // not an error even not exist.
	}

	// The meta file hash should be the same as hash that calcuated dynamically.
	metaPath := filepath.Join(d.cacheDir, nameVersion, "metas", buildHash+".meta")
	if !fileio.PathExists(metaPath) {
		return "", nil
	}
	metaBytes, err := os.ReadFile(metaPath)
	if err != nil {
		return "", err
	}
	metaHash := sha256.Sum256(metaBytes)
	if fmt.Sprintf("%x", metaHash) != buildHash {
		return "", nil
	}

	// Create tmp dir for extracting inside.
	if err := dirs.CleanTmpFilesDir(); err != nil {
		return "", fmt.Errorf("failed to clean tmp files dir -> %w", err)
	}
	tempDir, err := os.MkdirTemp(dirs.TmpFilesDir, "devcache-extract-*")
	if err != nil {
		return "", err
	}
	defer os.RemoveAll(tempDir)

	// Extract to a tmp dir and move back to dest dir.
	if err := fileio.Extract(cachePath, tempDir); err != nil {
		return "", err
	}
	if err := os.RemoveAll(packageDir); err != nil {
		return "", err
	}
	if err := os.MkdirAll(filepath.Dir(packageDir), os.ModePerm); err != nil {
		return "", err
	}
	if err := os.Rename(tempDir, packageDir); err != nil {
		return "", err
	}

	return cachePath, nil
}

// Store compresses the package dir and store in cache,
// the meta is expected to be a string and would be used to calculate the hash key for cache.
func (d DevArtifactCache) Store(packageDir, meta string) error {
	if !fileio.PathExists(packageDir) {
		return fmt.Errorf("package dir does not exist: %s", packageDir)
	}

	// Validate packageDir format and extract metadata.
	// Path format: ~/celer/x86_64-linux-dev/nameVersion
	parts := strings.Split(filepath.ToSlash(packageDir), "/")
	if len(parts) < 1 {
		return fmt.Errorf("invalid package dir: %s", packageDir)
	}

	// Extract from path components.
	nameVersion := parts[len(parts)-1]

	// Validate nameVersion format (should be name@version)
	versionParts := strings.Split(nameVersion, "@")
	if len(versionParts) != 2 {
		return fmt.Errorf("invalid package dir: %s", packageDir)
	}

	// Extract tar.gz to a tmp dir.
	archiveName := fmt.Sprintf("%s.tar.gz", nameVersion)
	if err := dirs.CleanTmpFilesDir(); err != nil {
		return fmt.Errorf("failed to clean tmp files dir -> %w", err)
	}
	tempArchive, err := os.CreateTemp(dirs.TmpFilesDir, archiveName+".*")
	if err != nil {
		return err
	}
	tempArchivePath := tempArchive.Name()
	tempArchive.Close()
	defer os.Remove(tempArchivePath)

	if err := fileio.Targz(tempArchivePath, packageDir, false); err != nil {
		return err
	}

	destDir := filepath.Join(d.cacheDir, nameVersion)
	metaDir := filepath.Join(destDir, "metas")

	// Calculate checksum of metadata (this would be the cache key).
	data := sha256.Sum256([]byte(meta))
	hash := fmt.Sprintf("%x", data)

	// Create dirs.
	if err := os.MkdirAll(destDir, os.ModePerm); err != nil {
		return err
	}
	if err := os.MkdirAll(metaDir, os.ModePerm); err != nil {
		return err
	}

	// Copy archive directly to final path.
	archivePath := filepath.Join(destDir, hash+".tar.gz")
	if err := fileio.CopyFile(tempArchivePath, archivePath); err != nil {
		return err
	}

	// Write meta file directly to final path.
	metaPath := filepath.Join(metaDir, hash+".meta")
	if err := os.WriteFile(metaPath, []byte(meta), os.ModePerm); err != nil {
		return err
	}

	return nil
}
