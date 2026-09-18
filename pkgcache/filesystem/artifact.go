package filesystem

import (
	"crypto/sha256"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/celer-pkg/celer/pkgs/logger"
)

type ArtifactConfig struct {
	fsCache
	ctx        context.Context
	cacheDir   string
	writable   bool
	maxRetries int
}

func NewArtifactConfig(ctx context.Context) *ArtifactConfig {
	pkgCache := ctx.PkgCache()
	if pkgCache == nil || pkgCache.GetFS() == nil || pkgCache.GetFS().GetDir(pkgcache.DirRoot, ctx.Version()) == "" {
		return nil
	}

	cacheRootDir := pkgCache.GetFS().GetDir(pkgcache.DirRoot, ctx.Version())
	return &ArtifactConfig{
		fsCache: fsCache{
			cacheDirRoot: cacheRootDir,
		},
		ctx:        ctx,
		cacheDir:   pkgCache.GetFS().GetDir(pkgcache.DirArtifacts, ctx.Version()),
		writable:   pkgCache.GetOptions().Writable,
		maxRetries: 3,
	}
}

// Restore restores the cached package to package directory if cache hit.
// Returns true when the package was restored from cache, false on cache miss.
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
		logger.PrintWarning("======== no artifact found for %s and it'll build from source ========", nameVersion)
		return false, nil // not an error even not exist.
	}

	// The meta file hash should be the same as hash that calcuated dynamically.
	remoteMetaPath := filepath.Join(remoteFileDir, "metas", buildHash+".meta")
	metaBytes, err := os.ReadFile(remoteMetaPath)
	if err != nil {
		if errors.Is(err, os.ErrNotExist) {
			logger.PrintWarning("======== cached artifact for %s has no metadata, it'll build from source ========", nameVersion)
			return false, nil
		}
		return false, err
	}
	metaHash := sha256.Sum256(metaBytes)
	if fmt.Sprintf("%x", metaHash) != buildHash {
		return false, fmt.Errorf("cache metadata checksum mismatch for %s", nameVersion)
	}

	// Use a task-owned tmp dir for the downloaded archive and the extracted
	// dir, so concurrent installs never clash and cleanup removes both.
	localTmpDir, err := dirs.NewTmpFilesDir()
	if err != nil {
		return false, fmt.Errorf("failed to create tmp files dir -> %w", err)
	}
	defer os.RemoveAll(localTmpDir)

	// Download the remote archive to a local tmp file with progress.
	downloaded, err := a.downloadFile(localTmpDir, remoteFilePath, nameVersion)
	if err != nil {
		return false, fmt.Errorf("failed to restore %s from pkgcache -> %w", nameVersion, err)
	}

	tmpExtractDir := filepath.Join(localTmpDir, "extract")
	if err := os.MkdirAll(tmpExtractDir, os.ModePerm); err != nil {
		return false, err
	}

	// Extract to a tmp dir.
	if err := fileio.Extract(downloaded, tmpExtractDir); err != nil {
		return false, fmt.Errorf("failed to extract '%s' to '%s' -> %w", downloaded, tmpExtractDir, err)
	}

	// Clean package dir and rename to pacakge dir.
	if err := os.RemoveAll(packageDir); err != nil {
		return false, err
	}
	if err := os.MkdirAll(filepath.Dir(packageDir), os.ModePerm); err != nil {
		return false, err
	}
	if err := os.Rename(tmpExtractDir, packageDir); err != nil {
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
	destDir := filepath.Join(a.cacheDir, platformName, projectName, buildType, nameVersion)
	archivePath := filepath.Join(destDir, hash+".tar.gz")
	metaPath := filepath.Join(destDir, "metas", hash+".meta")

	// Compress package dir into a task-owned tmp dir.
	archiveName := fmt.Sprintf("%s@%s.tar.gz", libName, libVersion)
	localTmpDir, err := dirs.NewTmpFilesDir()
	if err != nil {
		return fmt.Errorf("failed to create tmp files dir -> %w", err)
	}
	defer os.RemoveAll(localTmpDir)
	tempArchivePath := filepath.Join(localTmpDir, archiveName)

	if err := fileio.Targz(tempArchivePath, packageDir, false); err != nil {
		return err
	}

	// Store the meta file before the archive. It is tiny so skip the progress
	// bar. uploadFile stages it in the FS root tmp dir, then atomically renames
	// it into the meta's dir.
	tmpMetaPath := filepath.Join(localTmpDir, hash+".meta")
	if err := os.WriteFile(tmpMetaPath, []byte(meta), os.ModePerm); err != nil {
		return err
	}
	if err := a.uploadSilent(tmpMetaPath, metaPath, hash); err != nil {
		return err
	}

	// Store the archive with retry for transient IO failures. uploadFile skips
	// the upload when the remote archive already matches sha256 (e.g. another
	// user won the race), so retries are always safe.
	archiveSha256, err := fileio.SHA256Sum(tempArchivePath)
	if err != nil {
		return err
	}
	if err := a.uploadFile(tempArchivePath, archivePath, archiveSha256, nameVersion); err != nil {
		return fmt.Errorf("failed to upload file '%s' -> %w", nameVersion, err)
	}

	return nil
}
