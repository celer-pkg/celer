package netfs

import (
	"crypto/sha256"
	"errors"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

type ArtifactConfig struct {
	ctx        context.Context
	writable   bool
	maxRetries int
}

func NewArtifactConfig(ctx context.Context) *ArtifactConfig {
	pkgCacheConfig := ctx.PkgCacheConfig()
	if pkgCacheConfig == nil || pkgCacheConfig.GetDir(pkgcache.PkgCacheDirArtifacts) == "" {
		return nil
	}

	return &ArtifactConfig{
		ctx:        ctx,
		writable:   pkgCacheConfig.IsWritable(),
		maxRetries: 3,
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

	artifactCacheDir := a.ctx.PkgCacheConfig().GetDir(pkgcache.PkgCacheDirArtifacts)
	remoteArchiveDir := filepath.Join(artifactCacheDir, platformName, projectName, buildType, nameVersion)
	remoteArchivePath := filepath.Join(remoteArchiveDir, buildHash+".tar.gz")
	if !fileio.PathExists(remoteArchivePath) {
		color.PrintWarning("======== no artifact found for %s and it'll build from source ========\n", nameVersion)
		return "", nil // not an error even not exist.
	}

	// The meta file hash should be the same as hash that calcuated dynamically.
	metaPath := filepath.Join(remoteArchiveDir, "metas", buildHash+".meta")
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

	// Copy remote archive file with progress.
	remoteArchiveFile, err := os.OpenFile(remoteArchivePath, os.O_RDONLY, os.ModePerm)
	if err != nil {
		return "", fmt.Errorf("can not open archive path: %s -> %w", remoteArchivePath, err)
	}
	defer remoteArchiveFile.Close()

	// Create a local tmp file as the copy destination.
	destFile, err := os.CreateTemp(os.TempDir(), "artifact-*.tar.gz")
	if err != nil {
		return "", fmt.Errorf("can not create tmp artifact file -> %w", err)
	}
	localArchivePath := destFile.Name()
	defer func() {
		destFile.Close()
		os.Remove(localArchivePath)
	}()

	// Read file info to get file size.
	info, err := os.Stat(remoteArchivePath)
	if err != nil {
		return "", fmt.Errorf("can not get file info for %s -> %w", remoteArchivePath, err)
	}

	// Copy file to local with progress bar.
	completed := func(formattedTimeCost, formattedSize string) {
		color.PrintInline(color.Pass, "[✔] %s (%s) in %s\n", nameVersion+"'s artifact is restored", formattedSize, formattedTimeCost)
		color.PrintHint("Location: %s", remoteArchivePath)
	}
	progress := fileio.NewProgressBar("restore artifact", info.Size(), completed)
	if _, err := io.Copy(io.MultiWriter(destFile, progress), remoteArchiveFile); err != nil {
		return "", fmt.Errorf("failed to restore %s from pkgcache -> %w", nameVersion, err)
	}

	// Flush and close the local archive so extraction can read it (esp. on Windows).
	if err := destFile.Sync(); err != nil {
		return "", fmt.Errorf("failed to sync file to cache -> %w", err)
	}
	if err := destFile.Close(); err != nil {
		return "", fmt.Errorf("failed to close tmp artifact file -> %w", err)
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

	// Extract to a tmp dir (retry for read hiccups).
	var restoreErr error
	for attempt := 1; attempt <= a.maxRetries; attempt++ {
		restoreErr = fileio.Extract(localArchivePath, tempDir)
		if restoreErr == nil {
			break
		}
		if attempt < a.maxRetries {
			color.Printf(color.Warning, "Restore pkgcache failed (attempt %d/%d): %v\n", attempt, a.maxRetries, restoreErr)
			time.Sleep(time.Duration(attempt) * time.Second)
		}
	}
	if restoreErr != nil {
		return "", restoreErr
	}

	// Clean package dir.
	if err := os.RemoveAll(packageDir); err != nil {
		return "", err
	}
	if err := os.MkdirAll(filepath.Dir(packageDir), os.ModePerm); err != nil {
		return "", err
	}

	if err := os.Rename(tempDir, packageDir); err != nil {
		return "", err
	}

	return remoteArchivePath, nil
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
	artifactCacheDir := a.ctx.PkgCacheConfig().GetDir(pkgcache.PkgCacheDirArtifacts)
	destDir := filepath.Join(artifactCacheDir, platformName, projectName, buildType, nameVersion)
	metaDir := filepath.Join(destDir, "metas")

	// Calculate checksum of metadata，this would be the cache key.
	data := sha256.Sum256([]byte(meta))
	hash := fmt.Sprintf("%x", data)
	archivePath := filepath.Join(destDir, hash+".tar.gz")

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

	// Create dirs and write to cache.
	if err := os.MkdirAll(destDir, fileio.CacheDirPerm); err != nil {
		return err
	}
	if err := os.MkdirAll(metaDir, fileio.CacheDirPerm); err != nil {
		return err
	}

	// Write the archive here first, then atomically os.Rename into the +a-protected dir.
	cacheRootDir := a.ctx.PkgCacheConfig().GetDir(pkgcache.PkgCacheDirRoot)
	remoteTmpDir := filepath.Join(cacheRootDir, "tmp")

	// Copy to tmp with retry for transient IO failures.
	tmpArchiveName := fmt.Sprintf("%s-%d.tar.gz", hash, time.Now().UnixNano())
	tmpArchive := filepath.Join(remoteTmpDir, tmpArchiveName)

	var storeErr error
	for attempt := 1; attempt <= a.maxRetries; attempt++ {
		if err := os.MkdirAll(remoteTmpDir, fileio.CacheDirPerm); err != nil {
			return err
		}
		storeErr = fileio.CopyFile(tempArchivePath, tmpArchive)
		if storeErr == nil {
			break
		}
		if attempt < a.maxRetries {
			color.Printf(color.Warning, "Store pkgcache failed (attempt %d/%d): %v\n", attempt, a.maxRetries, storeErr)
			time.Sleep(time.Duration(attempt) * time.Second)
		}
	}
	if storeErr != nil {
		os.Remove(tmpArchive) // best-effort cleanup
		return err
	}
	defer os.Remove(tmpArchive) // cleanup on success or early return

	// Check if another user already cached this archive (multi-user race).
	if fileio.PathExists(archivePath) {
		return nil
	}

	// Atomically place the file at its final path.
	// Source is in tmp/ (no +a), dest is in artifacts/… (has +a, but dest does not
	// exist yet so rename only creates a new entry — which +a allows).
	if err := os.Rename(tmpArchive, archivePath); err != nil {
		// Dest may have appeared between our check and rename (another user won the race).
		if fileio.PathExists(archivePath) {
			return nil
		}
		return err
	}

	// Write meta file atomically — same pattern.
	metaPath := filepath.Join(metaDir, hash+".meta")
	tmpMetaName := fmt.Sprintf("%s-%d.meta", hash, time.Now().UnixNano())
	remoteTmpMeta := filepath.Join(remoteTmpDir, tmpMetaName)
	if err := os.WriteFile(remoteTmpMeta, []byte(meta), fileio.CacheFilePerm); err != nil {
		return err
	}
	defer os.Remove(remoteTmpMeta)

	// Atomic rename meta to final location.
	if err := os.Rename(remoteTmpMeta, metaPath); err != nil {
		if fileio.PathExists(metaPath) {
			return nil // race: another user wrote it first.
		}

		// Fallback: write directly (meta is small; partial-read risk is minimal).
		if err := os.WriteFile(metaPath, []byte(meta), fileio.CacheFilePerm); err != nil {
			return err
		}
	}

	return nil
}

// Remove removes the cache for the specified platform, project, build type and name version.
func (a ArtifactConfig) Remove(nameVersion string) error {
	platformName := a.ctx.Platform().GetName()
	projectName := a.ctx.Project().GetName()
	buildType := a.ctx.BuildType()
	artifactCacheDir := a.ctx.PkgCacheConfig().GetDir(pkgcache.PkgCacheDirArtifacts)
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
	artifactCacheDir := a.ctx.PkgCacheConfig().GetDir(pkgcache.PkgCacheDirArtifacts)
	archivePath := filepath.Join(artifactCacheDir, platformName, projectName, buildType, nameVersion, hash+".tar.gz")
	metaFilePath := filepath.Join(artifactCacheDir, platformName, projectName, buildType, nameVersion, "metas", hash+".meta")
	return fileio.PathExists(archivePath) && fileio.PathExists(metaFilePath)
}
