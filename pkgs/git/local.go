package git

import (
	"bytes"
	"fmt"
	"os"
	"os/exec"
	"strings"
)

// GetRepoUrl get git repo origin URL.
func GetRepoUrl(repoDir string) (string, error) {
	cmd := exec.Command("git", "remote", "get-url", "origin")
	cmd.Dir = repoDir

	var buffer bytes.Buffer
	cmd.Stdout = &buffer

	if err := cmd.Run(); err != nil {
		return "", fmt.Errorf("can not get origin URL: %v", err)
	}

	return strings.TrimSpace(buffer.String()), nil
}

// CheckIfLocalBranch check if repoRef is a branch.
func CheckIfLocalBranch(repoDir, repoRef string) (bool, error) {
	// Also can call `git symbolic-ref --short HEAD`
	cmd := exec.Command("git", "rev-parse", "--abbrev-ref", "HEAD")
	cmd.Dir = repoDir
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false, err
	}
	return strings.TrimSpace(string(output)) == repoRef, nil
}

// CheckIfLocalTag check if repoRef is a tag.
func CheckIfLocalTag(repoDir, repoRef string) (bool, error) {
	cmd := exec.Command("git", "describe", "--exact-match", "--tags", "HEAD")
	cmd.Dir = repoDir
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false, err
	}
	return strings.TrimSpace(string(output)) == repoRef, nil
}

// CheckIfLocalCommit check if repoRef is a commit.
func CheckIfLocalCommit(repoDir, repoRef string) (bool, error) {
	cmd := exec.Command("git", "rev-parse", "HEAD")
	cmd.Dir = repoDir
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false, err
	}
	return strings.TrimSpace(string(output)) == repoRef, nil
}

// IsModified check if repo is modified.
func IsModified(repoDir string) (bool, error) {
	cmd := exec.Command("git", "status", "--porcelain")
	cmd.Dir = repoDir
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false, fmt.Errorf("failed to check if repo is modified -> %s", output)
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

// DefaultBranch read git default branch.
func DefaultBranch(repoDir string) (string, error) {
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

	return "", fmt.Errorf("default branch not found")
}

func BranchOfLocal(repoDir string) (string, error) {
	cmd := exec.Command("git", "rev-parse", "--abbrev-ref", "HEAD")
	cmd.Dir = repoDir
	output, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("get current branch name: %s", output)
	}
	return strings.TrimSpace(string(output)), nil
}

func InitRepo(repoDir, message string) error {
	cmd := exec.Command("git", "init")
	cmd.Dir = repoDir
	output, err := cmd.CombinedOutput()
	if err != nil {
		return fmt.Errorf("failed to git init repo -> %w", output)
	}

	cmd = exec.Command("git", "add", "-A")
	cmd.Dir = repoDir
	output, err = cmd.CombinedOutput()
	if err != nil {
		return fmt.Errorf("failed to git add -A -> %w", output)
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
