package filesystem

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
)

type RepoConfig struct {
	fsCache
	ctx      context.Context
	cacheDir string
	writable bool
}

func NewRepoConfig(ctx context.Context) *RepoConfig {
	pkgCache := ctx.PkgCache()
	if pkgCache == nil || pkgCache.GetFS() == nil || pkgCache.GetFS().GetDir(pkgcache.DirRoot, ctx.Version()) == "" {
		return nil
	}

	filesystem := pkgCache.GetFS()
	cacheRootDir := filesystem.GetDir(pkgcache.DirRoot, ctx.Version())
	repoCacheDir := filesystem.GetDir(pkgcache.DirRepos, ctx.Version())
	return &RepoConfig{
		fsCache: fsCache{
			cacheDirRoot: cacheRootDir,
		},
		ctx:      ctx,
		cacheDir: repoCacheDir,
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
	// for archive source repo, the storage archive ext is same as original archive.
	archiveExt := ".tar.gz"
	if !strings.HasSuffix(repoUrl, ".git") {
		archiveExt = fileio.Ext(filepath.Base(repoUrl))
	}

	// Locate cached archive by repoRef.
	remoteFilePath := filepath.Join(r.cacheDir, nameVersion, repoRef+archiveExt)
	if !fileio.PathExists(remoteFilePath) {
		return false, nil
	}

	// Download the cached archive to a local tmp file with progress.
	downloaded, err := r.downloadFile(pkgcache.KindRepo, remoteFilePath, nameVersion)
	if err != nil {
		return false, fmt.Errorf("failed to download '%s' -> %w", remoteFilePath, err)
	}
	defer os.Remove(downloaded)

	// Create a clean repo dir.
	if err := os.RemoveAll(repoDir); err != nil {
		return false, err
	}
	if err := os.MkdirAll(repoDir, os.ModePerm); err != nil {
		return false, err
	}

	// Extract archive to repo dir.
	if err := fileio.Extract(downloaded, repoDir); err != nil {
		return false, err
	}

	// Flatten nested directory, many source archives contain a single wrapping dir like ffmpeg-4.4/.
	if !strings.HasSuffix(repoUrl, ".git") {
		if err := fileio.FlattenNestedDir(repoDir); err != nil {
			_ = os.RemoveAll(repoDir)
			return false, err
		}
	}

	// Verify cached archive integrity.
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
			}
			if localCommit != expectedCommit {
				_ = os.RemoveAll(repoDir)
				return false, fmt.Errorf("repo commit don't match, expect '%s', got '%s'", expectedCommit, localCommit)
			}
		}
	} else {
		// Verify checksum if not empty.
		if checksum != "" {
			remoteChecksum, err := fileio.SHA256Sum(downloaded)
			if err != nil {
				_ = os.RemoveAll(repoDir)
				return false, fmt.Errorf("invalid cached repo, verify checksum failed for %s -> %w", nameVersion, err)
			}
			if remoteChecksum != checksum {
				_ = os.RemoveAll(repoDir)
				return false, fmt.Errorf("cached repo checksum mismatch, expect %s, got %s", checksum, remoteChecksum)
			}
		}

		// Initialize archive source as local git repo, so they won't be treated as user local modifications.
		// Clone returns early after successful Restore, so the git init that normally happens
		// in the Clone archive branch is skipped. Restore must init the git repo itself.
		if err := git.InitAsLocalRepo(repoDir, nameVersion); err != nil {
			return false, fmt.Errorf("failed to init %s for tracing file change -> %w", nameVersion, err)
		}

		// Copy to downloads also, it's required to compute meta when build.
		downloadsDir := r.ctx.Downloads()
		fileName, err := fileio.FileName(r.ctx, repoUrl)
		if err != nil {
			return false, fmt.Errorf("failed to get file name with '%s' -> %w", repoUrl, err)
		}
		archiveName = expr.If(archiveName == "", fileName, archiveName)
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
	// A repo without a fixed ref (e.g. HEAD-tracking with empty repoRef) cannot
	// be keyed, skip caching it.
	if strings.TrimSpace(repoRef) == "" {
		return nil
	}

	// Ignore when repo archive is stored before.
	// Archive name will be like: repos/x264@stable/stable.tar.gz
	remoteFilePath := filepath.Join(r.cacheDir, nameVersion, repoRef+".tar.gz")
	if fileio.PathExists(remoteFilePath) {
		return nil
	}

	// Compress to a local temp file first (outside cache), then upload it.
	localTmpFile := filepath.Join(os.TempDir(), fmt.Sprintf("celer-repo-%s-%d.tar.gz", nameVersion, time.Now().UnixMilli()))
	if err := fileio.Targz(localTmpFile, repoDir, false); err != nil {
		return err
	}
	defer os.Remove(localTmpFile)

	// The archive is named after the repoRef, use its sha256 to skip re-uploads.
	archiveSha256, err := fileio.SHA256Sum(localTmpFile)
	if err != nil {
		return err
	}
	return r.uploadFile(pkgcache.KindRepo, localTmpFile, remoteFilePath, archiveSha256, nameVersion)
}

func (r RepoConfig) storeArchiveRepo(repoRef, nameVersion, archiveFile string) error {
	// A source without a fixed ref cannot be keyed, skip caching it.
	if strings.TrimSpace(repoRef) == "" {
		return nil
	}

	// Skip when original archive is not available (e.g. file:/// URLs).
	if !fileio.PathExists(archiveFile) {
		return nil
	}

	// Preserve original archive extension so extract dispatches correctly.
	ext := fileio.Ext(archiveFile)
	archivePath := filepath.Join(r.cacheDir, nameVersion, repoRef+ext)

	// Skip if already cached.
	if fileio.PathExists(archivePath) {
		return nil
	}

	// The archive's sha256 lets uploadFile skip re-uploads and resolve
	// multi-user races (another user stored the same archive first).
	checksum, err := fileio.SHA256Sum(archiveFile)
	if err != nil {
		return err
	}
	return r.uploadFile(pkgcache.KindRepo, archiveFile, archivePath, checksum, nameVersion)
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
