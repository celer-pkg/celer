package minio

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/expr"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/celer-pkg/celer/pkgs/git"
	"github.com/minio/minio-go/v7"
)

type RepoConfig struct {
	minioCache
	ctx      context.Context
	cacheDir string
	writable bool
}

func NewRepoConfig(ctx context.Context, client *minio.Client) *RepoConfig {
	pkgCache := ctx.PkgCache()
	if pkgCache == nil || pkgCache.GetMinio() == nil {
		return nil
	}

	minioConfig := pkgCache.GetMinio()
	return &RepoConfig{
		ctx: ctx,
		minioCache: minioCache{
			client:     client,
			bucketName: bucketName,
		},
		cacheDir: minioConfig.GetDir(pkgcache.DirRepos, ctx.Version()),
		writable: pkgCache.IsWritable(),
	}
}

// Store packs a source tree into repo cache.
// - for archive sources, repoDir is the source dir in buildtrees.
// - for archive source, the archiveFile is the path to the original archive file.
func (r RepoConfig) Store(cacheKey string, repoInfo pkgcache.RepoInfo, archiveFile string) error {
	// skip when offline.
	if r.ctx.Offline() {
		return nil
	}

	// skip when pkgcache is not writable.
	if !r.writable {
		return nil
	}

	// Only third-party libraries can be cached.
	if !r.shouldCacheRepo(repoInfo.NameVersion) {
		return nil
	}

	// Create default bucket if not exist.
	if err := r.CreateBucketIfNotExist(); err != nil {
		return err
	}

	if strings.HasSuffix(repoInfo.RepoUrl, ".git") {
		return r.storeGitRepo(repoInfo.RepoDir, cacheKey, repoInfo.NameVersion)
	} else {
		return r.storeArchiveRepo(cacheKey, repoInfo.NameVersion, archiveFile)
	}
}

// Restore extracts the cached repo archive to repoDir.
// The checksum maybe sha256 of a file or git commit hash.
func (r RepoConfig) Restore(cacheKey string, repoInfo pkgcache.RepoInfo, archiveName string, prebuilt bool) (bool, error) {
	// skip when offline.
	if r.ctx.Offline() {
		return false, nil
	}

	// Only third-party libraries can be cached.
	if !r.shouldCacheRepo(repoInfo.NameVersion) {
		return false, nil
	}

	// For git source repo, the storage archive ext is ".tar.gz",
	// For archive source repo, the storage archive ext is same as original archive.
	ext := ".tar.gz"
	if !strings.HasSuffix(repoInfo.RepoUrl, ".git") {
		ext = fileio.Ext(filepath.Base(repoInfo.RepoUrl))
	}

	// Locate cached archive by repoRef.
	objectName := filepath.Join(r.cacheDir, repoInfo.NameVersion, cacheKey+ext)

	// Check if the cached archive exists before downloading.
	remoteInfo, err := r.GetFileInfo(objectName)
	if err != nil {
		return false, fmt.Errorf("failed to get object info for '%s' -> %w", objectName, err)
	}
	if remoteInfo == nil {
		return false, nil
	}

	// Download the cached archive into a task-owned tmp dir with progress.
	localTmpDir, err := dirs.NewTmpFilesDir()
	if err != nil {
		return false, err
	}
	defer os.RemoveAll(localTmpDir)

	tmpDownloaded, err := r.downloadFile(localTmpDir, objectName, repoInfo.NameVersion)
	if err != nil {
		return false, fmt.Errorf("failed to download '%s' -> %w", objectName, err)
	}

	// Create a clean repo dir.
	if err := os.RemoveAll(repoInfo.RepoDir); err != nil {
		return false, err
	}
	if err := os.MkdirAll(repoInfo.RepoDir, os.ModePerm); err != nil {
		return false, err
	}

	// Extract archive to repo dir (status reported here, not at flatten).
	if err := fileio.NewProgressTask(fileio.OpExtract, repoInfo.NameVersion).Start(repoInfo.RepoDir, func() error {
		if err := fileio.Extract(tmpDownloaded, repoInfo.RepoDir); err != nil {
			return err
		}
		// Flatten nested directory, many source archives contain a single wrapping dir like ffmpeg-4.4/.
		if !strings.HasSuffix(repoInfo.RepoUrl, ".git") {
			if err := fileio.FlattenNestedDir(repoInfo.RepoDir); err != nil {
				return err
			}
		}
		return nil
	}); err != nil {
		_ = os.RemoveAll(repoInfo.RepoDir)
		return false, err
	}

	if strings.HasSuffix(repoInfo.RepoUrl, ".git") {
		// Verify the checkout matches the expected commit. Prefer the known
		// checksum; otherwise resolve repoRef to its commit locally. Comparing
		// commits instead of tag names is robust when several tags point at the
		// same commit (e.g. spirv-tools tags both 'vulkan-sdk-1.4.335.0' and
		// 'v2025.5' at the same commit).
		expectedCommit := strings.TrimSpace(repoInfo.Checksum)
		if expectedCommit == "" {
			commit, err := git.ResolveRefCommit(repoInfo.RepoDir, repoInfo.RepoRef)
			if err != nil {
				_ = os.RemoveAll(repoInfo.RepoDir)
				return false, fmt.Errorf("invalid cached repo, resolve ref '%s' failed for '%s' -> %w",
					repoInfo.RepoRef, repoInfo.NameVersion, err)
			}
			expectedCommit = commit
		}

		if expectedCommit != "" {
			localCommit, err := git.GetCommitHash(repoInfo.RepoDir)
			if err != nil {
				_ = os.RemoveAll(repoInfo.RepoDir)
				return false, fmt.Errorf("git repo is broken for '%s' -> %w", repoInfo.RepoDir, err)
			} else if localCommit != expectedCommit {
				_ = os.RemoveAll(repoInfo.RepoDir)
				return false, fmt.Errorf("repo commit don't match, expect '%s', got '%s'", expectedCommit, localCommit)
			}
		}
	} else {
		// Check if stored repo was modified by comparing sha256.
		if expected := r.metaSha256(remoteInfo); expected != "" {
			if got, err := fileio.SHA256Sum(tmpDownloaded); err != nil {
				return false, err
			} else if got != expected {
				return false, nil
			}
		}

		// Verify checksum if not empty also.
		if repoInfo.Checksum != "" {
			localChecksum, err := fileio.SHA256Sum(tmpDownloaded)
			if err != nil {
				_ = os.RemoveAll(repoInfo.RepoDir)
				return false, fmt.Errorf("invalid cached repo, verify checksum failed for %s -> %w", repoInfo.NameVersion, err)
			} else if localChecksum != repoInfo.Checksum {
				_ = os.RemoveAll(repoInfo.RepoDir)
				return false, fmt.Errorf("invalid cached repo, verify checksum failed for %s -> %w", repoInfo.NameVersion, err)
			}
		}

		// Initialize archive source as local git repo, so they won't be treated as user local modifications.
		// Clone returns early after successful Restore, so the git init that normally happens
		// in the Clone archive branch is skipped. Restore must init the git repo itself.
		// Skip prebuilt: its files are deleted from buildtrees right after being copied to
		// the package dir, so git init/add is wasted work.
		if !prebuilt {
			if err := git.InitAsLocalRepo(repoInfo.RepoDir, repoInfo.NameVersion); err != nil {
				return false, fmt.Errorf("failed to init %s for tracing file change -> %w", repoInfo.NameVersion, err)
			}
		}

		// Restore to downloads also, it's required to compute meta when build.
		downloadsDir := r.ctx.Downloads()
		archiveName = expr.If(archiveName == "", filepath.Base(repoInfo.RepoUrl), archiveName)
		destArchivePath := filepath.Join(downloadsDir, archiveName)
		if err := os.MkdirAll(downloadsDir, os.ModePerm); err != nil {
			return false, fmt.Errorf("failed to mkdir downloads '%s' -> %w", downloadsDir, err)
		}
		if err := fileio.CopyFile(tmpDownloaded, destArchivePath); err != nil {
			return false, fmt.Errorf("failed to move archive to downloads -> %w", err)
		}
	}

	return true, nil
}

