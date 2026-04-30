package pkgcache

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/celer-pkg/celer/pkgs/git"
)

const RepoCacheDir = "repos"

type Repo struct {
	ctx          context.Context
	repoCacheDir string
	writable     bool
}

func NewRepo(ctx context.Context, pkgCacheDir string, writable bool) *Repo {
	if pkgCacheDir == "" {
		return nil
	}

	return &Repo{
		ctx:          ctx,
		repoCacheDir: filepath.Join(pkgCacheDir, RepoCacheDir),
		writable:     writable,
	}
}

// Store packs a source tree into repo cache.
// For archive sources, repoRef is the original archive checksum used as the cache key.
func (r Repo) Store(nameVersion, repoUrl, repoDir string) (string, error) {
	// skip storing cache when offline.
	if r.ctx.Offline() {
		return "", nil
	}

	if !r.writable {
		return "", nil
	}

	// Create folder to store repo archive.
	if err := os.MkdirAll(r.repoCacheDir, os.ModePerm); err != nil {
		return "", err
	}

	if strings.HasSuffix(repoUrl, ".git") {
		commit, err := git.GetCommitHash(repoDir)
		if err != nil {
			return "", fmt.Errorf("read current commit -> %w", err)
		}

		// Ignore when repo archive is stored before.
		// Archive name will be like: x264@stable/472338e072b6a83fd47825cc91cef81dc848e564.tar.gz
		archivePath := filepath.Join(r.repoCacheDir, nameVersion, commit+".tar.gz")
		if fileio.PathExists(archivePath) {
			return "", nil
		}

		// Create repo name folder if not exist.
		if err := os.MkdirAll(filepath.Dir(archivePath), os.ModePerm); err != nil {
			return "", err
		}

		// Compress as a tmp tar.gz and mv to final repo archive.
		millisecond := time.Now().UnixMilli()
		tempArchivePath := archivePath + fmt.Sprintf(".tmp-%d", millisecond)
		if err := fileio.Targz(tempArchivePath, repoDir, false); err != nil {
			return "", err
		}
		if err := os.Rename(tempArchivePath, archivePath); err != nil {
			_ = os.Remove(tempArchivePath)
			return "", err
		}
		return archivePath, nil
	} else {
		// Compress as a tmp tar.gz and mv to final repo archive.
		millisecond := time.Now().UnixMilli()
		tempArchivePath := fmt.Sprintf("%s/archive-%d.tmp", os.TempDir(), millisecond)
		if err := fileio.Targz(tempArchivePath, repoDir, false); err != nil {
			return "", err
		}

		checksum, err := fileio.GetFileSha256(tempArchivePath)
		if err != nil {
			os.Remove(tempArchivePath)
			return "", err
		}

		// Ignore when repo archive is stored before.
		// Archive name will be like: x264@stable/472338e072b6a83fd47825cc91cef81dc848e564.tar.gz
		archivePath := filepath.Join(r.repoCacheDir, nameVersion, checksum+".tar.gz")
		if fileio.PathExists(archivePath) {
			_ = os.Remove(tempArchivePath)
			return "", nil
		}

		// Create repo name folder if not exist.
		if err := os.MkdirAll(filepath.Dir(archivePath), os.ModePerm); err != nil {
			_ = os.Remove(tempArchivePath)
			return "", err
		}
		if err := os.Rename(tempArchivePath, archivePath); err != nil {
			_ = os.Remove(tempArchivePath)
			return "", err
		}
		return archivePath, nil
	}
}

// Restore extract restored archive to destination and return the archive filepath that restored from.
// the checksum maybe sha256 of a file or git commit hash.
func (r Repo) Restore(nameVersion, repoUrl, repoDir, checksum string) (string, error) {
	// skip restore cache when offline.
	if r.ctx.Offline() {
		return "", nil
	}

	// Ignore when repoRef is empty.
	if strings.TrimSpace(checksum) == "" {
		return "", nil
	}

	// Check if repo archive exist.
	archivePath := filepath.Join(r.repoCacheDir, nameVersion, checksum+".tar.gz")
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

	// Extract archive to repor dir.
	if err := fileio.Extract(archivePath, repoDir); err != nil {
		return "", err
	}

	// Check if commit hash matches for git repo cache.
	var localChecksum string
	if strings.HasSuffix(repoUrl, ".git") {
		if commitHash, err := git.GetCommitHash(repoDir); err != nil {
			_ = os.RemoveAll(repoDir)
			return "", fmt.Errorf("invalid cached repo, read commit failed -> %w", err)
		} else {
			localChecksum = commitHash
		}
	} else {
		checksum, err := fileio.GetFileSha256(archivePath)
		if err != nil {
			_ = os.RemoveAll(repoDir)
			return "", fmt.Errorf("invalid cached repo, read commit failed -> %w", err)
		} else {
			localChecksum = checksum
		}
	}

	// Check if stored repo was tampered.
	if localChecksum != checksum {
		_ = os.RemoveAll(repoDir)
		return "", fmt.Errorf("cached repo checksum mismatch, expect %s, got %s", checksum, localChecksum)
	}

	return archivePath, nil
}

func (r Repo) gitRepoName(repoURL string) string {
	repoURL = strings.TrimSuffix(strings.TrimSpace(repoURL), "/")
	name := filepath.Base(repoURL)
	return strings.TrimSuffix(name, ".git")
}
