package netfs

import (
	"crypto/sha256"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

type ArtifactConfig struct {
	netfsCache
	ctx        context.Context
	cacheDir   string // ${pkgcache_root}/artifacts
	writable   bool
	maxRetries int
}

// Restore restores the cached package to package directory if cache hit.
// Returns false when there is no valid cached artifact, it's then the
// caller's job to build from source.
func (a ArtifactConfig) Restore(packageDir, nameVersion, buildHash string) (bool, error) {
	// skip when offline.
	if a.ctx.Offline() {
		return false, nil
	}

	platformName := a.ctx.Platform().GetName()
	projectName := a.ctx.Project().GetName()
	buildType := a.ctx.BuildType()

	remoteFileDir := filepath.Join(a.cacheDir, platformName, projectName, buildType, nameVersion)
	remoteFilePath := filepath.Join(remoteFileDir, buildHash+".tar.gz")
	if !fileio.PathExists(remoteFilePath) {
		color.PrintWarning("======== no artifact found for %s and it'll build from source ========", nameVersion)
		return false, nil // not an error even not exist.
	}

	// The meta file hash should be the same as hash that calcuated dynamically.
	remoteMetaPath := filepath.Join(remoteFileDir, "metas", buildHash+".meta")
	metaBytes, err := os.ReadFile(remoteMetaPath)
	if err != nil {
		if errors.Is(err, os.ErrNotExist) {
			color.PrintWarning("======== cached artifact for %s has no metadata, it'll build from source ========", nameVersion)
			return false, nil
		}
		return false, err
	}
	metaHash := sha256.Sum256(metaBytes)
	if fmt.Sprintf("%x", metaHash) != buildHash {
		return false, fmt.Errorf("cache metadata checksum mismatch for %s", nameVersion)
	}

	// Copy remote archive to a local tmp file with progress.
	destFile, err := os.CreateTemp(os.TempDir(), "celer-pkgcache-artifact-*.tar.gz")
	if err != nil {
		return false, fmt.Errorf("can not create tmp artifact file -> %w", err)
	}
	destFile.Close()
	defer os.Remove(destFile.Name())

	if err := a.copyWithProgress(remoteFilePath, destFile.Name(),
		"restore artifact", nameVersion+"'s artifact is restored"); err != nil {
		return false, fmt.Errorf("failed to restore %s from pkgcache -> %w", nameVersion, err)
	}

	// Create tmp dir and extract inside.
	if err := dirs.CleanTmpFilesDir(); err != nil {
		return false, fmt.Errorf("failed to clean tmp files dir -> %w", err)
	}
	tempDir, err := os.MkdirTemp(dirs.TmpFilesDir, "celer-pkgcache-pkgcache-extract-*")
	if err != nil {
		return false, err
	}
	defer os.RemoveAll(tempDir)
	if err := fileio.Extract(destFile.Name(), tempDir); err != nil {
		return false, fmt.Errorf("failed to extract '%s' to '%s'-> %w", destFile.Name(), tempDir, err)
	}

	// Clean package dir and move to it.
	if err := os.RemoveAll(packageDir); err != nil {
		return false, err
	}
	if err := os.MkdirAll(filepath.Dir(packageDir), os.ModePerm); err != nil {
		return false, err
	}
	if err := os.Rename(tempDir, packageDir); err != nil {
		return false, err
	}

	return true, nil
}

// Store compresses the package dir and store in cache,
// the meta is expected to be a string and would be used to calculate the hash key for cache.
func (a ArtifactConfig) Store(packageDir, meta string) error {
	// skip when offline.
	if a.ctx.Offline() {
		return nil
	}

	if !fileio.PathExists(packageDir) {
		return fmt.Errorf("package dir does not exist: %s", packageDir)
	}

	// Validate packageDir format and extract metadata.
	// Path format: packages/platform/project/buildType/nameVersion
	parts := strings.Split(filepath.ToSlash(packageDir), "/")
	if len(parts) < 5 {
		return fmt.Errorf("invalid package dir: %s", packageDir)
	}

	// Extract from path components.
	nameVersion := parts[len(parts)-1]
	buildType := parts[len(parts)-2]
	projectName := parts[len(parts)-3]
	platformName := parts[len(parts)-4]

	// Validate nameVersion format (should be name@version)
	versionParts := strings.Split(nameVersion, "@")
	if len(versionParts) != 2 {
		return fmt.Errorf("invalid package dir: %s", packageDir)
	}

	var (
		libName    = versionParts[0]
		libVersion = versionParts[1]
	)

	// Calculate checksum of metadata，this would be the cache key.
	data := sha256.Sum256([]byte(meta))
	hash := fmt.Sprintf("%x", data)

	remoteFileDir := filepath.Join(a.cacheDir, platformName, projectName, buildType, nameVersion)
	remoteFilePath := filepath.Join(remoteFileDir, hash+".tar.gz")
	metaPath := filepath.Join(remoteFileDir, "metas", hash+".meta")

	// Compress package dir to a temp archive.
	if err := dirs.CleanTmpFilesDir(); err != nil {
		return fmt.Errorf("failed to clean tmp files dir -> %w", err)
	}
	tmpFile, err := os.CreateTemp(dirs.TmpFilesDir, fmt.Sprintf("%s@%s-*.tar.gz", libName, libVersion))
	if err != nil {
		return err
	}
	tmpFile.Close()
	defer os.Remove(tmpFile.Name())

	if err := fileio.Targz(tmpFile.Name(), packageDir, false); err != nil {
		return err
	}

	// Store the meta file before the archive, it's tiny so skip the progress bar.
	metaTmpPath := filepath.Join(dirs.TmpFilesDir, hash+".meta")
	if err := os.WriteFile(metaTmpPath, []byte(meta), os.ModePerm); err != nil {
		return err
	}
	defer os.Remove(metaTmpPath)
	if err := a.uploadFile(metaTmpPath, metaPath, hash, false); err != nil {
		return err
	}

	// Store the archive with retry for transient IO failures.
	tmpFileSha256, err := fileio.SHA256Sum(tmpFile.Name())
	if err != nil {
		return err
	}
	if err := a.UploadFile(tmpFile.Name(), remoteFilePath, tmpFileSha256); err != nil {
		return err
	}

	return nil
}
