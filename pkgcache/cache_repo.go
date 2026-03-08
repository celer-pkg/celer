package pkgcache

import (
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
	repoCacheDir string
	writable     bool
}

func NewRepo(pkgCacheDir string, writable bool) *Repo {
	if pkgCacheDir == "" {
		return nil
	}

	return &Repo{
		repoCacheDir: filepath.Join(pkgCacheDir, RepoCacheDir),
		writable:     writable,
	}
}

func (r Repo) Store(repoUrl, repoDir string) (string, error) {
	if !r.writable {
		return "", nil
	}

	commit, err := git.GetCurrentCommit(repoDir)
	if err != nil {
		return "", fmt.Errorf("read current commit -> %w", err)
	}

	// Create folder to store repo archive.
	if err := os.MkdirAll(r.repoCacheDir, os.ModePerm); err != nil {
		return "", err
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
}

func (r Repo) Fetch(repoUrl, repoDir, repoRef string) (string, error) {
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
