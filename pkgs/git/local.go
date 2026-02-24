package git

import (
	"fmt"
	"os"
	"os/exec"
	"strings"
)

// GetRepoUrl get git repo origin URL.
func GetRepoUrl(repoDir string) (string, error) {
	cmd := exec.Command("git", "-C", repoDir, "remote", "get-url", "origin")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("get repo url -> %s", output)
	}
	return strings.TrimSpace(string(output)), nil
}

// GetCurrentBranch read current branch of repo.
func GetCurrentBranch(repoDir string) (string, error) {
	cmd := exec.Command("git", "-C", repoDir, "rev-parse", "--abbrev-ref", "HEAD")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("get current branch -> %s", output)
	}
	return strings.TrimSpace(string(output)), nil
}

// GetCurrentTag read current tag of repo.
func GetCurrentTag(repoDir string) (string, error) {
	cmd := exec.Command("git", "-C", repoDir, "describe", "--exact-match", "--tags", "HEAD")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("get current tag -> %s", output)
	}
	return strings.TrimSpace(string(output)), nil
}

// IsModified check if repo is modified.
func IsModified(repoDir string) (bool, error) {
	cmd := exec.Command("git", "-C", repoDir, "status", "--porcelain")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false, fmt.Errorf("check if repo is modified -> %s", output)
	}

	return strings.TrimSpace(string(output)) != "", nil
}

// GetCurrentCommit read git commit hash.
func GetCurrentCommit(repoDir string) (string, error) {
	cmd := exec.Command("git", "-C", repoDir, "rev-parse", "HEAD")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("read git commit hash: %s", output)
	}

	return strings.TrimSpace(string(output)), nil
}

// GetDefaultBranch read git default branch.
func GetDefaultBranch(repoDir string) (string, error) {
	cmd := exec.Command("git", "-C", repoDir, "remote", "show", "origin")
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

// CheckIfUpToDate check if repo is already latest.
func CheckIfUpToDate(repoDir string) (bool, error) {
	// Read current branch.
	currentBranch, err := GetCurrentBranch(repoDir)
	if err != nil {
		return false, err
	}

	// Retrieve remote latest data.
	cmd := exec.Command("git", "-C", repoDir, "fetch")
	if err = cmd.Run(); err != nil {
		return false, fmt.Errorf("git fetch failed: %v", err)
	}

	// Check if upstream branch exists
	cmd = exec.Command("git", "-C", repoDir, "rev-parse", "--abbrev-ref", currentBranch+"@{u}")
	if err = cmd.Run(); err != nil {
		return false, fmt.Errorf("no upstream branch configured for %s", currentBranch)
	}

	// Check for differences between local and remote
	cmd = exec.Command("git", "-C", repoDir, "rev-list", "--count", "HEAD.."+currentBranch+"@{u}")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false, fmt.Errorf("git rev-list failed: %v", err)
	}

	// Parse the number of commits behind
	behindCount := strings.TrimSpace(string(output))
	return behindCount == "0", nil
}

// InitAsLocalRepo init folder as a local repo.
func InitAsLocalRepo(repoDir, message string) error {
	cmd := exec.Command("git", "-C", repoDir, "init")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return fmt.Errorf("failed to git init repo -> %s", output)
	}

	cmd = exec.Command("git", "-C", repoDir, "add", "-A")
	output, err = cmd.CombinedOutput()
	if err != nil {
		return fmt.Errorf("failed to git add -A -> %s", output)
	}

	cmd = exec.Command("git", "-C", repoDir, "commit", "-m", message)
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
