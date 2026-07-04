package pkgcache

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/celer-pkg/celer/pkgs/git"
)

type RepoConfig struct {
	ctx      context.Context
	writable bool
	chattrFS *fileio.ChattrFS
}

func NewRepoConfig(ctx context.Context, writable bool) *RepoConfig {
	pkgCacheConfig := ctx.PkgCacheConfig()
	if pkgCacheConfig == nil || pkgCacheConfig.GetDir(context.PkgCacheDirRoot) == "" {
		return nil
	}

	return &RepoConfig{
		ctx:      ctx,
		writable: writable,
		chattrFS: fileio.NewChattrFS(pkgCacheConfig.GetDir(context.PkgCacheDirRoot)),
	}
}

// Store packs a source tree into repo cache.
// - for archive sources, repoDir is the source dir in buildtrees.
// - for archive source, the archiveFile is the path to the original archive file.
func (r RepoConfig) Store(nameVersion, repoUrl, repoDir, archiveFile string) (string, error) {
	// skip storing cache when offline.
	if r.ctx.Offline() {
		return "", nil
	}

	// skip when pkgcache is not writable.
	if !r.writable {
		return "", nil
	}

	// Only third-party libraries can be cached.
	if !r.shouldCacheRepo(nameVersion) {
		return "", nil
	}

	// Create folder to store repo archive.
	cacheRepoDir := r.ctx.PkgCacheConfig().GetDir(context.PkgCacheDirRepos)
	if err := r.chattrFS.MkdirAll(cacheRepoDir, fileio.CacheDirPerm); err != nil {
		return "", err
	}

	if strings.HasSuffix(repoUrl, ".git") {
		commit, err := git.GetCommitHash(repoDir)
		if err != nil {
			return "", fmt.Errorf("read current commit -> %w", err)
		}

		// Ignore when repo archive is stored before.
		// Archive name will be like: x264@stable/472338e072b6a83fd47825cc91cef81dc848e564.tar.gz
		archivePath := filepath.Join(cacheRepoDir, nameVersion, commit+".tar.gz")
		if fileio.PathExists(archivePath) {
			return "", nil
		}

		// Create repo name folder.
		if err := r.chattrFS.MkdirAll(filepath.Dir(archivePath), fileio.CacheDirPerm); err != nil {
			return "", err
		}

		// Compress to temp dir first (outside cache), then copy to final path.
		// chattr +a allows creating new files but not renaming.
		if err := dirs.CleanTmpFilesDir(); err != nil {
			return "", fmt.Errorf("failed to clean tmp files dir -> %w", err)
		}
		tempArchivePath := filepath.Join(dirs.TmpFilesDir, fmt.Sprintf("%s-%d.tar.gz", nameVersion, time.Now().UnixMilli()))
		if err := fileio.Targz(tempArchivePath, repoDir, false); err != nil {
			return "", err
		}
		defer os.Remove(tempArchivePath)
		if err := r.chattrFS.CopyFile(tempArchivePath, archivePath); err != nil {
			return "", err
		}

		return archivePath, nil
	} else {
		// Skip when original archive is not available (e.g. file:/// URLs).
		if !fileio.PathExists(archiveFile) {
			return "", nil
		}

		checksum, err := fileio.ComputeSHA256(archiveFile)
		if err != nil {
			return "", err
		}

		// Preserve original archive extension so Extract dispatches correctly.
		ext := fileio.Ext(filepath.Base(archiveFile))
		repoCacheDir := r.ctx.PkgCacheConfig().GetDir(context.PkgCacheDirRepos)
		archivePath := filepath.Join(repoCacheDir, nameVersion, checksum+ext)

		// Skip if already cached.
		if fileio.PathExists(archivePath) {
			return "", nil
		}

		// Create repo name folder.
		if err := r.chattrFS.MkdirAll(filepath.Dir(archivePath), fileio.CacheDirPerm); err != nil {
			return "", err
		}

		// Copy original archive to repo cache dir.
		if err := r.chattrFS.CopyFile(archiveFile, archivePath); err != nil {
			return "", err
		}

		return archivePath, nil
	}
}

// Restore extract restored archive to destination and return the archive filepath that restored from.
// the checksum maybe sha256 of a file or git commit hash.
func (r RepoConfig) Restore(nameVersion, repoUrl, repoDir, checksum string) (string, error) {
	// skip restore cache when offline.
	if r.ctx.Offline() {
		return "", nil
	}

	// Ignore when repoRef is empty.
	if strings.TrimSpace(checksum) == "" {
		return "", nil
	}

	// Only third-party libraries can be cached.
	if !r.shouldCacheRepo(nameVersion) {
		return "", nil
	}

	// For git source repo, the storage archive archiveExt is ".tar.gz",
	// For archive source repo, the storage archive archiveExt is same as original archive.
	archiveExt := ".tar.gz"
	if !strings.HasSuffix(repoUrl, ".git") {
		archiveExt = fileio.Ext(filepath.Base(repoUrl))
	}

	// Locate cached archive by checksum.
	reposCacheDir := r.ctx.PkgCacheConfig().GetDir(context.PkgCacheDirRepos)
	archivePath := filepath.Join(reposCacheDir, nameVersion, checksum+archiveExt)
	if !fileio.PathExists(archivePath) {
		return "", nil
	}

	// Create a clean repo dir.
	if err := os.RemoveAll(repoDir); err != nil {
		return "", err
	}
	if err := os.MkdirAll(repoDir, os.ModePerm); err != nil {
		return "", err
	}

	// Extract archive to repo dir.
	if err := fileio.Extract(archivePath, repoDir); err != nil {
		return "", err
	}

	// Flatten nested directory, many source archives contain a single wrapping dir like ffmpeg-4.4/.
	if !strings.HasSuffix(repoUrl, ".git") {
		if err := fileio.FlattenNestedDir(repoDir); err != nil {
			_ = os.RemoveAll(repoDir)
			return "", err
		}
	}

	// Verify cached archive integrity.
	var localChecksum string
	if strings.HasSuffix(repoUrl, ".git") {
		commitHash, err := git.GetCommitHash(repoDir)
		if err != nil {
			_ = os.RemoveAll(repoDir)
			return "", fmt.Errorf("invalid cached repo, read commit failed -> %w", err)
		}
		localChecksum = commitHash
	} else {
		cachedChecksum, err := fileio.ComputeSHA256(archivePath)
		if err != nil {
			_ = os.RemoveAll(repoDir)
			return "", fmt.Errorf("invalid cached repo, verify checksum failed -> %w", err)
		}
		localChecksum = cachedChecksum

		// Initialize archive source as local git repo, so they won't be treated as user local modifications.
		// Clone returns early after successful Restore, so the git init that normally happens
		// in the Clone archive branch is skipped. Restore must init the git repo itself.
		if err := git.InitAsLocalRepo(repoDir, "init for tracking file change"); err != nil {
			return "", err
		}
	}

	// Check if stored repo was modified.
	if localChecksum != checksum {
		_ = os.RemoveAll(repoDir)
		return "", fmt.Errorf("cached repo checksum mismatch, expect %s, got %s", checksum, localChecksum)
	}

	return archivePath, nil
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
