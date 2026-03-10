package pkgcache

import (
	"celer/context"
	"celer/pkgs/fileio"
	"celer/pkgs/git"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"time"
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

// Store pack package as archive and save it to sub-dir in package dir.
func (r Repo) Store(repoUrl, repoDir string) (string, error) {
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
		commit, err := git.GetCurrentCommit(repoDir)
		if err != nil {
			return "", fmt.Errorf("read current commit -> %w", err)
		}

		// Ignore when repo archive is stored before.
		// Archive name will be like: x264/472338e072b6a83fd47825cc91cef81dc848e564.tar.gz
		repoName := r.gitRepoName(repoUrl)
		archivePath := filepath.Join(r.repoCacheDir, repoName, commit+".tar.gz")
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
		tempArchivePath := fmt.Sprintf("%s/archive.tmp-%d", os.TempDir(), millisecond)
		if err := fileio.Targz(tempArchivePath, repoDir, false); err != nil {
			return "", err
		}
		checksum, err := fileio.CalculateChecksum(tempArchivePath)
		if err != nil {
			return "", err
		}

		// Ignore when repo archive is stored before.
		// Archive name will be like: x264/472338e072b6a83fd47825cc91cef81dc848e564.tar.gz
		repoName := r.gitRepoName(repoUrl)
		archivePath := filepath.Join(r.repoCacheDir, repoName, checksum+".tar.gz")
		if fileio.PathExists(archivePath) {
			return "", nil
		}

		// Create repo name folder if not exist.
		if err := os.MkdirAll(filepath.Dir(archivePath), os.ModePerm); err != nil {
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
func (r Repo) Restore(repoUrl, repoDir, repoRef string) (string, error) {
	// skip restore cache when offline.
	if r.ctx.Offline() {
		return "", nil
	}

	// Ignore when repoRef is not git commit hash.
	if strings.TrimSpace(repoRef) == "" || !git.IsCommitHash(repoRef) {
		return "", nil
	}

	// Check if repo archive exist.
	archivePath := filepath.Join(r.repoCacheDir, r.gitRepoName(repoUrl), repoRef+".tar.gz")
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

	// Check if commit hash matches.
	restoredCommit, err := git.GetCurrentCommit(repoDir)
	if err != nil {
		_ = os.RemoveAll(repoDir)
		return "", fmt.Errorf("invalid cached repo, read commit failed -> %w", err)
	}
	if restoredCommit != repoRef {
		_ = os.RemoveAll(repoDir)
		return "", fmt.Errorf("cached repo commit mismatch, expect %s, got %s", repoRef, restoredCommit)
	}

	return archivePath, nil
}

func (r Repo) gitRepoName(repoURL string) string {
	repoURL = strings.TrimSuffix(strings.TrimSpace(repoURL), "/")
	name := filepath.Base(repoURL)
	return strings.TrimSuffix(name, ".git")
}
