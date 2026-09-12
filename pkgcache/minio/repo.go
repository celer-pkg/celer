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
		writable: pkgCache.GetOptions().Writable,
	}
}

// Store packs a source tree into repo cache.
// - for archive sources, repoDir is the source dir in buildtrees.
// - for archive source, the archiveFile is the path to the original archive file.
func (r RepoConfig) Store(repoDir, repoUrl, repoRef, nameVersion, archiveFile string) error {
	// skip when offline.
	if r.ctx.Offline() {
		return nil
	}

	// skip when pkgcache is not writable.
	if !r.writable {
		return nil
	}

	// Only third-party libraries can be cached.
	if !r.shouldCacheRepo(nameVersion) {
		return nil
	}

	if strings.HasSuffix(repoUrl, ".git") {
		return r.storeGitRepo(repoDir, repoRef, nameVersion)
	} else {
		return r.storeArchiveRepo(repoRef, nameVersion, archiveFile)
	}
}

// Restore extracts the cached repo archive to repoDir.
// The checksum maybe sha256 of a file or git commit hash.
func (r RepoConfig) Restore(repoDir, repoUrl, repoRef, nameVersion, checksum, archiveName string) (bool, error) {
	// skip when offline.
	if r.ctx.Offline() {
		return false, nil
	}

	// Only third-party libraries can be cached.
	if !r.shouldCacheRepo(nameVersion) {
		return false, nil
	}

	// For git source repo, the storage archive ext is ".tar.gz",
	// For archive source repo, the storage archive ext is same as original archive.
	ext := ".tar.gz"
	if !strings.HasSuffix(repoUrl, ".git") {
		ext = fileio.Ext(filepath.Base(repoUrl))
	}

	// Locate cached archive by repoRef.
	objectName := filepath.Join(r.cacheDir, nameVersion, repoRef+ext)

	// Check if the cached archive exists before downloading.
	remoteInfo, err := r.GetFileInfo(objectName)
	if err != nil {
		return false, fmt.Errorf("failed to get object info for '%s' -> %w", objectName, err)
	}
	if remoteInfo == nil {
		return false, nil
	}

	downloaded, err := r.downloadFile(objectName, nameVersion)
	if err != nil {
		return false, fmt.Errorf("failed to download '%s' -> %w", objectName, err)
	}
	defer os.Remove(downloaded)

	// Create a clean repo dir.
	if err := os.RemoveAll(repoDir); err != nil {
		return false, err
	}
	if err := os.MkdirAll(repoDir, os.ModePerm); err != nil {
		return false, err
	}

	// Extract archive to repo dir (status reported here, not at flatten).
	if err := fileio.NewProgressTask(fileio.OpExtract, nameVersion).Start(repoDir, func() error {
		if err := fileio.Extract(downloaded, repoDir); err != nil {
			return err
		}
		// Flatten nested directory, many source archives contain a single wrapping dir like ffmpeg-4.4/.
		if !strings.HasSuffix(repoUrl, ".git") {
			if err := fileio.FlattenNestedDir(repoDir); err != nil {
				return err
			}
		}
		return nil
	}); err != nil {
		_ = os.RemoveAll(repoDir)
		return false, err
	}

	if strings.HasSuffix(repoUrl, ".git") {
		// Verify the checkout matches the expected commit. Prefer the known
		// checksum; otherwise resolve repoRef to its commit locally. Comparing
		// commits instead of tag names is robust when several tags point at the
		// same commit (e.g. spirv-tools tags both 'vulkan-sdk-1.4.335.0' and
		// 'v2025.5' at the same commit).
		expectedCommit := strings.TrimSpace(checksum)
		if expectedCommit == "" {
			commit, err := git.ResolveRefCommit(repoDir, repoRef)
			if err != nil {
				_ = os.RemoveAll(repoDir)
				return false, fmt.Errorf("invalid cached repo, resolve ref '%s' failed for '%s' -> %w", repoRef, nameVersion, err)
			}
			expectedCommit = commit
		}

		if expectedCommit != "" {
			localCommit, err := git.GetCommitHash(repoDir)
			if err != nil {
				_ = os.RemoveAll(repoDir)
				return false, fmt.Errorf("git repo is broken for '%s' -> %w", repoDir, err)
			} else if localCommit != expectedCommit {
				_ = os.RemoveAll(repoDir)
				return false, fmt.Errorf("repo commit don't match, expect '%s', got '%s'", expectedCommit, localCommit)
			}
		}
	} else {
		// Check if stored repo was modified by comparing sha256.
		if expected := r.metaSha256(remoteInfo); expected != "" {
			if got, err := fileio.SHA256Sum(downloaded); err != nil {
				return false, err
			} else if got != expected {
				return false, nil
			}
		}

		// Verify checksum if not empty also.
		if checksum != "" {
			localChecksum, err := fileio.SHA256Sum(downloaded)
			if err != nil {
				_ = os.RemoveAll(repoDir)
				return false, fmt.Errorf("invalid cached repo, verify checksum failed for %s -> %w", nameVersion, err)
			} else if localChecksum != checksum {
				_ = os.RemoveAll(repoDir)
				return false, fmt.Errorf("invalid cached repo, verify checksum failed for %s -> %w", nameVersion, err)
			}
		}

		// Initialize archive source as local git repo, so they won't be treated as user local modifications.
		// Clone returns early after successful Restore, so the git init that normally happens
		// in the Clone archive branch is skipped. Restore must init the git repo itself.
		if err := git.InitAsLocalRepo(repoDir, nameVersion); err != nil {
			return false, fmt.Errorf("failed to init %s for tracing file change -> %w", nameVersion, err)
		}

		// Restore to downloads also, it's required to compute meta when build.
		downloadsDir := r.ctx.Downloads()
		archiveName = expr.If(archiveName == "", filepath.Base(repoUrl), archiveName)
		destArchivePath := filepath.Join(downloadsDir, archiveName)
		if err := os.MkdirAll(downloadsDir, os.ModePerm); err != nil {
			return false, fmt.Errorf("failed to mkdir downloads '%s' -> %w", downloadsDir, err)
		}
		if err := fileio.CopyFile(downloaded, destArchivePath); err != nil {
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
