package pkgcache

import (
	"crypto/sha256"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

type ArtifactConfig struct {
	context.Context

	writable   bool
	chattrFS   *fileio.ChattrFS
	maxRetries int
}

func NewArtifactConfig(ctx context.Context, writable bool) *ArtifactConfig {
	pkgCacheConfig := ctx.PkgCacheConfig()
	if pkgCacheConfig == nil || pkgCacheConfig.GetDir(context.PkgCacheDirArtifacts) == "" {
		return nil
	}

	return &ArtifactConfig{
		Context:    ctx,
		writable:   writable,
		chattrFS:   fileio.NewChattrFS(pkgCacheConfig.GetDir(context.PkgCacheDirRoot)),
		maxRetries: 3,
	}
}

// Restore restores the cached package to package directory if cache hit, and return the archive path.
// If cache miss, just return empty string without error.
func (a ArtifactConfig) Restore(nameVersion, buildHash, packageDir string) (string, error) {
	// skip restore cache when offline.
	if a.Offline() {
		return "", nil
	}

	platformName := a.Platform().GetName()
	projectName := a.Project().GetName()
	buildType := a.BuildType()

	artifactCacheDir := a.PkgCacheConfig().GetDir(context.PkgCacheDirArtifacts)
	archiveDir := filepath.Join(artifactCacheDir, platformName, projectName, buildType, nameVersion)
	archivePath := filepath.Join(archiveDir, buildHash+".tar.gz")
	if !fileio.PathExists(archivePath) {
		color.PrintWarning("======== no artifact found for %s and it'll build from source ========", nameVersion)
		return "", nil // not an error even not exist.
	}

	// The meta file hash should be the same as hash that calcuated dynamically.
	metaPath := filepath.Join(archiveDir, "metas", buildHash+".meta")
	metaBytes, err := os.ReadFile(metaPath)
	if err != nil {
		if errors.Is(err, os.ErrNotExist) {
			return "", fmt.Errorf("cache archive exists but metadata is missing: %s", metaPath)
		}
		return "", err
	}
	metaHash := sha256.Sum256(metaBytes)
	if fmt.Sprintf("%x", metaHash) != buildHash {
		return "", fmt.Errorf("cache metadata checksum mismatch for %s", nameVersion)
	}

	// Create tmp dir for extracting inside.
	if err := dirs.CleanTmpFilesDir(); err != nil {
		return "", fmt.Errorf("failed to clean tmp files dir -> %w", err)
	}
	tempDir, err := os.MkdirTemp(dirs.TmpFilesDir, "pkgcache-extract-*")
	if err != nil {
		return "", err
	}
	defer os.RemoveAll(tempDir)

	// Extract to a tmp dir (retry for NFS read hiccups).
	var restoreErr error
	for attempt := 1; attempt <= a.maxRetries; attempt++ {
		restoreErr = fileio.Extract(archivePath, tempDir)
		if restoreErr == nil {
			break
		}
		if attempt < a.maxRetries {
			color.Printf(color.Warning, "Restore pkgcache failed (attempt %d/%d): %v\n", attempt, a.maxRetries, err)
			time.Sleep(time.Duration(attempt) * time.Second)
		}
	}
	if restoreErr != nil {
		return "", restoreErr
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

	return archivePath, nil
}

// Store compresses the package dir and store in cache,
// the meta is expected to be a string and would be used to calculate the hash key for cache.
func (a ArtifactConfig) Store(packageDir, meta string) error {
	// skip storing cache when offline.
	if a.Offline() {
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

	// Extract tar.gz to a tmp dir.
	artifactCacheDir := a.PkgCacheConfig().GetDir(context.PkgCacheDirArtifacts)
	destDir := filepath.Join(artifactCacheDir, platformName, projectName, buildType, nameVersion)
	metaDir := filepath.Join(destDir, "metas")

	// Calculate checksum of metadata，this would be the cache key.
	data := sha256.Sum256([]byte(meta))
	hash := fmt.Sprintf("%x", data)
	archivePath := filepath.Join(destDir, hash+".tar.gz")

	// Skip if already cached — rebuild with same metadata produces identical output.
	if fileio.PathExists(archivePath) {
		return nil
	}

	// Compress package dir to a temp archive.
	archiveName := fmt.Sprintf("%s@%s.tar.gz", libName, libVersion)
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

	// Create dirs and write to cache (with retry for NFS transient issues).
	destName := filepath.Join(platformName, projectName, buildType, nameVersion)
	if err := a.chattrFS.MkdirAll(destDir, fileio.CacheDirPerm); err != nil {
		return storeErrorDiagnostic(err, destName, destDir)
	}
	if err := a.chattrFS.MkdirAll(metaDir, fileio.CacheDirPerm); err != nil {
		return storeErrorDiagnostic(err, destName, metaDir)
	}

	// Copy the compressed archive to cache. Retry on transient IO failures.
	var storeErr error
	for attempt := 1; attempt <= a.maxRetries; attempt++ {
		storeErr = a.chattrFS.CopyFile(tempArchivePath, archivePath)
		if storeErr == nil {
			break
		}
		if attempt < a.maxRetries {
			color.Printf(color.Warning, "Store pkgcache failed (attempt %d/%d): %v\n", attempt, a.maxRetries, err)
			time.Sleep(time.Duration(attempt) * time.Second)
		}
	}
	if storeErr != nil {
		return storeErrorDiagnostic(storeErr, destName, archivePath)
	}

	// Write meta file.
	metaPath := filepath.Join(metaDir, hash+".meta")
	if err := a.chattrFS.WriteFile(metaPath, []byte(meta), fileio.CacheFilePerm); err != nil {
		return err
	}

	return nil
}

// Remove removes the cache for the specified platform, project, build type and name version.
func (a ArtifactConfig) Remove(nameVersion string) error {
	platformName := a.Platform().GetName()
	projectName := a.Project().GetName()
	buildType := a.BuildType()
	artifactCacheDir := a.PkgCacheConfig().GetDir(context.PkgCacheDirArtifacts)
	pacakgeDir := filepath.Join(artifactCacheDir, platformName, projectName, buildType, nameVersion)
	if fileio.PathExists(pacakgeDir) {
		if err := os.RemoveAll(pacakgeDir); err != nil {
			return fmt.Errorf("failed toremove cache package %s -> %w", pacakgeDir, err)
		}
	}

	return nil
}

// Exist check both archive file and build desc file exist.
func (a ArtifactConfig) Exist(nameVersion, hash string) bool {
	platformName := a.Platform().GetName()
	projectName := a.Project().GetName()
	buildType := a.BuildType()
	artifactCacheDir := a.PkgCacheConfig().GetDir(context.PkgCacheDirArtifacts)
	archivePath := filepath.Join(artifactCacheDir, platformName, projectName, buildType, nameVersion, hash+".tar.gz")
	metaFilePath := filepath.Join(artifactCacheDir, platformName, projectName, buildType, nameVersion, "metas", hash+".meta")
	return fileio.PathExists(archivePath) && fileio.PathExists(metaFilePath)
}
