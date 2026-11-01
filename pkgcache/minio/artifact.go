package minio

import (
	"crypto/sha256"
	"fmt"
	"os"
	"path"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgcache/meta"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/celer-pkg/celer/pkgs/logger"
	"github.com/minio/minio-go/v7"
)

type ArtifactConfig struct {
	Minio
	ctx        context.Context
	cacheDir   string
	writable   bool
	maxRetries int
}

func NewArtifactConfig(ctx context.Context, client *minio.Client) *ArtifactConfig {
	pkgCache := ctx.PkgCache()
	if pkgCache == nil || pkgCache.GetMinio() == nil {
		return nil
	}

	minioConfig := pkgCache.GetMinio()
	return &ArtifactConfig{
		ctx: ctx,
		Minio: Minio{
			client:     client,
			bucketName: bucketName,
		},
		cacheDir:   minioConfig.GetDir(pkgcache.DirArtifacts, ctx.Version()),
		writable:   pkgCache.IsWritable(),
		maxRetries: 3,
	}
}

// Store compresses the package dir and store in cache,
// the meta is expected to be a string and would be used to calculate the hash key for cache.
func (a ArtifactConfig) Store(packageDir, meta string) error {
	// skip when offline.
	if a.ctx.Offline() {
		return nil
	}

	// skip for not writable.
	if !a.writable {
		return nil
	}

	if !fileio.PathExists(packageDir) {
		return fmt.Errorf("package dir does not exist: %s", packageDir)
	}

	// Create default bucket if not exist.
	if err := a.CreateBucketIfNotExist(); err != nil {
		return err
	}

	// Validate packageDir format and extract metadata.
	// Path format: packages/platform/project/buildType/nameVersion
	parts := strings.Split(filepath.ToSlash(packageDir), "/")
	if len(parts) < 5 {
		return fmt.Errorf("invalid package dir format: %s", packageDir)
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

	// Object keys always use '/' as separator, regardless of the local platform.
	var (
		libName       = versionParts[0]
		libVersion    = versionParts[1]
		remoteDir     = path.Join(a.cacheDir, platformName, projectName, buildType, nameVersion)
		remoteMetaDir = path.Join(remoteDir, "metas")
	)

	// Calculate checksum of metadata，this would be the cache key.
	hashData := sha256.Sum256([]byte(meta))
	hashStr := fmt.Sprintf("%x", hashData)
	remoteArtifactPath := path.Join(remoteDir, hashStr+".tar.gz")

	// Compress package dir into a task-owned tmp dir.
	fileName := fmt.Sprintf("%s@%s.tar.gz", libName, libVersion)
	localTmpDir, err := dirs.NewTmpFilesDir()
	if err != nil {
		return fmt.Errorf("failed to create tmp files dir -> %w", err)
	}
	defer os.RemoveAll(localTmpDir)
	tmpArchivePath := filepath.Join(localTmpDir, fileName)
	if err := fileio.Targz(tmpArchivePath, packageDir, false); err != nil {
		return fmt.Errorf("failed to compress package as archive for '%s' -> %w", nameVersion, err)
	}

	// Upload the meta file before the archive.
	metaFilePath := path.Join(remoteMetaDir, hashStr+".meta")
	tmpMetaPath := filepath.Join(localTmpDir, hashStr+".meta")
	if err := os.WriteFile(tmpMetaPath, []byte(meta), os.ModePerm); err != nil {
		return fmt.Errorf("failed to write meta into file for '%s' -> %w", nameVersion, err)
	}
	if err := a.uploadSilent(tmpMetaPath, metaFilePath); err != nil {
		return fmt.Errorf("failed to upload meta for '%s' -> %w", nameVersion, err)
	}

	// Store never deletes: an entry that already exists under this key is
	// replaced in place.
	if info, err := a.GetFileInfo(remoteArtifactPath); err != nil {
		return fmt.Errorf("failed to check cached artifact '%s' -> %w", remoteArtifactPath, err)
	} else if info != nil {
		logger.PrintWarning("cached artifact for %s exists, it will be overwritten by this build.", nameVersion)
	}

	// Upload archive file with progress.
	if err := a.uploadFile(tmpArchivePath, remoteArtifactPath, nameVersion); err != nil {
		return fmt.Errorf("failed to upload artifact for '%s' -> %w", nameVersion, err)
	}

	return nil
}

// Restore restores the cached package to package directory if cache hit.
func (a ArtifactConfig) Restore(packageDir, nameVersion, buildsystem, buildHash string) (bool, error) {
	// skip when offline.
	if a.ctx.Offline() {
		return false, nil
	}

	// Get file meta info and check if checksum matches.
	remoteMetaFilePath, remoteArtifactPath := a.artifactPaths(nameVersion, buildHash)
	remoteMetaInfo, err := a.GetFileInfo(remoteMetaFilePath)
	if err != nil {
		return false, fmt.Errorf("failed to get file object info '%s' -> %w", remoteMetaFilePath, err)
	}

	// Use a task-owned tmp dir for the downloaded meta.
	localTmpDir, err := dirs.NewTmpFilesDir()
	if err != nil {
		return false, fmt.Errorf("failed to create tmp files dir -> %w", err)
	}
	defer os.RemoveAll(localTmpDir)

	// The meta is a tiny cache-metadata check downloaded silently; only the
	// archive download below prints a done line.
	if remoteMetaInfo == nil {
		if buildsystem != "nobuild" {
			logger.PrintWarning("cached artifact for %s has no metadata, it'll build from source.", nameVersion)
		}
		return false, nil
	}
	tmpMetaFile, err := a.downloadSilent(localTmpDir, remoteMetaFilePath)
	if err != nil {
		return false, fmt.Errorf("failed to download meta file '%s' -> %w", remoteMetaFilePath, err)
	}

	// Meta meta file and check if meta matches.
	metaBytes, err := os.ReadFile(tmpMetaFile)
	if err != nil {
		return false, fmt.Errorf("failed to read meta file '%s' -> %w", tmpMetaFile, err)
	}
	metaHash := sha256.Sum256(metaBytes)
	if fmt.Sprintf("%x", metaHash) != buildHash {
		return false, fmt.Errorf("cache metadata checksum mismatch for %s", nameVersion)
	}

	// Get file archive info and check if checksum matches.
	remoteInfo, err := a.GetFileInfo(remoteArtifactPath)
	if err != nil {
		return false, fmt.Errorf("failed to get file info '%s' -> %w", remoteArtifactPath, err)
	}
	if remoteInfo == nil {
		logger.PrintWarning("no cached artifact for %s", nameVersion)
		return false, nil
	}

	tmpDownloaded, err := a.downloadFile(localTmpDir, remoteArtifactPath, nameVersion)
	if err != nil {
		return false, fmt.Errorf("failed to download artifact '%s' -> %w", remoteArtifactPath, err)
	}

	// Verify the downloaded content with sha265.
	if expected := a.metaSha256(remoteInfo); expected != "" {
		if got, err := fileio.SHA256Sum(tmpDownloaded); err != nil {
			return false, err
		} else if got != expected {
			logger.PrintWarning("cached artifact for %s is corrupted, it'll build from source.", nameVersion)
			return false, nil
		}
	}

	tempExtractDir := filepath.Join(localTmpDir, "extract")
	if err := os.MkdirAll(tempExtractDir, os.ModePerm); err != nil {
		return false, err
	}

	// Extract to a tmp dir.
	if err := fileio.Extract(tmpDownloaded, tempExtractDir); err != nil {
		return false, fmt.Errorf("failed to extract artifact '%s' -> %w", nameVersion, err)
	}

	// Clean package dir.
	if err := os.RemoveAll(packageDir); err != nil {
		return false, err
	}
	if err := os.MkdirAll(filepath.Dir(packageDir), os.ModePerm); err != nil {
		return false, err
	}

	// Rename extracted dir as package dir.
	if err := os.Rename(tempExtractDir, packageDir); err != nil {
		return false, err
	}

	return true, nil
}

// objectIndex is the listed state of a port's objects: the stored version ids of
// each object key, and its newest storage time.
type objectIndex struct {
	versions   map[string][]string
	modifiedAt map[string]int64
}

// Prune keeps only the newest keepCount source refs of nameVersion, ordered by the
// git commit time recorded in each artifact's .meta. Without a resolver, or when
// the ref is not a resolvable commit (archive sources record a checksum), the
// object's last-modified time is used. Both the archive and its meta of a pruned
// entry are removed.
func (a ArtifactConfig) Prune(nameVersion string, keepCount int, commitTime pkgcache.CommitTime) error {
	if a.ctx.Offline() || !a.writable || keepCount <= 0 {
		return nil
	}

	// List all available object versions of this port's artifacts.
	remoteDir := path.Join(a.cacheDir, a.ctx.Platform().GetName(), a.ctx.Project().GetName(), a.ctx.BuildType(), nameVersion)
	objects, err := a.ListObjectsWithVersions(remoteDir + "/")
	if err != nil {
		return fmt.Errorf("failed to list artifacts in '%s/' -> %w", remoteDir, err)
	}

	index := indexObjects(objects)
	artifacts, err := a.collectArtifacts(remoteDir, nameVersion, index, commitTime)
	if err != nil {
		return err
	}

	// Do remove and print pruned info.
	pruned := a.removeArtifacts(remoteDir, pkgcache.FilterExpiredArtifacts(artifacts, keepCount), index)
	if len(pruned) > 0 {
		logger.PrintPass("pruned %d expired artifact(s) of %s (kept newest %d):", len(pruned), nameVersion, keepCount)
		for _, item := range pruned {
			logger.PrintHint("  - commit: %s", item.RepoRef)
			logger.PrintHint("    build:  %s", item.BuildHash)
		}
	}

	return nil
}

// indexObjects groups the listed objects by key, and remembers each object's
// newest last-modified time as the storage-time fallback.
func indexObjects(objects []minio.ObjectInfo) objectIndex {
	index := objectIndex{
		versions:   map[string][]string{},
		modifiedAt: map[string]int64{},
	}
	for _, object := range objects {
		index.versions[object.Key] = append(index.versions[object.Key], object.VersionID)
		if unix := object.LastModified.Unix(); unix > index.modifiedAt[object.Key] {
			index.modifiedAt[object.Key] = unix
		}
	}
	return index
}

// collectArtifacts reads the meta of every stored artifact and resolves its
// ordering time: the commit time of the source ref, or the storage time for
// archive sources, which record a checksum instead of a commit.
func (a ArtifactConfig) collectArtifacts(remoteDir, nameVersion string, index objectIndex, commitTime pkgcache.CommitTime) ([]pkgcache.PruneArtifact, error) {
	tmpDir, err := dirs.NewTmpFilesDir()
	if err != nil {
		return nil, err
	}
	defer os.RemoveAll(tmpDir)

	prefix := remoteDir + "/"
	var artifacts []pkgcache.PruneArtifact
	for objectKey := range index.versions {
		// The archive object name is what we want, ignore all other objects.
		fileName := strings.TrimPrefix(objectKey, prefix)
		if strings.Contains(fileName, "/") || !strings.HasSuffix(fileName, ".tar.gz") {
			continue
		}

		// Guess the meta file path and read its content.
		buildHash := strings.TrimSuffix(fileName, ".tar.gz")
		metaBytes, ok := a.readObject(tmpDir, path.Join(remoteDir, "metas", buildHash+".meta"))
		if !ok {
			continue
		}

		// Read the commit hash from meta content with nameVersion.
		commitHash, ok := meta.PortCommit(metaBytes, nameVersion)
		if !ok {
			continue
		}
		sortTime, resolved := int64(0), false
		if commitTime != nil {
			sortTime, resolved = commitTime(commitHash)
		}

		// Archive (non-git) sources have no commit to resolve; order them by the
		// object's storage time instead of keeping them forever.
		if !resolved {
			sortTime = index.modifiedAt[objectKey]
		}
		artifacts = append(artifacts, pkgcache.PruneArtifact{
			BuildHash:   buildHash,
			CommitHash:  commitHash,
			SortTime:    sortTime,
			StorageTime: index.modifiedAt[objectKey],
		})
	}
	return artifacts, nil
}

// removeArtifacts deletes the given artifacts with all their stored versions, and
// returns the ones fully removed.
func (a ArtifactConfig) removeArtifacts(remoteDir string, expired []pkgcache.PruneArtifact, index objectIndex) []pkgcache.PrunedArtifact {
	var pruned []pkgcache.PrunedArtifact
	for _, artifact := range expired {
		noError := true
		for _, objectName := range []string{
			path.Join(remoteDir, artifact.BuildHash+".tar.gz"),
			path.Join(remoteDir, "metas", artifact.BuildHash+".meta"),
		} {
			if err := a.removeObject(objectName, index.versions[objectName]); err != nil {
				logger.PrintWarning("failed to remove expired artifact '%s' -> %s", objectName, err)
				noError = false
			}
		}

		if noError {
			pruned = append(pruned, pkgcache.PrunedArtifact{BuildHash: artifact.BuildHash, RepoRef: artifact.CommitHash})
		}
	}
	return pruned
}

// Exists reports whether the package of nameVersion under buildHash can be
// restored from the cache. Both the archive and its metadata are required.
func (a ArtifactConfig) Exists(nameVersion, buildHash string) (bool, error) {
	// skip when offline.
	if a.ctx.Offline() {
		return false, nil
	}

	remoteMetaPath, remoteArtifactPath := a.artifactPaths(nameVersion, buildHash)
	remoteMetaInfo, err := a.GetFileInfo(remoteMetaPath)
	if err != nil {
		return false, fmt.Errorf("failed to get file object info '%s' -> %w", remoteMetaPath, err)
	}
	if remoteMetaInfo == nil {
		return false, nil
	}

	remoteInfo, err := a.GetFileInfo(remoteArtifactPath)
	if err != nil {
		return false, fmt.Errorf("failed to get file info '%s' -> %w", remoteArtifactPath, err)
	}
	return remoteInfo != nil, nil
}

func (a ArtifactConfig) artifactPaths(nameVersion, buildHash string) (metaPath, archivePath string) {
	remoteDir := path.Join(a.cacheDir, a.ctx.Platform().GetName(), a.ctx.Project().GetName(), a.ctx.BuildType(), nameVersion)
	return path.Join(remoteDir, "metas", buildHash+".meta"), path.Join(remoteDir, buildHash+".tar.gz")
}
