package pkgcache

import (
	"crypto/sha256"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

type ArtifactConfig struct {
	ctx        context.Context
	writable   bool
	permission context.Permission
}

func NewArtifactConfig(ctx context.Context, writable bool) *ArtifactConfig {
	pkgCache := ctx.PkgCache()
	if pkgCache == nil || pkgCache.GetDir(context.PkgCacheDirArtifacts) == "" {
		return nil
	}

	return &ArtifactConfig{
		ctx:        ctx,
		writable:   writable,
		permission: NewPermission("nfs"),
	}
}

// Restore restores the cached package to package directory if cache hit, and return the archive path.
// If cache miss, just return empty string without error.
func (a ArtifactConfig) Restore(nameVersion, buildHash, packageDir string) (string, error) {
	// skip restore cache when offline.
	if a.ctx.Offline() {
		return "", nil
	}

	platformName := a.ctx.Platform().GetName()
	projectName := a.ctx.Project().GetName()
	buildType := a.ctx.BuildType()

	artifactCacheDir := a.ctx.PkgCache().GetDir(context.PkgCacheDirArtifacts)
	archiveDir := filepath.Join(artifactCacheDir, platformName, projectName, buildType, nameVersion)
	archivePath := filepath.Join(archiveDir, buildHash+".tar.gz")
	if !fileio.PathExists(archivePath) {
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

	// Extract to a tmp dir and move back to dest dir.
	if err := fileio.Extract(archivePath, tempDir); err != nil {
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

	return archivePath, nil
}

// Store compresses the package dir and store in cache,
// the meta is expected to be a string and would be used to calculate the hash key for cache.
func (a ArtifactConfig) Store(packageDir, meta string) error {
	// skip storing cache when offline.
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

	// Extract tar.gz to a tmp dir.
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

	artifactCacheDir := a.ctx.PkgCache().GetDir(context.PkgCacheDirArtifacts)
	destDir := filepath.Join(artifactCacheDir, platformName, projectName, buildType, nameVersion)
	metaDir := filepath.Join(destDir, "metas")

	// Calculate checksum of metadata (this would be the cache key).
	data := sha256.Sum256([]byte(meta))
	hash := fmt.Sprintf("%x", data)

	// Create the dir if not exist, setting permissions on all intermediate directories.
	if err := a.permission.MkdirAll(destDir, a.ctx.PkgCache().GetDir(context.PkgCacheDirRoot)); err != nil {
		return err
	}

	// Create the meta dir if not exist, setting permissions on all intermediate directories.
	if err := a.permission.MkdirAll(metaDir, a.ctx.PkgCache().GetDir(context.PkgCacheDirRoot)); err != nil {
		return err
	}

	// Move tmp file to dest archive file.
	archivePath := filepath.Join(destDir, hash+".tar.gz")
	archiveTempPath := archivePath + ".tmp"
	if err := fileio.CopyFile(tempArchivePath, archiveTempPath); err != nil {
		return err
	}
	if err := os.Rename(archiveTempPath, archivePath); err != nil {
		return err
	}

	// Ensure full permissions and correct ownership for archived file.
	if err := a.permission.SetPermissions(archivePath); err != nil {
		return err
	}

	// Write meta file to meta dir.
	metaPath := filepath.Join(metaDir, hash+".meta")
	metaTempPath := metaPath + ".tmp"
	if err := os.WriteFile(metaTempPath, []byte(meta), os.ModePerm); err != nil {
		return err
	}
	if err := os.Rename(metaTempPath, metaPath); err != nil {
		return err
	}

	// Ensure full permissions and correct ownership for meta file.
	if err := a.permission.SetPermissions(metaPath); err != nil {
		return err
	}

	return nil
}

// Remove removes the cache for the specified platform, project, build type and name version.
func (a ArtifactConfig) Remove(nameVersion string) error {
	platformName := a.ctx.Platform().GetName()
	projectName := a.ctx.Project().GetName()
	buildType := a.ctx.BuildType()
	artifactCacheDir := a.ctx.PkgCache().GetDir(context.PkgCacheDirArtifacts)
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
	platformName := a.ctx.Platform().GetName()
	projectName := a.ctx.Project().GetName()
	buildType := a.ctx.BuildType()
	artifactCacheDir := a.ctx.PkgCache().GetDir(context.PkgCacheDirArtifacts)
	archivePath := filepath.Join(artifactCacheDir, platformName, projectName, buildType, nameVersion, hash+".tar.gz")
	metaFilePath := filepath.Join(artifactCacheDir, platformName, projectName, buildType, nameVersion, "metas", hash+".meta")
	return fileio.PathExists(archivePath) && fileio.PathExists(metaFilePath)
}
