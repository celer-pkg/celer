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
	"github.com/celer-pkg/celer/pkgcache/meta"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/celer-pkg/celer/pkgs/logger"
)

type ArtifactConfig struct {
	FileSystem
	ctx        context.Context
	cacheDir   string
	readOnly   bool
	maxRetries int
}

func NewArtifactConfig(ctx context.Context) *ArtifactConfig {
	pkgCache := ctx.PkgCache()
	if pkgCache == nil || pkgCache.GetFS() == nil || pkgCache.GetFS().GetDir(pkgcache.DirRoot, ctx.Version()) == "" {
		return nil
	}

	cacheRootDir := pkgCache.GetFS().GetDir(pkgcache.DirRoot, ctx.Version())
	return &ArtifactConfig{
		FileSystem: FileSystem{
			cacheDirRoot: cacheRootDir,
		},
		ctx:        ctx,
		cacheDir:   pkgCache.GetFS().GetDir(pkgcache.DirArtifacts, ctx.Version()),
		readOnly:   pkgCache.IsReadOnly(),
		maxRetries: 3,
	}
}

// Exists reports whether the package of nameVersion under buildHash can be
// restored from the cache.
func (a ArtifactConfig) Exists(nameVersion, buildHash string) (bool, error) {
	// skip when offline.
	if a.ctx.Offline() {
		return false, nil
	}

	metaPath, archivePath := a.artifactPaths(nameVersion, buildHash)
	if !fileio.PathExists(archivePath) || !fileio.PathExists(metaPath) {
		return false, nil
	}

	return true, nil
}

// Restore restores the cached package to package directory if cache hit.
// Returns true when the package was restored from cache, false on cache miss.
func (a ArtifactConfig) Restore(packageDir, nameVersion, buildsystem, buildHash string) (bool, error) {
	// skip when offline.
	if a.ctx.Offline() {
		return false, nil
	}

	remoteMetaPath, remoteFilePath := a.artifactPaths(nameVersion, buildHash)
	if !fileio.PathExists(remoteFilePath) {
		logger.PrintWarning("no cached artifact for %s.", nameVersion)
		return false, nil // not an error even not exist.
	}

	// The meta file hash should be the same as hash that calcuated dynamically.
	metaBytes, err := os.ReadFile(remoteMetaPath)
	if err != nil {
		if errors.Is(err, os.ErrNotExist) {
			if buildsystem != "nobuild" {
				logger.PrintWarning("cached artifact for %s has no metadata, it'll build from source.", nameVersion)
			}
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

	// skip when read-only.
	if a.readOnly {
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

// Prune keeps only the newest keepCount source refs of nameVersion, ordered by the
// git commit time recorded in each artifact's .meta. Without a resolver, or when
// the ref is not a resolvable commit (archive sources record a checksum), the
// artifact's own modification time is used. Both the archive and its meta of a
// pruned entry are removed.
func (a ArtifactConfig) Prune(nameVersion string, keepCount int, commitTime pkgcache.CommitTime) error {
	if a.ctx.Offline() || a.readOnly || keepCount <= 0 {
		return nil
	}

	packageDir := filepath.Join(
		a.cacheDir,
		a.ctx.Platform().GetName(),
		a.ctx.Project().GetName(),
		a.ctx.BuildType(),
		nameVersion,
	)
	entries, err := os.ReadDir(packageDir)
	if err != nil {
		if errors.Is(err, os.ErrNotExist) {
			return nil
		}
		return fmt.Errorf("failed to list artifacts in '%s' -> %w", packageDir, err)
	}

	// Do remove and print pruned info.
	artifacts := collectArtifacts(packageDir, nameVersion, entries, commitTime)
	pruned := removeArtifacts(packageDir, pkgcache.FilterExpiredArtifacts(artifacts, keepCount))
	if len(pruned) > 0 {
		logger.PrintPass("pruned %d expired artifact(s) of %s (kept newest %d):", len(pruned), nameVersion, keepCount)
		for _, item := range pruned {
			logger.PrintHint("  - commit: %s", item.RepoRef)
			logger.PrintHint("    build:  %s", item.BuildHash)
		}
	}

	return nil
}

// collectArtifacts reads the meta of every stored artifact and resolves its
// ordering time: the commit time of the source ref, or the artifact's own
// modification time for archive sources, which record a checksum instead.
func collectArtifacts(packageDir, nameVersion string, entries []os.DirEntry, commitTime pkgcache.CommitTime) []pkgcache.PruneArtifact {
	var artifacts []pkgcache.PruneArtifact
	for _, entry := range entries {
		if entry.IsDir() || !strings.HasSuffix(entry.Name(), ".tar.gz") {
			continue
		}
		buildHash := strings.TrimSuffix(entry.Name(), ".tar.gz")
		metaBytes, err := os.ReadFile(filepath.Join(packageDir, "metas", buildHash+".meta"))
		if err != nil {
			continue
		}
		commitHash, ok := meta.PortCommit(string(metaBytes), nameVersion)
		if !ok {
			continue
		}
		info, err := entry.Info()
		if err != nil {
			continue
		}
		sortTime, resolved := int64(0), false
		if commitTime != nil {
			sortTime, resolved = commitTime(commitHash)
		}
		if !resolved {
			// Archive (non-git) sources have no commit to resolve; order them by the
			// artifact's own storage time instead of keeping them forever.
			sortTime = info.ModTime().Unix()
		}
		artifacts = append(artifacts, pkgcache.PruneArtifact{
			BuildHash:   buildHash,
			CommitHash:  commitHash,
			SortTime:    sortTime,
			StorageTime: info.ModTime().Unix(),
		})
	}
	return artifacts
}

// removeArtifacts removes the given artifacts from packageDir.
func removeArtifacts(packageDir string, expired []pkgcache.PruneArtifact) []pkgcache.PrunedArtifact {
	var pruned []pkgcache.PrunedArtifact
	for _, artifact := range expired {
		if err := os.Remove(filepath.Join(packageDir, artifact.BuildHash+".tar.gz")); err != nil && !errors.Is(err, os.ErrNotExist) {
			logger.PrintWarning("failed to remove expired artifact '%s' -> %s", artifact.BuildHash, err)
			continue
		}
		_ = os.Remove(filepath.Join(packageDir, "metas", artifact.BuildHash+".meta"))
		pruned = append(pruned, pkgcache.PrunedArtifact{BuildHash: artifact.BuildHash, RepoRef: artifact.CommitHash})
	}
	return pruned
}

func (a ArtifactConfig) artifactPaths(nameVersion, buildHash string) (metaPath, archivePath string) {
	remoteFileDir := filepath.Join(a.cacheDir, a.ctx.Platform().GetName(), a.ctx.Project().GetName(), a.ctx.BuildType(), nameVersion)
	return filepath.Join(remoteFileDir, "metas", buildHash+".meta"), filepath.Join(remoteFileDir, buildHash+".tar.gz")
}
