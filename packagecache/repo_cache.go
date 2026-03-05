package packagecache

import (
	"celer/context"
	"celer/pkgs/color"
	"celer/pkgs/fileio"
	"celer/pkgs/git"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

type GitRepo struct {
	ctx     context.Context
	repoDir string
}

func NewGitRepo(ctx context.Context, repoDir string) GitRepo {
	return GitRepo{ctx: ctx, repoDir: repoDir}
}

func (g GitRepo) Store(repoURL string) error {
	cache := g.ctx.PackageCache()
	if cache == nil || cache.GetDir() == "" || !cache.IsWritable() {
		return nil
	}

	commit, err := git.GetCurrentCommit(g.repoDir)
	if err != nil {
		return fmt.Errorf("read current commit -> %w", err)
	}

	// Create folder to store repo archive.
	destDir := filepath.Join(cache.GetDir(), "repos")
	if err := os.MkdirAll(destDir, os.ModePerm); err != nil {
		return err
	}

	// Archive name will be like: x264-472338e072b6a83fd47825cc91cef81dc848e564.tar.gz
	archiveName := fmt.Sprintf("%s-%s.tar.gz", g.gitRepoName(repoURL), commit)
	archivePath := filepath.Join(destDir, archiveName)
	if fileio.PathExists(archivePath) {
		return nil
	}

	// Compress as a tmp tar.gz and mv to final repo archive.
	tempArchivePath := archivePath + ".tmp"
	if err := fileio.Targz(tempArchivePath, g.repoDir, false); err != nil {
		return err
	}
	if err := os.Rename(tempArchivePath, archivePath); err != nil {
		_ = os.Remove(tempArchivePath)
		return err
	}

	color.Printf(color.Hint, "✔ stored repo to cache: %s\n", archivePath)
	return nil
}

func (g GitRepo) Restore(repoURL, commit string) (bool, error) {
	cache := g.ctx.PackageCache()
	if cache == nil || cache.GetDir() == "" || commit == "" {
		return false, nil
	}

	// Check if repo archive exist.
	archivePath := filepath.Join(cache.GetDir(), "repos", fmt.Sprintf("%s-%s.tar.gz", g.gitRepoName(repoURL), commit))
	if !fileio.PathExists(archivePath) {
		return false, nil
	}

	// Create a clean repo dir.
	if err := os.RemoveAll(g.repoDir); err != nil {
		return false, err
	}
	if err := os.MkdirAll(g.repoDir, os.ModePerm); err != nil {
		return false, err
	}

	// Extract archive to repor dir.
	if err := fileio.Extract(archivePath, g.repoDir); err != nil {
		return false, err
	}

	// Check if commit hash matches.
	restoredCommit, err := git.GetCurrentCommit(g.repoDir)
	if err != nil {
		_ = os.RemoveAll(g.repoDir)
		return false, fmt.Errorf("invalid cached repo, read commit failed -> %w", err)
	}
	if restoredCommit != commit {
		_ = os.RemoveAll(g.repoDir)
		return false, fmt.Errorf("cached repo commit mismatch, expect %s, got %s", commit, restoredCommit)
	}

	color.Printf(color.Hint, "✔ restored repo from package-cache: %s\n", archivePath)
	return true, nil
}

func (g GitRepo) gitRepoName(repoURL string) string {
	repoURL = strings.TrimSuffix(strings.TrimSpace(repoURL), "/")
	name := filepath.Base(repoURL)
	return strings.TrimSuffix(name, ".git")
}
