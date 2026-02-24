package git

import (
	"fmt"
	"os"
	"os/exec"
	"strings"
)

// GetRepoUrl get git repo origin URL.
func GetRepoUrl(repoDir string) (string, error) {
	cmd := exec.Command("git", "remote", "get-url", "origin")
	cmd.Dir = repoDir
	output, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("get repo url -> %s", output)
	}
	return strings.TrimSpace(string(output)), nil
}

// GetCurrentBranch read current branch of repo.
func GetCurrentBranch(repoDir string) (string, error) {
	cmd := exec.Command("git", "symbolic-ref", "--short", "HEAD")
	cmd.Dir = repoDir
	output, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("get current branch -> %s", output)
	}
	return strings.TrimSpace(string(output)), nil
}

// GetCurrentTag read current tag of repo.
func GetCurrentTag(repoDir string) (string, error) {
	cmd := exec.Command("git", "describe", "--exact-match", "--tags", "HEAD")
	cmd.Dir = repoDir
	output, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("get current tag -> %s", output)
	}
	return strings.TrimSpace(string(output)), nil
}

// GetCurrentCommit read commit hash of repo.
func GetCurrentCommit(repoDir string) (string, error) {
	cmd := exec.Command("git", "rev-parse", "HEAD")
	cmd.Dir = repoDir
	output, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("get current commit -> %s", output)
	}
	return strings.TrimSpace(string(output)), nil
}

// IsModified check if repo is modified.
func IsModified(repoDir string) (bool, error) {
	cmd := exec.Command("git", "status", "--porcelain")
	cmd.Dir = repoDir
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false, fmt.Errorf("check if repo is modified -> %s", output)
	}

	return strings.TrimSpace(string(output)) != "", nil
}

// ReadLocalCommit read git commit hash.
func ReadLocalCommit(repoDir string) (string, error) {
	cmd := exec.Command("git", "rev-parse", "HEAD")
	cmd.Dir = repoDir
	output, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("read git commit hash: %s", output)
	}

	return strings.TrimSpace(string(output)), nil
}

// GetDefaultBranch read git default branch.
func GetDefaultBranch(repoDir string) (string, error) {
	cmd := exec.Command("git", "remote", "show", "origin")
	cmd.Dir = repoDir
	output, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("read git default branch: %s", output)
	}

	lines := strings.SplitSeq(string(output), "\n")
	for line := range lines {
		if strings.Contains(line, "HEAD branch") {
			parts := strings.Split(line, ":")
			if len(parts) > 1 {
				return strings.TrimSpace(parts[1]), nil
			}
		}
	}

	return "", fmt.Errorf("default branch not found of %s", repoDir)
}

// CheckIfRepoIsUpToDate check if repo is already latest.
func CheckIfRepoIsUpToDate(repoDir string) (bool, error) {
	// Read current branch.
	currentBranch, err := GetCurrentBranch(repoDir)
	if err != nil {
		return false, err
	}

	// Retrive remote latest data.
	cmd := exec.Command("git", "fetch")
	if err = cmd.Run(); err != nil {
		return false, fmt.Errorf("git fetch failed: %v", err)
	}

	// Retrive difference data bewteen latest and local.
	cmd = exec.Command("git", "log", "--oneline", "HEAD..origin/"+currentBranch)
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false, fmt.Errorf("git log failed: %v", err)
	}

	// No diffence means is already update to date.
	return len(output) == 0, nil
}

// InitAsLocalRepo init folder as a local repo.
func InitAsLocalRepo(repoDir, message string) error {
	cmd := exec.Command("git", "init")
	cmd.Dir = repoDir
	output, err := cmd.CombinedOutput()
	if err != nil {
		return fmt.Errorf("failed to git init repo -> %s", output)
	}

	cmd = exec.Command("git", "add", "-A")
	cmd.Dir = repoDir
	output, err = cmd.CombinedOutput()
	if err != nil {
		return fmt.Errorf("failed to git add -A -> %s", output)
	}

	cmd = exec.Command("git", "commit", "-m", message)
	cmd.Dir = repoDir
	cmd.Env = append(os.Environ(),
		"GIT_AUTHOR_NAME=CI Robot",
		"GIT_AUTHOR_EMAIL=ci@celer.com",
		"GIT_COMMITTER_NAME=CI Robot",
		"GIT_COMMITTER_EMAIL=ci@celer.com",
	)
	output, err = cmd.CombinedOutput()
	if err != nil {
		return fmt.Errorf("failed to git commit repo -> %s", output)
	}

	return nil
}