func (r RepoConfig) storeGitRepo(repoDir, repoRef, nameVersion string) error {
	// Skip uploading if already cached.
	remotePath := filepath.Join(r.cacheDir, nameVersion, repoRef+".tar.gz")
	remoteInfo, err := r.GetFileInfo(remotePath)
	if err != nil {
		return fmt.Errorf("failed to get minio object info for '%s' -> %w", remotePath, err)
	}
	if remoteInfo != nil {
		return nil
	}

	// Compress to local temp dir.
	localTmpFile := filepath.Join(os.TempDir(), fmt.Sprintf("celer-repo-%s-%d.tar.gz", nameVersion, time.Now().UnixMilli()))
	if err := fileio.Targz(localTmpFile, repoDir, false); err != nil {
		return err
	}
	defer os.Remove(localTmpFile)

	// Upload repo archive with progress.
	if err := r.uploadFile(localTmpFile, remotePath, nameVersion); err != nil {
		return fmt.Errorf("failed to upload repo for '%s' -> %w", nameVersion, err)
	}

	return nil
}

func (r RepoConfig) storeArchiveRepo(repoRef, nameVersion, archivePath string) error {
	// Skip when original archive is not available (e.g. file:/// URLs).
	if !fileio.PathExists(archivePath) {
		return nil
	}

	// Preserve original archive extension so extract dispatches correctly.
	ext := fileio.Ext(archivePath)
	remotePath := filepath.Join(r.cacheDir, nameVersion, repoRef+ext)

	// Skip uploading if already cached.
	remoteInfo, err := r.GetFileInfo(remotePath)
	if err != nil {
		return fmt.Errorf("failed to get minio object info for '%s' -> %w", remotePath, err)
	}
	if remoteInfo != nil {
		return nil
	}

	// Upload repo archive with progress.
	if err := r.uploadFile(archivePath, remotePath, nameVersion); err != nil {
		return fmt.Errorf("failed to upload repo for '%s' -> %w", nameVersion, err)
	}

	return nil
}

// shouldCacheRepo default we cache all third-party library repos that defined in ports dir.
func (r RepoConfig) shouldCacheRepo(nameVersion string) bool {
	parts := strings.Split(nameVersion, "@")
	if len(parts) != 2 {
		panic("invalid nameVersion: " + nameVersion)
	}

	// Only cache third-party repos that defined in ports dir.
	portName := parts[0]
	groupChar := strings.ToLower(string([]rune(portName)[0]))
	portPath := filepath.Join(dirs.PortsDir, groupChar, portName)
	return fileio.PathExists(portPath)
}
